#!/bin/bash
#!/bin/sh

#####################################################################
# Author: Eric Wing
# 
# This is a stripped down version of my original Build_OSG_OSX.sh
# script. This just copies all the already built binaries 
# into a structure that is near-ready for distribution.
# 
# Usage: You should run this from the directory above the 3-projects
# ("AnyDirectory" in the picture below)
# bash OpenSceneGraph/Xcode/Misc/ArrangeDMG.sh
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
#####################################################################

# CpMac is deprecated now that cp works properly in Tiger
#COPY="/Developer/Tools/CpMac -r"
COPY="/bin/cp -R"
#COPY="mv -f"

#BUILDACTION="clean build"
CONFIGURATION="Deployment"

GDAL_LOCATION="/Library/Frameworks"

# Clean up from previous builds?
echo "Deleteing PackageDir to begin anew"
rm -rf PackageDir
#rm -f OpenSceneGraph.dmg

# Make a directory that will hold all the things to be distributed.
mkdir -p PackageDir
# Make a subdirectory in PackageDir that will hold all the Frameworks
mkdir -p PackageDir/Frameworks
# Make a subdirectory in PackageDir that will hold all the osgPlugins
mkdir -p PackageDir/PlugIns

mkdir -p PackageDir/Examples

mkdir -p PackageDir/XcodeTemplates

mkdir -p PackageDir/Resources


# Everything should be built now. Move all the things to be distrubuted 
# to the PackageDir with the appropriate layout.

echo "Copying Frameworks..."

$COPY OpenThreads/Xcode/OpenThreads/build/$CONFIGURATION/OpenThreads.framework PackageDir/Frameworks
$COPY Producer/Xcode/Producer/build/$CONFIGURATION/Producer.framework PackageDir/Frameworks
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/osg*.framework PackageDir/Frameworks/

# Copy the gdal framework 
$COPY $GDAL_LOCATION/gdal.framework PackageDir/Frameworks

echo "Copying PlugIns..."
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/*.so PackageDir/PlugIns/

echo "Copying Examples..."
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/*.app PackageDir/Examples/

echo "Copying Xcode templates..."
$COPY OpenSceneGraph/Xcode/XcodeTemplates PackageDir
# If we are in CVS, all the CVS junk got copied in so we need to remove it
find -d PackageDir/XcodeTemplates -name CVS -exec rm -rf {} \;


echo "Copying License and ReadMe files..."
$COPY OpenThreads/COPYING.txt PackageDir/LICENSE_OpenThreads.txt
$COPY Producer/LICENSE.txt PackageDir/LICENSE_Producer.txt
$COPY OpenSceneGraph/LICENSE.txt PackageDir/LICENSE_OSG.txt
$COPY OpenSceneGraph/Xcode/OSX_OSG_README.rtf PackageDir



# Sorry, I think this is bourne only
echo "Setting up symbolic links for the .app's..."
(cd PackageDir/Examples
	for file in *.app; do
#		echo ${file}
		(cd "$file/Contents"; \
			ln -s ../../../Frameworks/ Frameworks; \
			ln -s ../../../PlugIns/ PlugIns; \
			ln -s ../../../Resources/ Resources;
		)
	done
)
#echo ""


# Not sure how to find the OSG data, so it has to be done manually
# Would Spotlight help?
echo "Next, you must (manually) copy all the OSG-data to PackageDir/Resources and the LICENCE_GDAL.rtf to PackageDir."

echo "Looking up location for OpenSceneGraph-Data"
#/usr/bin/perl OpenSceneGraph/Xcode/Misc/FindOSGData.pl
/usr/bin/perl OpenSceneGraph/Xcode/Misc/FindOSGData.pl --single
echo "Looking up location for LICENSE_GDAL.rtf"
/usr/bin/perl OpenSceneGraph/Xcode/Misc/FindOSGData.pl --single LICENSE_GDAL.rtf

echo "After you copy the remaining resources, you will want to package up the DMG. You can use the following line as the basis:"
echo "hdiutil create -ov -fs HFS+ -volname OpenSceneGraph -srcfolder PackageDir OpenSceneGraph.dmg"


# Now we want to package up everything into a .dmg
#hdiutil create -ov -fs HFS+ -volname OpenSceneGraph -srcfolder PackageDir OpenSceneGraph.dmg


