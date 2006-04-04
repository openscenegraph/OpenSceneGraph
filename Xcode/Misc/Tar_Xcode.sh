#!/bin/sh

# Must be run from the directory above OpenThreads, Producer, and OpenSceneGraph


(cd OpenThreads; \
	rm -f XcodeOpenThreads.tar.gz; \
	tar -zcvf XcodeOpenThreads.tar.gz --exclude='*.pbxuser' --exclude='*.mode1' --exclude='*.perspective' --exclude='build' --exclude='.DS_Store' Xcode/;
)

(cd Producer; \
	rm -f XcodeProducer.tar.gz 
	tar -zcvf XcodeProducer.tar.gz --exclude='*.pbxuser' --exclude='*.mode1' --exclude='*.perspective' --exclude='build' --exclude='.DS_Store' Xcode/
)

(cd OpenSceneGraph; \
	rm -f XcodeOpenSceneGraph.tar.gz
	tar -zcvf XcodeOpenSceneGraph.tar.gz --exclude='*.pbxuser' --exclude='*.mode1' --exclude='*.perspective' --exclude='build' --exclude='.DS_Store' Xcode/
)

rm -f md5list.txt
md5 OpenThreads/XcodeOpenThreads.tar.gz >> md5list.txt
md5 Producer/XcodeProducer.tar.gz >> md5list.txt
md5 OpenSceneGraph/XcodeOpenSceneGraph.tar.gz >> md5list.txt

mkdir -p PackageDir/Xcode

rm -f PackageDir/Xcode/XcodeOpenThreads.tar.gz
rm -f PackageDir/Xcode/XcodeProducer.tar.gz
rm -f PackageDir/Xcode/XcodeOpenSceneGraph.tar.gz
rm -f PackageDir/Xcode/md5list.txt

mv md5list.txt PackageDir/Xcode/
cp OpenThreads/XcodeOpenThreads.tar.gz PackageDir/Xcode/
cp Producer/XcodeProducer.tar.gz PackageDir/Xcode/
cp OpenSceneGraph/XcodeOpenSceneGraph.tar.gz PackageDir/Xcode/


