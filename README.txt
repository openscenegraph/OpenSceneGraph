Welcome to the OpenSceneGraph (OSG).

For up-to-date information on the project, in-depth details on how to 
compile and run libraries and examples, see the documentation on the 
OpenSceneGraph website:

    http://www.openscenegraph.org
  
For the impatient, read the simplified build notes below.

Robert Osfield.
Project Lead.
6th June 2007.

--

Notes for 1.9.8 release
=======================

The OpenThreads/include and /src directories have now been merged 
directly into the OpenSceneGraph distribution. This means that you 
don't need to download, compile or install OpenThreads separately 
anymore. You can remove any external OpenThreads from your system.

--

How to build the OpenSceneGraph
===============================

The OpenSceneGraph uses the CMake build system to generate a 
platform-specific build environment.  CMake reads the CMakeLists.txt 
files that you'll find throughout the OpenSceneGraph directories, 
checks for installed dependenciesand then generates the appropriate 
build system.

If you don't already have CMake installed on your system you can grab 
it from http://www.cmake.org, use version 2.4.6 or later.

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

   http://www.openscenegraph.com/index.php?page=Build.CMake

Under OSX you can either use the CMake build system above, or use the 
Xcode projects that you will find in the OpenSceneGraph/Xcode 
directory.

For further details on compilation, installation and platform-specific 
information read "Getting Started" at http://www.openscenegraph.org, 
under "Documentation".
