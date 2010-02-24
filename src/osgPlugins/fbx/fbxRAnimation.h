#ifndef FBXRANIMATION_H
#define FBXRANIMATION_H

#include <fbxfilesdk/fbxfilesdk_def.h>

std::string readFbxAnimation(
    FBXFILESDK_NAMESPACE::KFbxNode*,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>&,
    const char* targetName);

#endif
