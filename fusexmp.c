
/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
 
 */

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <xmlUtils.h>

xmlDocPtr doc = NULL;
/*
 * Most of the logic for the filesystem is inside of this function
 * This function sets the attributes of the file/directories of the filesystem
 * It assigns a mode for each individual node either as a directory or as a file and allows permissions accordingly
 * i.e if it is a file that you can't add a file inside of it, if it is a folder you can etc..
 */
static int xmp_getattr(const char *path, struct stat *stbuf) {
    int res;
    //checks to see if path is root or not if it is then it is a directory
//stbuf is used to change the mode of the node because it contains variables inside of the struct stat that will change the mode of the node.
    if (strcmp(path, "/") == 0) {
        memset(stbuf, 0, sizeof (struct stat));
        //labels the node as a directory/folder
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
    } else {
        printf("GETATTR -- path=%s \n",path);
        int atts[2];
	//returns the number of children as well as the content value inside of the atts[] variable
        int res = getFileAtts((xmlChar*) path, doc,atts);
        int childVal = atts[0];
        printf("GETATTR -- path=%s , childVal = %d\n",path,childVal);
        //if the node is a (directory/not last node) or a leaf/last node
        if (res >= 0){
            if (childVal > 0) {
                memset(stbuf, 0, sizeof (struct stat));
                //declare the mode of the node as directory
                stbuf->st_mode = S_IFDIR | 0777;
                stbuf->st_nlink = 2;
            } else {
                memset(stbuf, 0, sizeof (struct stat));
                //declare the mode of the node as file
                stbuf->st_mode = S_IFREG | 0777;
                stbuf->st_size = atts[1];
                stbuf->st_nlink = 1;
            }
            return 0;
        }else{
            return -ENOENT;
        }
    }

    return 0;
}

static int xmp_access(const char *path, int mask) {
    int res;
    printf("ACCESS =%s",path);
    res = access(path, mask);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size) {
    int res;

    res = readlink(path, buf, size - 1);
    if (res == -1)
        return -errno;

    buf[res] = '\0';
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    int i;
    (void) offset;
    (void) fi;
    
    printf("Readdir path -- = %s\n ", path);
    char *path2 = malloc(strlen(path) + 2);
    //if it is reading parent 
    if (strcmp(path, "/") == 0) {
        //show all files and directories underneath the parent using *
        sprintf(path2, "%s*", path);
    } else {
        //show all folders and files under the current directory(not parent) using /*
        sprintf(path2, "%s/*", path);
    }
    int fileStatus[1024];
    //input the new path2 into readDir function 
    char **lines = readDir((xmlChar*) path2, doc, fileStatus);
    //if lines is empty, it has no children, then it is an empty file
    if (lines == NULL) {
        struct stat stbuf;
        //cleans stbuf
        memset(&stbuf, 0, sizeof (struct stat));
        //changes the mode of the node to a file
        stbuf.st_mode = S_IFREG | 0777;
        stbuf.st_nlink = 1;
        //mark the path as a file
        filler(buf, path, &stbuf, 0);
    } else {
        //else lines must have more than one node meaning it must contain a parent
        printf("Readdir path2 = %s\n ", path2);
	struct stat stbuf2;
        //cleans stbuf
	 memset(&stbuf2, 0, sizeof (struct stat));
         //parent is always a directory
	        stbuf2.st_mode = S_IFDIR | 0777;
                stbuf2.st_nlink = 2;
        filler(buf, ".", &stbuf2, 0);
        filler(buf, "..",&stbuf2, 0);
        //traverse all of the nodes and check for the fstatus of each node
        for (i = 0; lines[i] != NULL; i++) {
            struct stat stbuf;
            memset(&stbuf, 0, sizeof (struct stat));
            //if file status is 1 that means it is a directory
            if (fileStatus[i] == 1) {
                //change mode to be a directory
                stbuf.st_mode = S_IFDIR | 0777;
                stbuf.st_nlink = 2;
            } else {
                //if 0 then it is a file, change mode to file
                stbuf.st_mode = S_IFREG | 0777;
                stbuf.st_nlink = 1;
            }
            //print all nodes
            printf("lines =%s,counter=%d\n", lines[i], i);
            //fill buf with the all nodes 
            filler(buf, lines[i], &stbuf, 0);
        }
    }


   
    return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev) {
    int res;
    printf("MKNOD -- = %s",path);
    makeNewNode(path,doc,0);
    
    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode) {
    int res;
    printf("MKDIR -- path = %s\n",path);
    makeNewNode(path,doc,1);
   
    return 0;
}

static int xmp_unlink(const char *path) {
    int res;
    printf("UNLINK -- = %s", path);
    removeNode(path,doc);
    
    
    return 0;
}

static int xmp_rmdir(const char *path) {
    int res;
    printf("RMDIR -- = %s",path);
    removeNode(path,doc);
    
    return 0;
}

static int xmp_symlink(const char *from, const char *to) {
    int res;

    res = symlink(from, to);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_rename(const char *from, const char *to) {
    int res;

    printf ("RENAME from %s to %s \n",from,to);
    renameNode (from,to,doc);
    
    
    
    return 0;
}

static int xmp_link(const char *from, const char *to) {
    int res;

    res = link(from, to);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_chmod(const char *path, mode_t mode) {
    int res;
    printf("CHMOD -- path = %s\n",path);
    res = chmod(path, mode);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid) {
    int res;

    res = lchown(path, uid, gid);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_truncate(const char *path, off_t size) {
    int res;
    printf("TRUNCATE -- path =%s\n",path);
    writeFileContent(path,doc,"",0);
   
    return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2]) {
    int res;
    struct timeval tv[2];

    tv[0].tv_sec = ts[0].tv_sec;
    tv[0].tv_usec = ts[0].tv_nsec / 1000;
    tv[1].tv_sec = ts[1].tv_sec;
    tv[1].tv_usec = ts[1].tv_nsec / 1000;

    res = utimes(path, tv);
  

    return 0;
}
/*
 * Opens only files not folders
 */
static int xmp_open(const char *path, struct fuse_file_info *fi) {
    int res;
    printf("OPEN path=%s\n",path);
    int atts[2];
    //checks if it has any children.
    //if it doesn't than it should return true if it does than it is a folder and it should return false
    int childVal = numChilds((xmlChar*) path, doc);
    return (!childVal);
   
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi) {
    
    int fd;
    int res;
    printf("READ path=%s Size =%d BuffSize = %d Offset = %d\n",path,size,strlen(buf),offset);
    (void) fi;
    char *content = getFileContent(path, doc,size);
    //if there is content than copy to buffer
    if (content != NULL){
        strcpy (buf,content);
    } else
        return -errno;
    //return length of the content
    return strlen(content);
    
}

static int xmp_write(const char *path, const char *buf, size_t size,
        off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;

    printf("WRITE path=%s Size =%d BuffSize = %d Offset = %d\n",path,size,strlen(buf),offset);
    (void) fi;
   int nlen = writeFileContent(path, doc,buf,size);
     
    return nlen;
    
   
}

static int xmp_statfs(const char *path, struct statvfs *stbuf) {
    int res;

    printf("STATFS-- path = %s\n",path);
    res = statvfs(path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi) {
  

    (void) path;
    (void) fi;
    return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
        struct fuse_file_info *fi) {
  
    (void) path;
    (void) isdatasync;
    (void) fi;
    return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
        int fd;

        printf ("CRERATE path= %s\n",path);
        
        
        return 0;
}





static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .access = xmp_access,
    .readlink = xmp_readlink,
    .readdir = xmp_readdir,
    .mknod = xmp_mknod,
    .mkdir = xmp_mkdir,
    .symlink = xmp_symlink,
    .unlink = xmp_unlink,
    .rmdir = xmp_rmdir,
    .rename = xmp_rename,
    .link = xmp_link,
    .chmod = xmp_chmod,
    .chown = xmp_chown,
    .truncate = xmp_truncate,
    .utimens = xmp_utimens,
    .open = xmp_open,
    .read = xmp_read,
    .write = xmp_write,
    .statfs = xmp_statfs,
    .release = xmp_release,
    .fsync = xmp_fsync,
#ifdef HAVE_SETXATTR
    .setxattr = xmp_setxattr,
    .getxattr = xmp_getxattr,
    .listxattr = xmp_listxattr,
    .removexattr = xmp_removexattr,
#endif
};

int main(int argc, char *argv[]) {
    
    if (argc < 5){
       printf ("Usage : ./fusexmp -d -s -f {MOUNT FOLDER} {XML file} \n");

    } 
    umask(0);
    //Parses the document included at run time and returns a tree
    doc = getdoc(argv[argc-1]); //"/home/nahit7/keyword.xml");
    printf ("argc = %d xml=%s\n",argc,argv[argc-1]);
    argc = argc - 1;
    
    return fuse_main(argc, argv, &xmp_oper, NULL);
}


