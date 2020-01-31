[![Build Status](https://travis-ci.org/openscenegraph/OpenSceneGraph.svg?branch=master)](https://travis-ci.org/openscenegraph/OpenSceneGraph)
[![Coverity Status](https://scan.coverity.com/projects/9159/badge.svg)](https://scan.coverity.com/projects/openscenegraph-openscenegraph)
[![Documentation](https://codedocs.xyz/openscenegraph/OpenSceneGraph.svg)](https://codedocs.xyz/openscenegraph/OpenSceneGraph/)
[ABI Tracker](https://abi-laboratory.pro/tracker/timeline/openscenegraph/ "ABI Tracker")

# Introduction

Welcome to the OpenSceneGraph (OSG).

For up-to-date information on the project, in-depth details on how to compile and run libraries and examples, see the documentation on the OpenSceneGraph website:

    http://www.openscenegraph.org/index.php/documentation

For support subscribe to our public mailing list or forum, details at:

    http://www.openscenegraph.org/index.php/support

For the impatient, we've included quick build instructions below, these are are broken down is three parts:

  1) General notes on building the OpenSceneGraph
  2) macOS release notes
  3) iOS release notes

If details below are not sufficient then head over to the openscenegraph.org to the Documentation/GettingStarted and Documentation/PlatformSpecifics sections for more indepth instructions.

Robert Osfield.
Project Lead.
31th January 2020.

---

## Section 1. How to build OpenSceneGraph

If you are using the [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager you can download and install OpenSceneGraph from source with CMake integration using a single command:
```
vcpkg install osg
```

The OpenSceneGraph uses the CMake build system to generate a platform-specific build environment.  CMake reads the `CMakeLists.txt` files that you'll find throughout the OpenSceneGraph directories, checks for installed dependencies and then generates files for the selected build system.

If you don't already have CMake installed on your system you can grab it from http://www.cmake.org, use version 2.8.0 or later.  Details on the OpenSceneGraph's CMake build can be found at:

    http://www.openscenegraph.org/projects/osg/wiki/Build/CMake

Under Unix-like systems (i.e. Linux, IRIX, Solaris, Free-BSD, HP-UX, AIX, macOS) use the `cmake` or `ccmake` command-line utils. Note that `cmake .` defaults to building Release to ensure that you get the best performance from your final libraries/applications.

    cd OpenSceneGraph
    cmake .
    make
    sudo make install

Alternatively, you can create an out-of-source build directory and run cmake or ccmake from there. The advantage to this approach is that the temporary files created by CMake won't clutter the OpenSceneGraph source directory, and also makes it possible to have multiple independent build targets by creating multiple build directories. In a directory alongside the OpenSceneGraph use:

    mkdir build
    cd build
    cmake ../OpenSceneGraph
    make
    sudo make install

Under Windows use the GUI tool CMakeSetup to build your VisualStudio files. The following page on our wiki dedicated to the CMake build system should help guide you through the process:

    http://www.openscenegraph.org/index.php/documentation/platform-specifics/windows

Under macOS you can either use the CMake build system above, or use the Xcode projects that you will find in the OpenSceneGraph/Xcode directory. See release notes on macOS CMake build below.

For further details on compilation, installation and platform-specific information read "Getting Started" guide:

    http://www.openscenegraph.org/index.php/documentation/10-getting-started


## Section 2. Release notes on macOS build, by Eric Sokolowski et al.

There are two ways to compile OpenSceneGraph under macOS.  The recommended way is to use CMake to generate Xcode project files and then use Xcode to build the library. The default project will be able to build Debug or Release libraries, examples, and sample applications.

The alternative is to build OpenSceneGraph from the command line using `make` or `ninja` using the instructions for Unix-like systems above.

Here are some key settings to consider when using CMake:

- BUILD_OSG_EXAMPLES - By default this is turned off. Turn this setting on to compile many great example programs.
- CMAKE_OSX_ARCHITECTURES - Xcode can create applications, executables, libraries, and frameworks that can be run on more than one architecture. Use this setting to indicate the architectures on which to build OSG. x86_64 is the only supported value for OS versions > 10.7.
- OSG_BUILD_APPLICATION_BUNDLES - Normally only executable binaries are created for the examples and sample applications. Turn this option on if you want to create real macOS .app bundles. There are caveats to creating `.app` bundles, see below.
- OSG_DEFAULT_IMAGE_PLUGIN_FOR_OSX - By default macOS uses the `imageio` plugin instead of the plugins for the individual file types (e.g. `jpg`, `gif`, etc.) to load image file types. The `imageio` plugin can handle all popular file formats through the ImageIO framework.
- OSG_WINDOWING_SYSTEM - You have the choice to use Cocoa, Carbon, or X11 when building applications on macOS. Cocoa is the default for OS versions >= 10.5. Carbon and X11 are no longer actively supported, either by Apple or the OSG community.


### APPLICATION BUNDLES (.app bundles)

The example programs when built as application bundles only contain the executable file. They do not contain the dependent libraries as would a normal bundle, so they are not generally portable to other machines.
They also do not know where to find plugins. An environmental variable OSG_LIBRARY_PATH may be set to point to the location where the plugin .so files are located. OSG_FILE_PATH may be set to point to the location where data files are located. Setting OSG_FILE_PATH to the OpenSceneGraph-Data directory is very useful when testing OSG by running the example programs.

Many of the example programs use command-line arguments. When double-clicking on an application (or using the equivalent "open" command on the command line) only those examples and applications that do not require command-line arguments will successfully run. The executable file within the .app bundle can be run from the command-line if command-line arguments are needed.


## Section 3. Release notes on iOS build, by Thomas Hogarth

With CMake 3.11, XCode 9.4 and the iOS sdk 11.4 installed you can generate an iOS XCode project using the following command line:

    export THIRDPARTY_PATH=/path/to/3rdParty
    cmake ./ -G Xcode -DOSG_BUILD_PLATFORM_IPHONE:BOOL=ON \
    -DIPHONE_SDKVER="11.4" \
    -DIPHONE_VERSION_MIN="10.0" \
    -DOPENGL_PROFILE:STRING=GLES3 \
    -DOSG_CPP_EXCEPTIONS_AVAILABLE:BOOL=ON \
    -DBUILD_OSG_APPLICATIONS:BOOL=OFF \
    -DBUILD_OSG_EXAMPLES:BOOL=ON \
    -DOSG_WINDOWING_SYSTEM:STRING=IOS \
    -DOSG_DEFAULT_IMAGE_PLUGIN_FOR_OSX="imageio" \
    -DDYNAMIC_OPENSCENEGRAPH:BOOL=OFF \
    -DDYNAMIC_OPENTHREADS:BOOL=OFF \
    -DCURL_INCLUDE_DIR:PATH="$THIRDPARTY_PATH/curl-ios-device/include" \
    -DCURL_LIBRARY:PATH="$THIRDPARTY_PATH/curl-ios-device/lib/libcurl.a" \
    -DFREETYPE_INCLUDE_DIR_freetype2:PATH="$THIRDPARTY_PATH/freetype-ios-universal/include/freetype" \
    -DFREETYPE_INCLUDE_DIR_ft2build:PATH="$THIRDPARTY_PATH/freetype-ios-universal/include" \
    -DFREETYPE_LIBRARY:PATH="$THIRDPARTY_PATH/freetype-ios-universal/lib/libFreetype2.a" \
    -DTIFF_INCLUDE_DIR:PATH="$THIRDPARTY_PATH/tiff-ios-device/include" \
    -DTIFF_LIBRARY:PATH="$THIRDPARTY_PATH/tiff-ios-device/lib/libtiff.a" \
    -DGDAL_INCLUDE_DIR:PATH="$THIRDPARTY_PATH/gdal-ios-device/include" \
    -DGDAL_LIBRARY:PATH="$THIRDPARTY_PATH/gdal-ios-device/lib/libgdal.a"


Be sure to set the THIRDPARTY_PATH to the path containing your thirdparty dependencies. Set IPHONE_SDKVER to the version of the iOS sdk you have installed, in this instance 11.4. IPHONE_VERSION_MIN controls the deployment sdk used by xcode, and lastly set OPENGL_PROFILE to the version of GLES you want to use.

Once this completes an XCode project will have been generated in the osg root folder. Open the generated Xcode project, select the example_osgViewerIPhone target. In 'General' tab set a development team.

Once this is done you should be able to build and deploy the `example_osgViewerIPhone` target on your device.
