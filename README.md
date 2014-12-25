Install
========

1) Fuse 2.8.5.
“yum install fuse 2.8.5”

2) libxml2.
“yum install libxml2”

3) libxml2-devel.
“yum install libxml2-devel”

Build
========

1) “make clean” <br>
2) “make” <br>
3) “sudo modprobe fuse” <br>

Run
========

Mount:
“./fusexmp -d -s -f ./fusepoint/ {XML file(give full path)}”

Unmount:
“fusermount -u fusepoint”

Test
========

XML files: <br>
keyword.xml <br>
project.xml

Shell Script: <br>
test.sh



