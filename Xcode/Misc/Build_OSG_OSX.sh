#!/bin/sh

#####################################################################
# Author: Eric Wing
# 
# This script will build OpenThreads, Producer, and OpenSceneGraph
# (using the Xcode projects I created for each) and package up 
# the Frameworks and PlugIns into a disk image (.dmg) for
# easy distribution. This script may be used towards automation
# of nightly builds.
# 
# The Xcode projects were designed so all three projects could be 
# built without any configuration or installation on the users end 
# if the directory structure followed this simple layout:
#
# AnyDirectory/
#   OpenThreads/
#     Xcode/
#       OpenThreads/
#         OpenThreads.xcode
#    Producer/
#      Xcode/
#        Producer/
#          Producer.xcode
#    OpenSceneGraph/
#      Xcode/
#        OpenSceneGraph/
#          OpenSceneGraph.xcode
#
# Simply put, the root directories for the 3 projects must be at the 
# same level. If you placed my Xcode tarball for each of 3 projects in
# each of the project's respective root directories, my projects should 
# extract themselves in the correct layout when you double click the 
# tarballs.
#
# You may place this script and run it from the same directory level 
# that OpenThreads, Producer, and OpenSceneGraph exist in.
# 
# The script will build each of the projects, and then move the built
# files to a temporary subdirectory called PackageDir. A disk image (.dmg)
# will then be created containing everything in PackageDir. The 
# disk image is designed to be easily redistrutable over the internet.
#
# The end user simply needs to download the disk image, mount it, and 
# then go to the Frameworks folder and drag and drop all the frameworks to 
# a standard location. These might be:
# ~/Library/Frameworks
# /Library/Frameworks
# Or you may place them inside your application bundle e.g.
# YourApp.app/Frameworks
#
# Plugins are somewhat problematic. Though they build correctly,
# the code to locate the plugins from the "stardard" locations is
# incomplete. Currently, optionally installed Plugins like Demeter
# are not built.
#
# Examples are even more problematic because they require the X11 
# system and currently cannot access the native Mac system. This 
# makes double click launching impossible. However, when the 
# day comes that Producer gets a native Mac backend, this will 
# all fall into place.
# 
# To build everything, you must have the Apple Developer Tools installed
# and you must also install Apple's X11 development package (if not 
# already installed). It can be found with the Developer Tools on the 
# last CD of the Operating System CDs.
#
# Also, since the code is C++, ABI issues apply. Since Panther is 
# gcc 3.3 based, the expectation is that the binaries produced 
# will require 3.3 compatibility.
#
#####################################################################

COPY="/Developer/Tools/CpMac -r"
#COPY="mv -f"

BUILDACTION="build"
#BUILDACTION="clean build"
CONFIGURATION="Deployment"

# Clean up from previous builds?
rm -rf PackageDir
rm OpenSceneGraph.dmg

# Make a directory that will hold all the things to be distributed.
mkdir -p PackageDir
# Make a subdirectory in PackageDir that will hold all the Frameworks
mkdir -p PackageDir/Frameworks
# Make a subdirectory in PackageDir that will hold all the osgPlugins
mkdir -p PackageDir/PlugIns

mkdir -p PackageDir/Examples

# First build OpenThreads
#	xcodebuild 	-project OpenThreads.xcode 	\
#				-buildstyle Deployment \
	# xcodebuild is the commandline tool that can build Xcode projects.
	# Specifying "clean build" will clean everything and then rebuild it.
	# Just specifying "build" will only rebuild things that need it.
(cd OpenThreads/Xcode/OpenThreads; \
	xcodebuild 	-project OpenThreads.xcodeproj 	\
				-target OpenThreads \
				-configuration $CONFIGURATION \
				$BUILDACTION \
				;
)

# Next build Producer
	# xcodebuild is the commandline tool that can build Xcode projects.
	# Specifying "clean build" will clean everything and then rebuild it.
	# Just specifying "build" will only rebuild things that need it.
#	xcodebuild 	-project Producer.xcode \
#				-buildstyle Deployment \
(cd Producer/Xcode/Producer; \
	xcodebuild 	-project Producer.xcodeproj \
				-target Producer \
				-configuration $CONFIGURATION \
				$BUILDACTION \
				;
)


# Now build OpenSceneGraph with everything
	# xcodebuild is the commandline tool that can build Xcode projects.
	# Specifying "clean build" will clean everything and then rebuild it.
	# Just specifying "build" will only rebuild things that need it.
#	xcodebuild 	-project OpenSceneGraph.xcode 	\
#				-buildstyle Deployment 			\
(cd OpenSceneGraph/Xcode/OpenSceneGraph; \
	xcodebuild 	-project OpenSceneGraph.xcodeproj 	\
				-target AllStandardTargets \
				-configuration $CONFIGURATION 			\
				$BUILDACTION \
				;
)

(cd OpenSceneGraph/Xcode/OpenSceneGraph; \
	xcodebuild 	-project OpenSceneGraph.xcodeproj 	\
				-target GDALdependentStuff \
				-configuration $CONFIGURATION 			\
				$BUILDACTION \
				;
)

# Everything should be built now. Move all the things to be distrubuted 
# to the PackageDir with the appropriate layout.
#$COPY OpenThreads/Xcode/OpenThreads/build/OpenThreads.framework PackageDir/Frameworks
#$COPY Producer/Xcode/Producer/build/Producer.framework PackageDir/Frameworks
#$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/osg*.framework PackageDir/Frameworks/

#$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/*.so PackageDir/PlugIns/
#$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/*.app PackageDir/Examples/

$COPY OpenThreads/Xcode/OpenThreads/build/$CONFIGURATION/OpenThreads.framework PackageDir/Frameworks
$COPY Producer/Xcode/Producer/build/$CONFIGURATION/Producer.framework PackageDir/Frameworks
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/osg*.framework PackageDir/Frameworks/

$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/*.so PackageDir/PlugIns/
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/*.app PackageDir/Examples/


# Now we want to package up everything into a .dmg
hdiutil create -ov -fs HFS+ -volname OpenSceneGraph -srcfolder PackageDir OpenSceneGraph.dmg


