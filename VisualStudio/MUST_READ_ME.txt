Detailed build documentation can be found on the OpenSceneGraph website at:

  http://openscenegraph.org/osgwiki/pmwiki.php/PlatformSpecifics/VisualStudio

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
    
For details on how to patch VisualStudio6.0 see the above website link. 
Note, osgIntrospection, src/osgWrapper plugins and the 
osgintrospection example cannot be compiled under VisualStudio plugin.  
You will also need to run the fixup-vc6-dsps.pl script to clean up the
project files that won't otherwise compile to do new elements required
for support of Window 64 bit build under VS 7.x and 8.x.    

Several of the plugins and demoes, and two of the core libraries - osgTerrain 
and osgProducer require external dependancies.  Full details on where to obtain
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

--

Building 64 bit binaries

64 bit OpenSceneGraph, Producer, and OpenThreads binaries can be built in Visual Studio 8, but several extra steps are required due to limits of the Visual Studio 6 project files:

1. For each of the OpenSceneGraph, and OpenThreads .dsw files, and the Producer .sln files:
  a. Open the .dsw or .sln file and convert all projects to VS 8 format.
  b. Open the Configuration Manager window under the Build menu, bring up the New Solution Platform window by selecting <New...> in the Active solution platform drop-down menu.  Select x64 as the new platform and copy settings from Win32 (you need to have the x64 compiler installed to see the x64 platform option).  Ensure the Create new project platforms checkbox is selected.  Click OK, then close the Configuration Manager window.
  c. Do a "Save All" to save the project files.

2. Visual Studio unfortunately chooses its own Output Directory setting for the x64 configurations in step 1.b., and this must be reset to the Win32 setting.

  IF PERL _IS_ INSTALLED (native or Cygwin), do the following:
  a. Close all Visual Studio solutions.
  b. Open a command shell and cd to the OpenSceneGraph/VisualStudio directory.
  c. Run the command "perl reset-64bit-outdirs.pl".
  d. Reopen the solutions.

  IF PERL _IS NOT_ INSTALLED, do the following to accomplish the same thing manually:
  a. In the OpenThreads solution, open the properties window for the OpenThreads project.
     i. Select multiple configurations, Debug and Release.  Under the General page, overwrite the Output Directory path of the x64 platform with the corresponding Win32 path.  For static builds, do the same thing for the Debug Static and Release Static configurations.
  b. Open the Producer project properties in the Producer solution.
     i. Repeat step 2.a.i.
  c. In the OpenSceneGraph solution:
     i. Select all the "Application" and "Example" projects in the Solution Explorer window and repeat step 2.a.i.  Note there are no static configurations.
     ii. Select all the "Core" projects _except "Core osgIntrospection"_ and repeat step 2.a.i.
     iii. Select "Core osgIntrospection" and repeat step 2.a.i.  Note there are no static configurations.
     iv. Select all the "osgPlugin" projects and repeat step 2.a.i.
     v. Select all the "osgWrapper" projects and repeat step 2.a.i.  Note there are no static configurations.
  d. Do a "Save All" to save the project files.

3. Select the desired x64 configuration, and build away!
