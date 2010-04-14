#ifndef FBXRANIMATION_H
#define FBXRANIMATION_H

#include <fbxfilesdk/fbxfilesdk_def.h>

std::string readFbxAnimation(
    KFbxNode*,
    KFbxScene& fbxScene,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>&,
    const char* targetName);

#endif
