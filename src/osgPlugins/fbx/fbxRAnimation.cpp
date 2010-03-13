#include <osg/MatrixTransform>

#include <osgAnimation/Animation>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Channel>
#include <osgAnimation/Sampler>

#if defined(_MSC_VER)
    #pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>
#include <fbxfilesdk/fbxfilesdk_nsuse.h>

osg::Quat makeQuat(const fbxDouble3&, ERotationOrder);

osg::Quat makeQuat(const osg::Vec3& radians, ERotationOrder fbxRotOrder)
{
    fbxDouble3 degrees(
        osg::RadiansToDegrees(radians.x()),
        osg::RadiansToDegrees(radians.y()),
        osg::RadiansToDegrees(radians.z()));
    return makeQuat(degrees, fbxRotOrder);
}

void readKeys(KFCurve* curveX, KFCurve* curveY, KFCurve* curveZ,
              std::vector<osgAnimation::TemplateKeyframe<osg::Vec3> >& keyFrameCntr, float scalar = 1.0f)
{
    KFCurve* curves[3] = {curveX, curveY, curveZ};

    typedef std::set<float> TimeSet;
    typedef std::map<float, float> TimeFloatMap;
    TimeSet times;
    TimeFloatMap curveTimeMap[3];

    for (int nCurve = 0; nCurve < 3; ++nCurve)
    {
        KFCurve* pCurve = curves[nCurve];
        
        int nKeys = pCurve->KeyGetCount();

        if (!nKeys)
        {
            times.insert(0.0f);
            curveTimeMap[nCurve][0.0f] = static_cast<float>(pCurve->GetValue()) * scalar;
        }

        for (int i = 0; i < nKeys; ++i)
        {
            KFCurveKey key = pCurve->KeyGet(i);
            float fTime = static_cast<float>(key.GetTime().GetSecondDouble());
            times.insert(fTime);
            curveTimeMap[nCurve][fTime] = static_cast<float>(key.GetValue()) * scalar;
        }
    }

    for (TimeSet::iterator it = times.begin(); it != times.end(); ++it)
    {
        float fTime = *it;
        osg::Vec3 val;
        for (int i = 0; i < 3; ++i)
        {
            if (curveTimeMap[i].empty()) continue;

            TimeFloatMap::iterator lb = curveTimeMap[i].lower_bound(fTime);
            if (lb == curveTimeMap[i].end()) --lb;
            val[i] = lb->second;
        }
        keyFrameCntr.push_back(osgAnimation::Vec3Keyframe(fTime, val));
    }
}

osgAnimation::Channel* readFbxChannels(KFCurve* curveX, KFCurve* curveY,
    KFCurve* curveZ, const char* targetName, const char* channelName)
{
    if (!(curveX && curveX->KeyGetCount()) &&
        !(curveY && curveY->KeyGetCount()) &&
        !(curveZ && curveZ->KeyGetCount()))
    {
        return 0;
    }

    osgAnimation::Vec3LinearChannel* pChannel = new osgAnimation::Vec3LinearChannel;
    osgAnimation::Vec3KeyframeContainer* pKeyFrameCntr =
        pChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();

    pChannel->setTargetName(targetName);
    pChannel->setName(channelName);
    readKeys(curveX, curveY, curveZ, *pKeyFrameCntr);

    return pChannel;
}

osgAnimation::Channel* readFbxChannels(
    KFbxTypedProperty<fbxDouble3>& fbxProp, const char* pTakeName,
    const char* targetName, const char* channelName)
{
    if (!fbxProp.IsValid()) return 0;

    return readFbxChannels(
        fbxProp.GetKFCurve("X", pTakeName),
        fbxProp.GetKFCurve("Y", pTakeName),
        fbxProp.GetKFCurve("Z", pTakeName),
        targetName, channelName);
}

osgAnimation::Channel* readFbxChannelsQuat(
    KFCurve* curveX, KFCurve* curveY, KFCurve* curveZ, const char* targetName,
    ERotationOrder rotOrder)
{
    if (!(curveX && curveX->KeyGetCount()) &&
        !(curveY && curveY->KeyGetCount()) &&
        !(curveZ && curveZ->KeyGetCount()))
    {
        return 0;
    }

    osgAnimation::QuatSphericalLinearChannel* pChannel = new osgAnimation::QuatSphericalLinearChannel;
    pChannel->setTargetName(targetName);
    pChannel->setName("quaternion");
    typedef std::vector<osgAnimation::TemplateKeyframe<osg::Vec3> > KeyFrameCntr;
    KeyFrameCntr eulerFrameCntr;
    readKeys(curveX, curveY, curveZ, eulerFrameCntr, static_cast<float>(osg::PI / 180.0));

    osgAnimation::QuatSphericalLinearSampler::KeyframeContainerType& quatFrameCntr =
        *pChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();
    quatFrameCntr.reserve(eulerFrameCntr.size());

    for (KeyFrameCntr::iterator it = eulerFrameCntr.begin(), end = eulerFrameCntr.end();
        it != end; ++it)
    {
        const osg::Vec3& euler = it->getValue();
        quatFrameCntr.push_back(osgAnimation::QuatKeyframe(
            it->getTime(), makeQuat(euler, rotOrder)));
    }

    return pChannel;
}

osgAnimation::Animation* addChannels(
    osgAnimation::Channel* pTranslationChannel,
    osgAnimation::Channel* pRotationChannel,
    osgAnimation::Channel* pScaleChannel,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimManager,
    const char* pTakeName)
{
    if (pTranslationChannel ||
        pRotationChannel ||
        pScaleChannel)
    {
        if (!pAnimManager) pAnimManager = new osgAnimation::BasicAnimationManager;

        osgAnimation::Animation* pAnimation = 0;
        const osgAnimation::AnimationList& anims = pAnimManager->getAnimationList();
        for (size_t i = 0; i < anims.size(); ++i)
        {
            if (anims[i]->getName() == pTakeName)
            {
                pAnimation = anims[i].get();
            }
        }

        if (!pAnimation)
        {
            pAnimation = new osgAnimation::Animation;
            pAnimation->setName(pTakeName);
            pAnimManager->registerAnimation(pAnimation);
        }

        if (pTranslationChannel) pAnimation->addChannel(pTranslationChannel);
        if (pRotationChannel) pAnimation->addChannel(pRotationChannel);
        if (pScaleChannel) pAnimation->addChannel(pScaleChannel);


        return pAnimation;
    }

    return 0;
}

osgAnimation::Animation* readFbxAnimation(KFbxNode* pNode,
    const char* pTakeName, const char* targetName,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimManager)
{
    if (!pTakeName)
    {
        return 0;
    }

    ERotationOrder rotOrder = pNode->RotationOrder.IsValid() ? pNode->RotationOrder.Get() : eEULER_XYZ;

    osgAnimation::Channel* pTranslationChannel = 0;
    osgAnimation::Channel* pRotationChannel = 0;

    if (pNode->LclRotation.IsValid())
    {
        fbxDouble3 fbxBaseValue = pNode->LclRotation.Get();

        pRotationChannel = readFbxChannelsQuat(
            pNode->LclRotation.GetKFCurve(KFCURVENODE_R_X, pTakeName),
            pNode->LclRotation.GetKFCurve(KFCURVENODE_R_Y, pTakeName),
            pNode->LclRotation.GetKFCurve(KFCURVENODE_R_Z, pTakeName),
            targetName, rotOrder);
    }

    if (pNode->LclTranslation.IsValid())
    {
        pTranslationChannel = readFbxChannels(
            pNode->LclTranslation.GetKFCurve(KFCURVENODE_T_X, pTakeName),
            pNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Y, pTakeName),
            pNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Z, pTakeName),
            targetName, "translate");
    }

    osgAnimation::Channel* pScaleChannel = readFbxChannels(
        pNode->LclScaling, pTakeName, targetName, "scale");

    return addChannels(pTranslationChannel, pRotationChannel, pScaleChannel, pAnimManager, pTakeName);
}

std::string readFbxAnimation(KFbxNode* pNode,
    osg::ref_ptr<osgAnimation::AnimationManagerBase>& pAnimManager,
    const char* targetName)
{
    std::string result;
    for (int i = 1; i < pNode->GetTakeNodeCount(); ++i)
    {
        const char* pTakeName = pNode->GetTakeNodeName(i);
        if (osgAnimation::Animation* pAnimation = readFbxAnimation(
            pNode, pTakeName, targetName, pAnimManager))
        {
            result = targetName;
        }
    }
    return result;
}
