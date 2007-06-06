Welcome to the OpenSceneGraph (OSG).

For up to date information on the project, how to indepth details on how to 
compile and run libraries and examples, and see the documentation on the 
OpenSceneGraph website.

    http://www.openscenegraph.org
  
For the impatient, read the simplified build notes below.

Robert Osfield.
Project Lead.
6th June 2007.

--

Notes for 1.9.8 release
=======================

OpenThreads/include and src directories has now been merged directly into 
the OpenSceneGraph distribution, this means that you don't need to download,
compile or install it, and will be able to remove the external OpenThreads
from your system.

--

How to build the OpenSceneGraph
===============================

The OpenSceneGraph use the CMake build system to generate platform specific
build environment.  CMake reads the CMakeLists.txt files that you'll find 
throughout the OpenSceneGraph directories, check for installed dependnecies
and then generate the appropriate build system.

If you don't already have CMake installed on your system you can grab it
from http://www.cmake.org, version 2.4.6 or later.

Under unices (i.e. Linux, IRIX, Solaris, Free-BSD, HP-Ux, AIX, OSX) use the
cmake or ccmake commandline utils, or use the included tiny configure script 
that'll run cmake for you.  The configure script simply runs 
'cmake . -DCMAKE_BUILD_TYPE=Release' to ensure that you get the best 
performance from your final libriaries/applications.
 
  cd OpenSceneGraph
  ./configure
  make
  sudo make install
  
Alternatively, you can create an out of source build directory and run configure
from there. The advantage to this approach is that the temporary files
created by CMake won't clutter the OpenSceneGraph source directory, also makes 
it possilble to build multiple build targets by creating multiple build 
directories. In a directory alongside the OpenSceneGraph use:

  mkdir build
  cd build
  cmake ../OpenSceneGraph -DCMAKE_BUILD_TYPE=Release
  make
  sudo make install

Under Windows use the GUI tool CMakeSetup to build your VisualStudio files. 
The following page on our wiki dedicated to the CMake build should help
guide you through the process:

   http://www.openscenegraph.com/index.php?page=Build.CMake

Under OSX you can either use the CMake build system above, or use the Xcode 
projects that you will find in OpenSceneGraph/Xcode.

For further details on compiliation, installation and platform specific information
read "Getting Started" at http://www.openscenegraph.org, under 
"Documentation".
