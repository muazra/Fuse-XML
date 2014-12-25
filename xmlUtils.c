#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
/*
 This function is used to Parse the XML document into a tree and return the tree
 */
xmlDocPtr getdoc(char *docname) {
    xmlDocPtr doc;
    //initialize parser
    xmlInitParser();
    //parse the document docname
    doc = xmlParseFile(docname);
    LIBXML_TEST_VERSION
    if (doc == NULL) {
        fprintf(stderr, "Document not parsed successfully. \n");
        return NULL;
    }

    return doc;
}
/*
 Retrieves the nodes of a given path from inside of the data structure 
 The data structure is embedded in XPATH
 This function returns the nodes(i.e files or directories) that are in path(i.e parameter xpath)
 */
xmlXPathObjectPtr getnodeset(xmlDocPtr doc, xmlChar *xpath) {
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    //makes a new context for XPATH of the doc
    context = xmlXPathNewContext(doc);
    if (context == NULL) {
        printf("Error in xmlXPathNewContext\n");
        return NULL;
    }
    //evaluates the xpath path parameter and returns a pointer to the specific nodes inside xpath path
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        printf("Error in xmlXPathEvalExpression\n");
        return NULL;
    }
    if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        xmlXPathFreeObject(result);
        printf("No result\n");
        return NULL;
    }
    //returns pointer to the nodes that are inside of the specific path xpath
    return result;
}
/*
 * Function used to implement readdir command in fuse.
 * It is used to read all of the directories inside of the filesystem
 * Take new updated file path and parsed doc and file status(file or directory)
 */
char **readDir(char* fusePath, xmlDocPtr doc, int fstatus[]) {
    
    xmlChar *xpath = (xmlChar*) fusePath;
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword, *name;
    char* childXpath;

    char *xpath2 = strdup(fusePath);
    int pathlen = strlen(xpath2);
    xpath2[pathlen - 2] = 0;
    //get the pointer to the set of nodes inside of the xpath path
    result = getnodeset(doc, xpath);
    if (result) {
        //ptr to the value of the nodes inside of the xpath path
        nodeset = result->nodesetval;
        char **childNames = (char**) malloc(sizeof (char *) * nodeset->nodeNr);
        //traverse the number of nodes, nodeset->Nr = the number of nodes inside of the node set 
        for (i = 0; i < nodeset->nodeNr; i++) {
            //get the name of each node inside of the node set          
            name = nodeset->nodeTab[i]->name;
            //print the name of the node
            printf("name: %s\n", name);
            //add into childNames 
            childNames[i] = strdup(name);
            
            childXpath = malloc(strlen(name) + strlen(xpath) + 4);
            //add the parent path and the name of the child into childXpath 
            sprintf(childXpath, "%s/%s/*", xpath2, name);
            //check the number of children that in this childXpath
            int childVal = numChilds(childXpath, doc);
            //if there are no children then fstatus is 0 meaning it is a file
            if (childVal == 0) {
                fstatus[i] = 0;
            //if there is a child than fstatus is 1 meaning it is a directory
            } else {
                fstatus[i] = 1;
            }
        }
        //make the last childName element NULL
        childNames[i] = NULL;
        xmlXPathFreeObject(result);
        //return all of the names inside of the node set with altered fstatus
        return childNames;
    } else {
        printf("not found from xPath=%s\n ", fusePath);
    }

    return NULL;
}
/*
 //
 find the number of children inside of the path
 */
int numChilds(char* fusePath, xmlDocPtr doc) {

    xmlChar *xpath = (xmlChar*) fusePath;
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword, *name;
    
    result = getnodeset(doc, xpath);
    if (result) {
        //nodeset = a ptr to the values of the node set
        nodeset = result->nodesetval;
        //Find the number of nodes or children inside of that path 
        int nodeNum = nodeset->nodeNr;
        printf("NodeNUM --NUMCHILD = %d fusePath=%s\n", nodeNum, fusePath);
        //return number of children or nodes in the path
        return nodeNum;

    } else {
        //there are no children in the path
        printf("nChild=0 from xPath=%s\n ", fusePath);
        return 0;
    }

    return 0;
}
/*
 * returns number of children
 * assigns the atts[] parameter with atts[0] = number of children and atts[1] = length of content
 * 
 */
int getFileAtts(char* fusePath, xmlDocPtr doc, int atts[]) {

    xmlChar *xpath = (xmlChar*) fusePath;
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword, *name;
    //get a ptr to the node set
    result = getnodeset(doc, xpath);
    if (result) {
        int contentLen = 0;
        //nodeset = the value of the nodes in the node set
        nodeset = result->nodesetval;
        //find the value of the node at the end of the path
        char *contentVal = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
        if (contentVal != NULL) {
            contentLen = strlen(contentVal);
        }
        
        char *path2 = malloc(strlen(fusePath) + 2);
        //add /* to path in order to find out the number of children nodes in the path
        sprintf(path2, "%s/*", fusePath);
        //get the number of children of the path
        int nChilds = numChilds((xmlChar*) path2, doc);
        //set the attributes of that file or directory
        atts[0] = nChilds;
        atts[1] = contentLen;

        printf("NodeNUM --NUMCHILD = %d fusePath=%s contentVal=%s contentlen=%d\n",
                nChilds, fusePath, contentVal, contentLen);

        return nChilds;

    } else {
        printf("nChild=0 from xPath=%s\n ", fusePath);
        return -1;
    }

    return 0;
}
/**
 * Gets the contents of the file and returns it
 * 
 */
char * getFileContent(char* fusePath, xmlDocPtr doc, int fsize) {

    xmlChar *xpath = (xmlChar*) fusePath;
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword, *name;
    
    result = getnodeset(doc, xpath);
    if (result) {
      //ptr to the value inside of the nodes in the xpath path
        nodeset = result->nodesetval;
        //retrieve the string/content inside of the node or file
        char *contentVal = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
        //calculate length of value
        int contentLen = strlen(contentVal);
        printf("getFileContent =%s\n", contentVal);
        char *s = strdup(contentVal);
        //ends the content value with a NULL character if it is greater than fsize 
        //delimits the amount of content that can be read to size fsize
        if (fsize < contentLen)
            s[fsize - 1] = 0;
        return s;
    } else {
        printf("getFileContent=0 from xPath=%s\n ", fusePath);
        return 0;
    }

    return 0;
}

int removeNode(char* fusePath, xmlDocPtr doc) {

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;

    result = getnodeset(doc, (xmlChar*) fusePath);
    
    if (result) {
        printf("Delete Node = %s \n", fusePath);
        nodeset = result->nodesetval;
        //pNode is the last node inside of the node set path
        xmlNodePtr pNode = nodeset->nodeTab[0];
        //remove the node at the end of the path
        xmlUnlinkNode(pNode);

        xmlSaveFileEnc("writeTest.xml", doc, "UTF-8");
        return 1;
    } else {
        printf("MAKE NEW NODE FAILED for=%s\n ", fusePath);
        return 0;
    }


}
/*
 You have to use file/directories a from and a to
 * find the parents of each
 * seperate the parent path from each 
 * copy or link the node to its new destination with the changed name
 * unlink the old node
 * 
 */
int renameNode(char* from, char *to, xmlDocPtr doc) {

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    char fromNodeName[1024], toNodeName[1024];
    //finding the parent path that the node is from
    char *fromParentName = strdup(from);
    //traverses string and cuts out the path before the last '/'
    char *p = strrchr(fromParentName, '/');
    strcpy(fromNodeName, p + 1);
    *p = 0;
    //finding the parent path that the node is going to
    char *toParentName = strdup(to);
    //traverses string and cuts out the path before the last '/'
    p = strrchr(toParentName, '/');
    strcpy(toNodeName, p + 1);
    *p = 0;

    xmlNodePtr fromNode = NULL, toParentNode = NULL;
    //aquire a pointer to the set of nodes that are in the path of from from doc
    result = getnodeset(doc, (xmlChar*) from);
    if (result) {
        //take the value of the node
        nodeset = result->nodesetval;
        //set fromNode ptr equal to value of the node that will be renamed
        fromNode = nodeset->nodeTab[0];
    }
    //aquire a pointer to the set of nodes that are in the path of toParentName or otherwise to
    result = getnodeset(doc, (xmlChar*) toParentName);
    if (result) {
        //take the value of the node
        nodeset = result->nodesetval;
        //set toParentNode ptr equal to the value of the new node name
        toParentNode = nodeset->nodeTab[0];
    }

    //if there are two nodes that are not null check if their parents are the same 
    if (fromNode != NULL && toParentNode != NULL) {
        if (strcmp(fromParentName, toParentName) == 0) {
            printf ("Copying in same folder %s to %s \n",fromNode->name,toParentName);
            //if parents are the same then copy the new name of the node into the old node fromNode
            strcpy(fromNode ->name, toNodeName);
            
        } else {
            printf ("Copying in different folder %s to %s \n",fromNode->name,toParentName);
            //if copying to a different folder other then the parents then
            //first copy the new name to the old node (fromNode)
            strcpy(fromNode ->name, toNodeName);
            //copy the old node
            xmlNodePtr copyNode = xmlCopyNode(fromNode,1); 
            //remove the old node (fromNode)
            xmlUnlinkNode(fromNode);
            //add the old node into the new parent node (toParentNode)
            xmlAddChild(toParentNode, copyNode);
        }

        xmlSaveFileEnc("writeTest.xml", doc, "UTF-8");
        return 1;

    } else {
        printf("Rename node from %s to %s \n ", from,to);
        return 0;
    }
}

/*
 Makes a new file into the given path
 */
int makeNewNode(char* fusePath, xmlDocPtr doc, int createTextNode) {

    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    char nodeName[1024];
    //Find the parent node path without the desired NewNode by traversing the path all the way to the last '/'
    char *parentNodePath = strdup(fusePath);
    char *p = strrchr(parentNodePath, '/');
    strcpy(nodeName, p + 1);
    *p = 0;
    printf("parentNewNODE -- = %s", parentNodePath);
    //return the set of nodes in that parentNodePath 
    result = getnodeset(doc, (xmlChar*) parentNodePath);
    //if result is a real path then add the new nodeName into the parentNodePath
    if (result) {
        printf("Make new Node = %s -- Node Name = %s\n", parentNodePath, nodeName);
        //sets value of the nodeset to the parent path
        nodeset = result->nodesetval;
        //is the node at the end of the node set, or the node at the end of the file path
        xmlNodePtr pNode = nodeset->nodeTab[0];
        //makes a new child or nodeName under the last node in the path,
        //the i.e. a/b/c so it adds the new child under c or pNode in this case
        xmlNodePtr newChild = xmlNewChild(pNode, NULL, BAD_CAST nodeName, NULL);
        //if the newNode created is a directory than add a textNode into it
        //this is done to supplement our logic of how a directory and file are read
        if (createTextNode) {
            xmlNewChild(newChild, NULL, BAD_CAST "textNode", NULL);
        }
        
        xmlSaveFileEnc("writeTest.xml", doc, "UTF-8");
        return 1;
    } else {
        printf("MAKE NEW NODE FAILED for=%s\n ", fusePath);
        return 0;
    }


}
/*
 Write content into a file
 */
int writeFileContent(char* fusePath, xmlDocPtr doc, char *buf, int fsize) {

    xmlChar *xpath = (xmlChar*) fusePath;
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    xmlChar *keyword, *name;

    result = getnodeset(doc, xpath);
    if (result) {
        printf("write to file = %s  content =%s\n", fusePath, buf);
        //has the value of all of the nodes in the node set
        nodeset = result->nodesetval;
        //the node that will be written to pNodeor otherwise known as the file node
        xmlNodePtr pNode = nodeset->nodeTab[0];
        //set content of the file node with whatever is inside of buf
        xmlNodeSetContent(pNode, (xmlChar *) buf);
        
        int contentLen = strlen(buf);
        xmlSaveFileEnc("writeTest.xml", doc, "UTF-8");
        return contentLen;
    } else {
        printf("writeFile Content=0 from xPath=%s\n ", fusePath);
        return 0;
    }


}
