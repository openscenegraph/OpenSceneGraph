IMPORTANT NOTE: Whilst the OSG will compile cleanly with the basic VC++6.0
and its own STL implementation, the OSG will crash regularily due to bugs
in VC++6.0's STL. VC++6.0's STL is horribly broken and therefore is *NOT*
supported. Do not attempt to use the OSG in conjunction with native VC++6.0
STL implemention. 

The supported combinations are: 

    1.Visual Studio7.0 .NET 
    2.Visual Studio6.0 + Dinkumware's STL bug fix patches
    3.Visual Studio6.0 + STLport
    
For details on how to patch VisualStudio6.0 read the doc/install.html 
documentation.

Several of the plugins and demoes, and two of the core libraries - osgText 
and osgGLUT require external dependancies.  Full details on where to obtain
these can be found in doc/dependancies.html.


--

For syntax highlighting in VisualStudio which the stanard C++ style headers 
found in the OSG :

VisualStudio6.0   

    Substiture the LANDEXT.DAT file found in this directory with the one found
    *\Common\MSDev98\Bin

VisualStudio7.0/.NET

    Install the syntaxhighlight.reg (just double click it). This will update 
    Extensionless file for Visual Studio.  Don't worry, it will keep the 
    current extensionless files (STL ones) intact.
