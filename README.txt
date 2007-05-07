Welcome to the OpenSceneGraph (OSG).

For up to date information on the project, how to indepth details on how to 
compile and run libraries and examples, and see the documentation on the 
OpenSceneGraph website.

    http://www.openscenegraph.org
  
For the impatient, read the simplified build notes below.

Robert Osfield.
Project Lead.
7th May 2007.

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
cmake or ccmake commandline utils:
 
  cd OpenSceneGraph
  ccmake .
  make
  sudo make install
  
Under Windows use the GUI tool CMakeSetup to build your VisualStudio files. 
The following page on our wiki dedicated to the CMake build should help
guide you through the process:

   http://www.openscenegraph.com/index.php?page=Build.CMake

For further details on compiliation, installation and platform specific information
read "Getting Started" at http://www.openscenegraph.org, under 
"Documentation".


