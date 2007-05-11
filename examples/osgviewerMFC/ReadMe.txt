This project is a very simple implementation of the Microsoft Multiple Document/View Architecture.
The only changes needed to compile the project should be to modify the include paths for headers
and librarys.  This project was written to show how to implement the new osgViewer library with MFC.

There is only one problem that I have seen to this point and that is when you have multiple OSG documents 
open and then you close one of them then all remaining OSG documents quit rendering.  I have a small work 
around in the code that calls AfxMessageBox when the closing windows tread exits and this keeps the other
windows rendering correctly.   Not sure what the problem is at this point so if anyone finds the cause and 
has a fix please update the code. 