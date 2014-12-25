echo "find -- (Finding all files and folders under mount directory)"
find fusepoint -name \* -print
echo "cat -- (Displaying the contents of the datawritten file)"
cat  fusepoint/story/storyinfo/datewritten
echo "grep -- (Finding the word (example) in the keyword file)"
grep example fusepoint/story/storyinfo/keyword
echo "grep -- (Finding the word Nahit in the test file)"
grep Nahit fusepoint/story/storyinfo/author/test
echo "(This is a message that is written into test file.We will see this message in the file when we cat the file)" 
echo "echo -- This is a message that is written into test file.We will see this message in the file when we cat the file" > fusepoint/story/storyinfo/author/test
echo "cat --(Now we are reading(cat) the test file.Here is its content)"
cat fusepoint/story/storyinfo/author/test
echo "cp -- (Copying test into same directory as test2)"
cp fusepoint/story/storyinfo/author/test fusepoint/story/storyinfo/author/test2
echo "find -- (We can now see that test and test2 are both under the directory author)"
find fusepoint/story/storyinfo/author/ -name \* -print
mv fusepoint/story/storyinfo/author/test fusepoint/story/storyinfo/author/test3
echo "find -- (We can now see that test has been renamed to test3)"
find fusepoint/story/storyinfo/author/ -name \* -print
echo "mkdir -- (Making a new directory inside author)"
mkdir fusepoint/story/storyinfo/author/afolder
echo "touch -- (Making a new file inside author)"
touch fusepoint/story/storyinfo/author/afile
echo "find -- (We can now see that afolder and afile are both under the directory author)"
find fusepoint/story/storyinfo/author/ -name \* -print
echo "rmdir -- (Removing the directory afolder from author)"
rmdir fusepoint/story/storyinfo/author/afolder
echo "rm -- (Removing the file afile from author)"
rm    fusepoint/story/storyinfo/author/afile
echo "find -- (We can now see that afolder and afile are both removed from the directory author)"
find fusepoint/story/storyinfo/author/ -name \* -print
mkdir fusepoint/story/storyinfo/author/afolder
echo "touch -- (Making a new file inside author)"
touch fusepoint/story/storyinfo/author/afile
echo "find -- (We can now see that afolder and afile are both under the directory author)"


