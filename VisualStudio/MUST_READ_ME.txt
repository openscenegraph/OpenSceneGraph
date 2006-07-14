IMPORTANT NOTE: Whilst the OSG will compile cleanly with the basic VC++6.0
and its own STL implementation, the OSG will crash regularily due to bugs
in VC++6.0's STL. VC++6.0's STL is horribly broken and therefore is *NOT*
supported. Do not attempt to use the OSG in conjunction with native VC++6.0
STL implemention. 

The supported combinations are: 

    1.Visual Studio8.x .NET 
    2.Visual Studio7.x .NET 
    
Deprecated are:

    3.Visual Studio6.0 + Dinkumware's STL bug fix patches
    4.Visual Studio6.0 + STLport
    
For details on how to patch VisualStudio6.0 read the doc/install.html 
documentation. Note, osgIntrospection, src/osgWrapper plugins and the 
osgintrospection example cannot be compiled under VisualStudio plugin.

Several of the plugins and demoes, and two of the core libraries - osgText 
and osgGLUT require external dependancies.  Full details on where to obtain
these can be found in doc/dependancies.html.


--

For syntax highlighting in VisualStudio which the standard C++ style headers 
found in the OSG :

VisualStudio6.0   

    Substiture the LANDEXT.DAT file found in this directory with the one found
    *\Common\MSDev98\Bin

VisualStudio7.x & 8.x/.NET

    Install the syntaxhighlight.reg (just double click it). This will update 
    Extensionless file for Visual Studio.  Don't worry, it will keep the 
    current extensionless files (STL ones) intact.

--

How to use the Visual Studio projects:

To build the OpenSceneGraph code in Visual Studio, you normally must use the VisualStudio.sln solution file provided.  The individual projects won't build as-is, because they depend on each other and only the VisualStudio.sln file provides those dependencies.

To create a program based on an example, probably the easiest way is to do this:

1. Copy the VisualStudio.sln project to a new file in the same directory
2. Copy the project you want to base your new project on to a new directory in the same level of the directory tree
3. Open the new .sln file you copied in step 1
4. Remove unneeded projects from it, but keep the core libraries (osg, osgDB, etc.).  Shift-clicking to select a bunch of projects at once makes this easier to do
5. Add the new project to that solution
6. Set the dependencies for your new project.  This is most easily done by opening the Solution Properties dialog, going to Project Dependencies, and checking off the libraries your project depends on

