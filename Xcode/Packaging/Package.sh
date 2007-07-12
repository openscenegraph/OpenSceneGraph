echo Copy Examples
cp -r ../OpenSceneGraph/build/Deployment/*.app Binary/Examples/

echo Copy Applications
mv Binary/Examples/osgviewer.app Binary/Applications
mv Binary/Examples/osgarchive.app Binary/Applications
mv Binary/Examples/osgversion.app Binary/Applications
mv Binary/Examples/osgconv.app Binary/Applications
cp Binary/Applications/*.app Binary/Applictions/Bundles/
cp Binary/Applications/osgviewer.app/Contents/MacOS/osgviewer Binary/Applications
cp Binary/Applications/osgarchive.app/Contents/MacOS/osgarchive Binary/Applications
cp Binary/Applications/osgversion.app/Contents/MacOS/osgversion Binary/Applications
cp Binary/Applications/osgconv.app/Contents/MacOS/osgconv Binary/Applications
mv Binary/Applications/osg*.app Binary/Applications/Bundles/


echo Copy Frameworks
cp -r ../OpenSceneGraph/build/Deployment/*.framework Binary/Frameworks/

echo Copy Plugins
cp ../OpenSceneGraph/build/Deployment/*.so Binary/Plugins/

echo Copying Complete

echo Package files...
echo "build these in order first to last!"
open OpenSceneGraph.pmproj
open Packages/Examples.pmproj
open Packages/PlugIns.pmproj
open Packages/Frameworks.pmproj
open Packages/ApplicationsBundles.pmproj
open Packages/ApplicationsCmdline.pmproj
