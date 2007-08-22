#!/bin/bash
#!/bin/sh

#####################################################################
# Author: Eric Wing
# 
# This is a stripped down version of my original Build_OSG_OSX.sh
# script. This just copies all the already built binaries 
# into a structure that is near-ready for distribution.
# 
# Usage: You should run this from the directory above the OSG-projects
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
# that OpenThreads and OpenSceneGraph exist in.
# 
#####################################################################

# CpMac is deprecated now that cp works properly in Tiger
#COPY="/Developer/Tools/CpMac -r"
COPY="/bin/cp -R"
SVN="svn"
#COPY="mv -f"

#BUILDACTION="clean build"
#CONFIGURATION="Development"
CONFIGURATION="Deployment"

#GDAL_LOCATION="/Library/Frameworks"

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

mkdir -p PackageDir/.background


# Everything should be built now. Move all the things to be distrubuted 
# to the PackageDir with the appropriate layout.

echo "Copying Frameworks..."

#$COPY OpenThreads/Xcode/OpenThreads/build/$CONFIGURATION/OpenThreads.framework PackageDir/Frameworks
#$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/osg*.framework PackageDir/Frameworks/
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/*.framework PackageDir/Frameworks/

# Copy the gdal framework 
#$COPY $GDAL_LOCATION/gdal.framework PackageDir/Frameworks

echo "Copying PlugIns..."
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/*.so PackageDir/PlugIns/

echo "Copying Examples..."
$COPY OpenSceneGraph/Xcode/OpenSceneGraph/build/$CONFIGURATION/*.app PackageDir/Examples/

echo "Copying Xcode templates..."
$COPY OpenSceneGraph/Xcode/XcodeTemplates PackageDir
# If we are in CVS, all the CVS junk got copied in so we need to remove it
#find -d PackageDir/XcodeTemplates -name CVS -exec rm -rf {} \;
find -d PackageDir/XcodeTemplates -name .svn -exec rm -rf {} \;


echo "Copying License and ReadMe files..."
#$COPY OpenThreads/COPYING.txt PackageDir/LICENSE_OpenThreads.txt
$COPY OpenSceneGraph/LICENSE.txt PackageDir/LICENSE_OSG.txt
$COPY OpenSceneGraph/Xcode/OSX_OSG_README.rtf PackageDir

# Copy the background image and .DS_Store for 'fancy' DMG
$COPY OpenSceneGraph/Xcode/Packaging/Resources/instlogo.pdf PackageDir/.background
$COPY OpenSceneGraph/Xcode/Misc/DSStoreForDMG PackageDir/.DS_Store


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

echo "Testing for OpenSceneGraph-Data..."


# If OpenSceneGraph-Data/ resides next to OpenSceneGraph/, then use it
if [ -d OpenSceneGraph-Data ]; then
	echo "Found OpenSceneGraph-Data and will copy into PackageDir/Resources."
	# Determine if it is a subversion copy or not; we don't want the repo info
	if [ -d OpenSceneGraph-Data/.svn ]; then
		$SVN export --force OpenSceneGraph-Data PackageDir/Resources	
	else
		$COPY OpenSceneGraph-Data PackageDir/Resources
	fi

	echo "Creating DMG using:"
	echo "hdiutil create -ov -fs HFS+ -volname OpenSceneGraph -srcfolder PackageDir OpenSceneGraph.dmg"
	/usr/bin/hdiutil create -ov -fs HFS+ -volname OpenSceneGraph -srcfolder PackageDir OpenSceneGraph.dmg

else
	# Not sure how to find the OSG data, so it has to be done manually
	# Would Spotlight help?
	#echo "Next, you must (manually) copy all the OSG-data to PackageDir/Resources and the LICENCE_GDAL.rtf to PackageDir."
	echo "Did not find OpenSceneGraph-Data/ aside OpenSceneGraph/."
	echo "Next, you must (manually) copy all the OSG-data to PackageDir/Resources."

	echo "Looking up possible location for OpenSceneGraph-Data"
	#/usr/bin/perl OpenSceneGraph/Xcode/Misc/FindOSGData.pl
	/usr/bin/perl OpenSceneGraph/Xcode/Misc/FindOSGData.pl --single
	#echo "Looking up location for LICENSE_GDAL.rtf"
	#/usr/bin/perl OpenSceneGraph/Xcode/Misc/FindOSGData.pl --single LICENSE_GDAL.rtf

	echo "After you copy the remaining resources, you will want to package up the DMG. You can use the following line as the basis:"
	echo "hdiutil create -ov -fs HFS+ -volname OpenSceneGraph -srcfolder PackageDir OpenSceneGraph.dmg"
fi


# Now we want to package up everything into a .dmg
#hdiutil create -ov -fs HFS+ -volname OpenSceneGraph -srcfolder PackageDir OpenSceneGraph.dmg


