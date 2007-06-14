#This Script Installs the files from the Xcode build into the correct location.

#echo Copy Examples
#cp -r ../OpenSceneGraph/build/Development/*.app Binary/Examples/

echo Install Applications
sudo cp build/Development/osgviewer.app/Contents/MacOS/osgviewer /usr/local/bin/
sudo cp build/Development/osgarchive.app/Contents/MacOS/osgarchive /usr/local/bin/
sudo cp build/Development/osgversion.app/Contents/MacOS/osgversion /usr/local/bin/
sudo cp build/Development/osgconv.app/Contents/MacOS/osgconv /usr/local/bin/
sudo cp build/Development/osgmovie.app/Contents/MacOS/osgmovie /usr/local/bin/

echo Install Frameworks
cp -r build/Development/*.framework ~/Library/Frameworks/

echo Install Plugins
cp build/Development/*.so ~/Library/Application\ Support/OpenSceneGraph/PlugIns/
cp build/Development/osgtext ~/Library/Application\ Support/OpenSceneGraph/PlugIns/
cp build/Development/osgpick ~/Library/Application\ Support/OpenSceneGraph/PlugIns/

echo Install Complete
