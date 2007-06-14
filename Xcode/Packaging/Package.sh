echo Copy Examples
cp -r ../OpenSceneGraph/build/Development/*.app Binary/Examples/

echo Copy Applications
mv Binary/Examples/osgviewer.app Binary/Applications
mv Binary/Examples/osgarchive.app Binary/Applications
mv Binary/Examples/osgversion.app Binary/Applications
mv Binary/Examples/osgconv.app Binary/Applications
mv Binary/Applications/osgviewer.app/Contents/MacOS/osgviewer Binary/Applications
mv Binary/Applications/osgarchive.app/Contents/MacOS/osgarchive Binary/Applications
mv Binary/Applications/osgversion.app/Contents/MacOS/osgversion Binary/Applications
mv Binary/Applications/osgconv.app/Contents/MacOS/osgconv Binary/Applications
rm -rf Binary/Applications/osg*.app

echo Copy Frameworks
cp -r ../OpenSceneGraph/build/Development/*.framework Binary/Frameworks/

echo Copy Plugins
cp ../OpenSceneGraph/build/Development/*.so Binary/Plugins/
cp ../OpenSceneGraph/build/Development/osgtext Binary/Plugins/
cp ../OpenSceneGraph/build/Development/osgpick Binary/Plugins/

echo Copying Complete

echo Package files...
echo "build these in order first to last!"
open OpenSceneGraph.pmproj
open Packages/Examples.pmproj
open Packages/PlugIns.pmproj
open Packages/Frameworks.pmproj
open Packages/Applications.pmproj
