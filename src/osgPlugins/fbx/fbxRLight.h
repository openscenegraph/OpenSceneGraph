#ifndef FBXRLIGHT_H
#define FBXRLIGHT_H

#include <fbxfilesdk/fbxfilesdk_def.h>
#include <osgDB/ReaderWriter>

osgDB::ReaderWriter::ReadResult readFbxLight(
    FBXFILESDK_NAMESPACE::KFbxNode* pNode, int& nLightCount);

#endif
