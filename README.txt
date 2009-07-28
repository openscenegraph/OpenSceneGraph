Welcome to the OpenSceneGraph (OSG).

For up-to-date information on the project, in-depth details on how to 
compile and run libraries and examples, see the documentation on the 
OpenSceneGraph website:

    http://www.openscenegraph.org
  
For the impatient, read the simplified build notes below. For support 
subscribe to our public mailing list:

    http://www.openscenegraph.org/projects/osg/wiki/MailingLists


Robert Osfield.
Project Lead.
28th July 2009.

--

How to build the OpenSceneGraph
===============================

The OpenSceneGraph uses the CMake build system to generate a 
platform-specific build environment.  CMake reads the CMakeLists.txt 
files that you'll find throughout the OpenSceneGraph directories, 
checks for installed dependenciesand then generates the appropriate 
build system.

If you don't already have CMake installed on your system you can grab 
it from http://www.cmake.org, use version 2.4.6 or later.  Details on the 
OpenSceneGraph's CMake build can be found at:

    http://www.openscenegraph.org/projects/osg/wiki/Build/CMake

Under unices (i.e. Linux, IRIX, Solaris, Free-BSD, HP-Ux, AIX, OSX) 
use the cmake or ccmake command-line utils, or use the included tiny 
configure script that'll run cmake for you.  The configure script 
simply runs 'cmake . -DCMAKE_BUILD_TYPE=Release' to ensure that you 
get the best performance from your final libraries/applications.
 
    cd OpenSceneGraph
    ./configure
    make
    sudo make install
  
Alternatively, you can create an out-of-source build directory and run 
cmake or ccmake from there. The advantage to this approach is that the 
temporary files created by CMake won't clutter the OpenSceneGraph 
source directory, and also makes it possible to have multiple 
independent build targets by creating multiple build directories. In a 
directory alongside the OpenSceneGraph use:

    mkdir build
    cd build
    cmake ../OpenSceneGraph -DCMAKE_BUILD_TYPE=Release
    make
    sudo make install

Under Windows use the GUI tool CMakeSetup to build your VisualStudio 
files. The following page on our wiki dedicated to the CMake build 
system should help guide you through the process:

    http://www.openscenegraph.org/projects/osg/wiki/Support/PlatformSpecifics/VisualStudio

Under OSX you can either use the CMake build system above, or use the 
Xcode projects that you will find in the OpenSceneGraph/Xcode 
directory. See release notes on OSX CMake build below.

For further details on compilation, installation and platform-specific 
information read "Getting Started" guide:

    http://www.openscenegraph.org/projects/osg/wiki/Support/GettingStarted
   
   
-- Release notes on OSX build, by Eric Sokolowsky, August 5, 2008

There are several ways to compile OpenSceneGraph under OSX.  The
recommended way is to use CMake 2.6 to generate Xcode projects, then use
Xcode to build the library. The default project will be able to build
Debug or Release libraries, examples, and sample applications. Here are
some key settings to consider when using CMake:

BUILD_OSG_EXAMPLES - By default this is turned off. Turn this setting on
to compile many great example programs.

CMAKE_OSX_ARCHITECTURES - Xcode can create applications, executables,
libraries, and frameworks that can be run on more than one architecture.
Use this setting to indicate the architectures on which to build OSG.
Possibilities include ppc, ppc64, i386, and x86_64. Building OSG using
either of the 64-bit options (ppc64 and x86_64) has its own caveats
below.

OSG_BUILD_APPLICATION_BUNDLES - Normally only executable binaries are
created for the examples and sample applications. Turn this option on if
you want to create real OSX .app bundles. There are caveats to creating
.app bundles, see below.

OSG_WINDOWING_SYSTEM - You have the choice to use Carbon or X11 when
building applications on OSX. Under Leopard and later, X11 applications,
when started, will automatically launch X11 when needed. However,
full-screen X11 applications will still show the menu bar at the top of
the screen. Since many parts of the Carbon user interface are not
64-bit, X11 is the only supported option for OSX applications compiled
for ppc64 or x86_64.

There is an Xcode directory in the base of the OSG software
distribution, but its future is limited, and will be discontinued once
the CMake project generator completely implements its functionality.


APPLICATION BUNDLES (.app bundles)

The example programs when built as application bundles only contain the
executable file. They do not contain the dependent libraries as would a
normal bundle, so they are not generally portable to other machines.
They also do not know where to find plugins. An environmental variable
OSG_LIBRARY_PATH may be set to point to the location where the plugin
.so files are located. OSG_FILE_PATH may be set to point to the location
where data files are located. Setting OSG_FILE_PATH to the
OpenSceneGraph-Data directory is very useful when testing OSG by running
the example programs.

Many of the example programs use command-line arguments. When
double-clicking on an application (or using the equivalent "open"
command on the command line) only those examples and applications that
do not require command-line arguments will successfully run. The
executable file within the .app bundle can be run from the command-line
if command-line arguments are needed.


64-BIT APPLICATION SUPPORT

OpenSceneGraph will not compile successfully when OSG_WINDOWING_SYSTEM is
Carbon and either x86_64 or ppc64 is selected under CMAKE_OSX_ARCHITECTURES,
as Carbon is a 32bit only API. A version of the osgviewer library written in
Cocoa is needed. However, OSG may be compiled under 64-bits if the X11
windowing system is selected. However, Two parts of the OSG default
distribution will not work with 64-bit X11: the osgviewerWX example
program and the osgdb_qt (Quicktime) plugin. These must be removed from
the Xcode project after Cmake generates it in order to compile with
64-bit architectures. The lack of the latter means that images such as
jpeg, tiff, png, and gif will not work, nor will animations dependent on
Quicktime. A new ImageIO-based plugin is being developed to handle the
still images, and a QTKit plugin will need to be developed to handle
animations.
