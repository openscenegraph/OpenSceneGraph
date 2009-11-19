#ifndef FBXRCAMERA_H
#define FBXRCAMERA_H

#include <fbxfilesdk/fbxfilesdk_def.h>
#include <osgDB/ReaderWriter>

osgDB::ReaderWriter::ReadResult readFbxCamera(
    FBXFILESDK_NAMESPACE::KFbxNode* pNode);

#endif
