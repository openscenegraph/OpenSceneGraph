#include "daeReader.h"
#include "domSourceReader.h"
#include <dae.h>
#include <dae/domAny.h>
#include <dom/domCOLLADA.h>
#include <dom/domConstants.h>

#include <osgAnimation/Channel>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/StackedTransform>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/UpdateMatrixTransform>

using namespace osgDAE;


// Mapping Collada animations to Osg animations
// domAnimation ->  osgAnimation::Animation
// domSampler   ->  osgAnimation::Channel
// domSource    ->  osgAnimation::Channel.Sampler
// domChannel   ->  osgAnimation::Channel.TargetName
osgAnimation::BasicAnimationManager* daeReader::processAnimationLibraries(domCOLLADA* document)
{
    domLibrary_animation_clips_Array domAnimationClipsLibraries = document->getLibrary_animation_clips_array();

    domLibrary_animations_Array domAnimationsLibraries = document->getLibrary_animations_array();
    osgAnimation::BasicAnimationManager* pOsgAnimationManager = NULL;

    // Only create an animationmanager if we have  animation clip libraries or animation libraries
    if ((domAnimationClipsLibraries.getCount() > 0) || (domAnimationsLibraries.getCount() > 0))
    {
        pOsgAnimationManager = new osgAnimation::BasicAnimationManager();

        // Process all animation clip libraries
        for (size_t i=0; i < domAnimationClipsLibraries.getCount(); i++)
        {
            domAnimation_clip_Array domAnimationClips = domAnimationClipsLibraries[i]->getAnimation_clip_array();
            // Process all animation clips in this library
            for (size_t j=0; j < domAnimationClips.getCount(); j++)
            {
                processAnimationClip(pOsgAnimationManager, domAnimationClips[j]);
            }
        }

        // If there are no clips then all animations are part of the same clip
        if (domAnimationClipsLibraries.getCount() == 0 && domAnimationsLibraries.getCount())
        {
            osgAnimation::Animation* pOsgAnimation = new osgAnimation::Animation;
            pOsgAnimation->setName("Default");
            pOsgAnimationManager->registerAnimation(pOsgAnimation);

            // Process all animation libraries
            for (size_t i=0; i < domAnimationsLibraries.getCount(); i++)
            {
                domAnimation_Array domAnimations = domAnimationsLibraries[i]->getAnimation_array();

                TargetChannelPartMap tcm;

                // Process all animations in this library
                for (size_t j=0; j < domAnimations.getCount(); j++)
                {
                    processAnimationChannels(domAnimations[j], tcm);
                }

                processAnimationMap(tcm, pOsgAnimation);
            }
        }
    }
    return pOsgAnimationManager;
}

// <animation_clip (id) (name) (start) (end)>
// 0..1 <asset>
// 1..* <instance_animation>
// 0..* <extra>
void daeReader::processAnimationClip(osgAnimation::BasicAnimationManager* pOsgAnimationManager, domAnimation_clip* pDomAnimationClip)
{
    // an <animation_clip> groups animations
    osgAnimation::Animation* pOsgAnimation = new osgAnimation::Animation;
    std::string name = pDomAnimationClip->getId() ? pDomAnimationClip->getId() : "AnimationClip";
    pOsgAnimation->setName(name);

    // We register the animation inside the scheduler
    pOsgAnimationManager->registerAnimation(pOsgAnimation);

    double start = pDomAnimationClip->getStart();
    double end = pDomAnimationClip->getEnd();

    pOsgAnimation->setStartTime(start);
    double duration = end - start;
    if (duration > 0)
    {
        pOsgAnimation->setDuration(duration);
    }

    TargetChannelPartMap tcm;

    // 1..* <instance_animation>
    domInstanceWithExtra_Array domInstanceArray = pDomAnimationClip->getInstance_animation_array();
    for (size_t i=0; i < domInstanceArray.getCount(); i++)
    {
        domAnimation *pDomAnimation = daeSafeCast<domAnimation>(getElementFromURI(domInstanceArray[i]->getUrl()));
        if (pDomAnimation)
        {
            processAnimationChannels(pDomAnimation, tcm);
        }
        else
        {
            OSG_WARN << "Failed to locate animation " << domInstanceArray[i]->getUrl().getURI() << std::endl;
        }
    }

    processAnimationMap(tcm, pOsgAnimation);
}

struct KeyFrameComparator
{
    bool operator () (const osgAnimation::Keyframe& a, const osgAnimation::Keyframe& b) const
    {
        return a.getTime() < b.getTime();
    }
    bool operator () (const osgAnimation::Keyframe& a, float t) const
    {
        return a.getTime() < t;
    }
    bool operator () (float t, const osgAnimation::Keyframe& b) const
    {
        return t < b.getTime();
    }
};

template <typename T>
void deCasteljau(osgAnimation::TemplateCubicBezier<T>& l, osgAnimation::TemplateCubicBezier<T>& n, osgAnimation::TemplateCubicBezier<T>& r, float t)
{
    T q1 = l.getPosition() + t * (l.getControlPointOut() - l.getPosition());
    T q2 = l.getControlPointOut() + t * (r.getControlPointIn() - l.getControlPointOut());
    T q3 = r.getControlPointIn() + t * (r.getPosition() - r.getControlPointIn());

    T r1 = q1 + t * (q2 - q1);
    T r2 = q2 + t * (q3 - q2);

    T s = r1 + t * (r2 - r1);

    n.setControlPointIn(r1);

    n.setPosition(s);

    n.setControlPointOut(r2);

    l.setControlPointOut(q1);

    r.setControlPointIn(q3);
}

void mergeKeyframeContainers(osgAnimation::Vec3CubicBezierKeyframeContainer* to,
                    osgAnimation::FloatCubicBezierKeyframeContainer** from,
                    daeReader::InterpolationType interpolationType,
                    const osg::Vec3& defaultValue)
{
    assert(to->empty());

    typedef std::set<float> TimeSet;
    TimeSet times;

    for (int i = 0; i < 3; ++i)
    {
        if (!from[i] || from[i]->empty())
        {
            continue;
        }

        for (osgAnimation::FloatCubicBezierKeyframeContainer::const_iterator
            it = from[i]->begin(), end = from[i]->end(); it != end; ++it)
        {
            times.insert(it->getTime());
        }
    }

    for (TimeSet::const_iterator it = times.begin(), end = times.end(); it != end; ++it)
    {
        const float time = *it;

        osgAnimation::Vec3CubicBezier value(defaultValue);

        for (int i = 0; i < 3; ++i)
        {
            if (!from[i] || from[i]->empty())
            {
                continue;
            }

            osgAnimation::FloatCubicBezierKeyframeContainer::iterator next =
                std::lower_bound(from[i]->begin(), from[i]->end(), time, KeyFrameComparator());
            if (next == from[i]->end())
            {
                --next;
                value.getPosition().ptr()[i] = next->getValue().getPosition();
                value.getControlPointIn().ptr()[i] = next->getValue().getControlPointIn();
                value.getControlPointOut().ptr()[i] = next->getValue().getControlPointOut();
            }
            else if (next == from[i]->begin() || next->getTime() == time)
            {
                value.getPosition().ptr()[i] = next->getValue().getPosition();
                value.getControlPointIn().ptr()[i] = next->getValue().getControlPointIn();
                value.getControlPointOut().ptr()[i] = next->getValue().getControlPointOut();
            }
            else
            {
                osgAnimation::FloatCubicBezierKeyframeContainer::iterator prev = next;
                --prev;

                switch (interpolationType)
                {
                case daeReader::INTERPOLATION_STEP:
                    value.getPosition().ptr()[i] = prev->getValue().getPosition();
                    break;
                case daeReader::INTERPOLATION_LINEAR:
                    {
                        float xp = prev->getTime(), xn = next->getTime();
                        float yp = prev->getValue().getPosition(), yn = next->getValue().getPosition();
                        value.getPosition().ptr()[i] = yp + (yn - yp) * (time - xp) / (xn - xp);
                    }
                    break;
                case daeReader::INTERPOLATION_BEZIER:
                    {
                        float xp = prev->getTime(), xn = next->getTime();

                        osgAnimation::FloatCubicBezier l(prev->getValue()), n, r(next->getValue());
                        deCasteljau(l, n, r, (time - xp) / (xn - xp));

                        value.getPosition().ptr()[i] = n.getPosition();
                        value.getControlPointIn().ptr()[i] = n.getControlPointIn();
                        value.getControlPointOut().ptr()[i] = n.getControlPointOut();

                        osgAnimation::Vec3CubicBezier prevValue = to->back().getValue();
                        prevValue.getControlPointOut().ptr()[i] = l.getControlPointOut();
                        to->back().setValue(prevValue);

                        prev->setValue(l);
                        next->setValue(r);
                        from[i]->insert(next, osgAnimation::FloatCubicBezierKeyframe(time, n));
                    }
                    break;
                default:
                    OSG_WARN << "Unsupported interpolation type." << std::endl;
                    break;
                }

                //todo - different types of interpolation
            }
        }

        to->push_back(osgAnimation::Vec3CubicBezierKeyframe(time, value));
    }
}

void daeReader::processAnimationChannels(
    domAnimation* pDomAnimation, TargetChannelPartMap& tcm)
{
    // 1..* <source>
    SourceMap sources;
    domSource_Array domSources = pDomAnimation->getSource_array();
    for (size_t i=0; i < domSources.getCount(); i++)
    {
        sources.insert(std::make_pair((daeElement*)domSources[i], domSourceReader(domSources[i])));
    }

    domChannel_Array domChannels = pDomAnimation->getChannel_array();
    for (size_t i=0; i < domChannels.getCount(); i++)
    {
        processChannel(domChannels[i], sources, tcm);
    }

    domAnimation_Array domAnimations = pDomAnimation->getAnimation_array();
    for (size_t i=0; i < domAnimations.getCount(); i++)
    {
        // recursively call
        processAnimationChannels(domAnimations[i], tcm);
    }
}

osgAnimation::Vec3KeyframeContainer* convertKeyframeContainerToLinear(osgAnimation::Vec3CubicBezierKeyframeContainer& from)
{
    osgAnimation::Vec3KeyframeContainer* linearKeyframes = new osgAnimation::Vec3KeyframeContainer;
    for (size_t i = 0; i < from.size(); ++i)
    {
        linearKeyframes->push_back(osgAnimation::Vec3Keyframe(
            from[i].getTime(), from[i].getValue().getPosition()));
    }
    return linearKeyframes;
}

template <typename T>
void convertHermiteToBezier(osgAnimation::TemplateKeyframeContainer<T>& keyframes)
{
    for (size_t i = 0; i < keyframes.size(); ++i)
    {
        T val = keyframes[i].getValue();
        val.setControlPointIn(val.getControlPointIn() / 3.0f + val.getPosition());
        val.setControlPointOut(val.getControlPointOut() / -3.0f + val.getPosition());
        keyframes[i].setValue(val);
    }
}

// osgAnimation requires control points to be in a weird order. This function
// reorders them from the conventional order to osgAnimation order.
template <typename T>
void reorderControlPoints(osgAnimation::TemplateKeyframeContainer<osgAnimation::TemplateCubicBezier<T> >& vkfCont)
{
    if (vkfCont.size() <= 1)
    {
        if (vkfCont.size() == 1)
        {
            osgAnimation::TemplateCubicBezier<T> tcb = vkfCont.front().getValue();
            T inCP = tcb.getControlPointIn();
            tcb.setControlPointIn(tcb.getControlPointOut());
            tcb.setControlPointOut(inCP);
            vkfCont.front().setValue(tcb);
        }
        return;
    }

    osgAnimation::TemplateCubicBezier<T> first = vkfCont.front().getValue();

    for (unsigned i = 0; i < vkfCont.size() - 1; ++i)
    {
        osgAnimation::TemplateCubicBezier<T> tcb = vkfCont[i].getValue();
        tcb.setControlPointIn(tcb.getControlPointOut());
        tcb.setControlPointOut(vkfCont[i + 1].getValue().getControlPointIn());
        vkfCont[i].setValue(tcb);
    }

    osgAnimation::TemplateCubicBezier<T> last = vkfCont.back().getValue();
    last.setControlPointIn(last.getControlPointOut());
    last.setControlPointOut(first.getControlPointIn());
    vkfCont.back().setValue(last);
}

// <animation (id) (name)>
// 0..1 <asset>
// option 1
//        1..* <source>
//        one of (<sampler>, <channel>, <animation>) or <animation> (see below)
// option 2
//        1..* <source>
//        1..* <sampler>
//        0..* <animation>
// option 3
//        1..* <animation>
// 0..* <extra>
void daeReader::processAnimationMap(const TargetChannelPartMap& tcm, osgAnimation::Animation* pOsgAnimation)
{
    for (TargetChannelPartMap::const_iterator lb = tcm.begin(), end = tcm.end(); lb != end;)
    {
        TargetChannelPartMap::const_iterator ub = tcm.upper_bound(lb->first);

        osgAnimation::Channel* pOsgAnimationChannel = NULL;
        std::string channelName, targetName, componentName;

        if (osgAnimation::Vec3Target* pTarget = dynamic_cast<osgAnimation::Vec3Target*>(lb->first))
        {
            osgAnimation::FloatCubicBezierKeyframeContainer* fkfConts[3] = {NULL, NULL, NULL};
            osgAnimation::Vec3CubicBezierKeyframeContainer* vkfCont = NULL;
            InterpolationType interpolationType = INTERPOLATION_DEFAULT;

            for (TargetChannelPartMap::const_iterator it = lb; it != ub; ++it)
            {
                ChannelPart* channelPart = it->second.get();
                extractTargetName(channelPart->name, channelName, targetName, componentName);
                interpolationType = channelPart->interpolation;

                if (osgAnimation::Vec3CubicBezierKeyframeContainer* v3cnt = dynamic_cast<osgAnimation::Vec3CubicBezierKeyframeContainer*>(channelPart->keyframes.get()))
                {
                    vkfCont = v3cnt;
                    break;
                }
                else if (osgAnimation::FloatCubicBezierKeyframeContainer* fcnt = dynamic_cast<osgAnimation::FloatCubicBezierKeyframeContainer*>(channelPart->keyframes.get()))
                {
                    if (strchr("xusr0", tolower(componentName[0])))
                    {
                        fkfConts[0] = fcnt;
                    }
                    else if (strchr("yvtg1", tolower(componentName[0])))
                    {
                        fkfConts[1] = fcnt;
                    }
                    else if (strchr("zpb2", tolower(componentName[0])))
                    {
                        fkfConts[2] = fcnt;
                    }
                    else
                    {
                        OSG_WARN << "Unrecognised vector component \"" << componentName << "\"" << std::endl;
                    }
                }
                else
                {
                    OSG_WARN << "Unrecognised keyframe container \"" << channelPart->name << "\"" << std::endl;
                }
            }

            if (!pOsgAnimationChannel && (fkfConts[0] || fkfConts[1] || fkfConts[2]))
            {
                vkfCont = new osgAnimation::Vec3CubicBezierKeyframeContainer;
                mergeKeyframeContainers(vkfCont, fkfConts, interpolationType, pTarget->getValue());
            }

            if (vkfCont)
            {
                if (interpolationType == INTERPOLATION_STEP)
                {
                    osgAnimation::Vec3StepChannel* channel = new osgAnimation::Vec3StepChannel;
                    pOsgAnimationChannel = channel;
                    channel->getOrCreateSampler()->setKeyframeContainer(convertKeyframeContainerToLinear(*vkfCont));
                }
                else if (interpolationType == INTERPOLATION_LINEAR)
                {
                    osgAnimation::Vec3LinearChannel* channel = new osgAnimation::Vec3LinearChannel;
                    pOsgAnimationChannel = channel;
                    channel->getOrCreateSampler()->setKeyframeContainer(convertKeyframeContainerToLinear(*vkfCont));
                }
                else if (interpolationType == INTERPOLATION_BEZIER)
                {
                    osgAnimation::Vec3CubicBezierChannel* channel = new osgAnimation::Vec3CubicBezierChannel;
                    pOsgAnimationChannel = channel;
                    reorderControlPoints(*vkfCont);
                    channel->getOrCreateSampler()->setKeyframeContainer(vkfCont);
                }
                else if (interpolationType == INTERPOLATION_HERMITE)
                {
                    osgAnimation::Vec3CubicBezierChannel* channel = new osgAnimation::Vec3CubicBezierChannel;
                    pOsgAnimationChannel = channel;
                    convertHermiteToBezier(*vkfCont);
                    reorderControlPoints(*vkfCont);
                    channel->getOrCreateSampler()->setKeyframeContainer(vkfCont);
                }
                else
                {
                    OSG_WARN << "Unsupported interpolation type" << std::endl;
                }
            }
        }
        else
        {
            ChannelPart* channelPart = lb->second.get();
            extractTargetName(channelPart->name, channelName, targetName, componentName);

            typedef osgAnimation::TemplateKeyframe<osgAnimation::TemplateCubicBezier<osg::Matrixf> > MatrixCubicBezierKeyframe;
            typedef osgAnimation::TemplateKeyframeContainer<osgAnimation::TemplateCubicBezier<osg::Matrixf> > MatrixCubicBezierKeyframeContainer;

            if (osgAnimation::FloatCubicBezierKeyframeContainer* kfCntr =
                dynamic_cast<osgAnimation::FloatCubicBezierKeyframeContainer*>(channelPart->keyframes.get()))
            {
                if (dynamic_cast<osgAnimation::MatrixTarget*>(lb->first))
                {
                    OSG_WARN << "Animating elements of matrices not supported." << std::endl;
                }
                else
                {
                    osgAnimation::FloatCubicBezierChannel* channel = new osgAnimation::FloatCubicBezierChannel;
                    reorderControlPoints(*kfCntr);
                    channel->getOrCreateSampler()->setKeyframeContainer(kfCntr);
                    pOsgAnimationChannel = channel;
                }
            }
            else if (MatrixCubicBezierKeyframeContainer* cbkfCntr =
                dynamic_cast<MatrixCubicBezierKeyframeContainer*>(channelPart->keyframes.get()))
            {
                osgAnimation::MatrixKeyframeContainer* kfCntr = new osgAnimation::MatrixKeyframeContainer;
                for (size_t i = 0; i < cbkfCntr->size(); ++i)
                {
                    const MatrixCubicBezierKeyframe& cbkf = cbkfCntr->at(i);
                    kfCntr->push_back(osgAnimation::MatrixKeyframe(cbkf.getTime(), cbkf.getValue().getPosition()));
                }
                osgAnimation::MatrixLinearChannel* channel = new osgAnimation::MatrixLinearChannel;
                channel->getOrCreateSampler()->setKeyframeContainer(kfCntr);
                pOsgAnimationChannel = channel;
            }
        }

        if (pOsgAnimationChannel)
        {
            pOsgAnimationChannel->setTargetName(targetName);
            pOsgAnimationChannel->setName(channelName);
            pOsgAnimation->addChannel(pOsgAnimationChannel);
        }
        lb = ub;
    }

    pOsgAnimation->computeDuration();
}

template <typename T, typename TArray>
osgAnimation::KeyframeContainer* makeKeyframes(
    const osg::FloatArray* pOsgTimesArray,
    TArray* pOsgPointArray,
    TArray* pOsgInTanArray,
    TArray* pOsgOutTanArray,
    daeReader::InterpolationType& interpolationType)
{
    osgAnimation::TemplateKeyframeContainer<osgAnimation::TemplateCubicBezier<T> >* keyframes =
        new osgAnimation::TemplateKeyframeContainer<osgAnimation::TemplateCubicBezier<T> >;

    for (size_t i = 0; i < pOsgTimesArray->size(); i++)
    {
        T pt = (*pOsgPointArray)[i];
        T cpIn = pt, cpOut = pt;
        if (pOsgInTanArray)
        {
            if (interpolationType == daeReader::INTERPOLATION_HERMITE)
                //convert from hermite to bezier
                cpIn += (*pOsgInTanArray)[i] / 3;
            else if (interpolationType == daeReader::INTERPOLATION_BEZIER)
                cpIn = (*pOsgInTanArray)[i];
        }
        if (pOsgOutTanArray)
        {
            if (interpolationType == daeReader::INTERPOLATION_HERMITE)
                //convert from hermite to bezier
                cpOut += (*pOsgOutTanArray)[i] / 3;
            else if (interpolationType == daeReader::INTERPOLATION_BEZIER)
                cpOut = (*pOsgOutTanArray)[i];
        }

        keyframes->push_back(
            osgAnimation::TemplateKeyframe<osgAnimation::TemplateCubicBezier<T> >(
            (*pOsgTimesArray)[i],
            osgAnimation::TemplateCubicBezier<T>(pt, cpIn, cpOut)));
    }

    if (interpolationType == daeReader::INTERPOLATION_HERMITE)
    {
        interpolationType = daeReader::INTERPOLATION_BEZIER;
    }

    return keyframes;
}

// Sampler tells how to use the sources to generate an animated value.
// <sampler (id)>
// 1..* <input>
//        1    semantic
//        1    source
daeReader::ChannelPart* daeReader::processSampler(domChannel* pDomChannel, SourceMap &sources)
{
    // And we finally define our channel
    // Note osg can only create time based animations

    //from the channel you know the target, from the target you know the type
    domSampler *pDomSampler = daeSafeCast<domSampler>(getElementFromURI(pDomChannel->getSource()));
    if (!pDomSampler)
    {
        return NULL;
    }

    domInputLocal_Array domInputArray = pDomSampler->getInput_array();

    daeElement* input_source = NULL;
    daeElement* output_source = NULL;
    daeElement* output_intangent_source = NULL;
    daeElement* output_outtangent_source = NULL;
    domInputLocal *tmp;

    osg::FloatArray* pOsgTimesArray = NULL;
    if (findInputSourceBySemantic(domInputArray, COMMON_PROFILE_INPUT_INPUT, input_source, &tmp))
    {
        domSource* pDomSource = daeSafeCast<domSource>(input_source);
        if (pDomSource)
        {
            domSource::domTechnique_common* pDomTechnique = pDomSource->getTechnique_common();
            if (pDomTechnique)
            {
                domAccessor* pDomAccessor = pDomTechnique->getAccessor();
                domParam_Array domParams = pDomAccessor->getParam_array();
                if (domParams.getCount() > 0)
                {
                    if (!strcmp("TIME", domParams[0]->getName()))
                    {
                        pOsgTimesArray = sources[input_source].getArray<osg::FloatArray>();
                    }
                    else
                    {
                        OSG_WARN << "Only TIME based animations are supported" <<std::endl;
                        return NULL;
                    }
                }
                else
                {
                    OSG_WARN << "No params in accessor" <<std::endl;
                    return NULL;
                }
            }
            else
            {
                OSG_WARN << "Unable to find <technique_common> in <source> " << pDomSource->getName() <<std::endl;
                return NULL;
            }
        }
        else
        {
            OSG_WARN << "Could not get animation 'INPUT' source"<<std::endl;
            return NULL;
        }
    }

    //const bool readDoubleKeyframes = (_precisionHint & osgDB::Options::DOUBLE_PRECISION_KEYFRAMES) != 0;
    static const bool readDoubleKeyframes = false;

    findInputSourceBySemantic(domInputArray, COMMON_PROFILE_INPUT_OUTPUT, output_source, &tmp);
    findInputSourceBySemantic(domInputArray, COMMON_PROFILE_INPUT_IN_TANGENT, output_intangent_source, &tmp);
    findInputSourceBySemantic(domInputArray, COMMON_PROFILE_INPUT_OUT_TANGENT, output_outtangent_source, &tmp);
    domSourceReader::ArrayType arrayType = sources[output_source].getArrayType(readDoubleKeyframes);

    struct InterpTypeName
    {
        InterpolationType interp;
        const char* str;
    };

    InterpTypeName interpTypeNames[] = {
        {INTERPOLATION_STEP, "STEP"},
        {INTERPOLATION_LINEAR, "LINEAR"},
        {INTERPOLATION_BEZIER, "BEZIER"},
        {INTERPOLATION_HERMITE, "HERMITE"},
        {INTERPOLATION_CARDINAL, "CARDINAL"},
        {INTERPOLATION_BSPLINE, "BSPLINE"}
    };
    const int interpTypeCount = sizeof(interpTypeNames) / sizeof(*interpTypeNames);

    // TODO multiple outputs may be possible?

    InterpolationType interpolationType = INTERPOLATION_DEFAULT;

    if (findInputSourceBySemantic(domInputArray, COMMON_PROFILE_INPUT_INTERPOLATION, input_source, &tmp))
    {
        domSource* pDomSource = daeSafeCast<domSource>(input_source);
        if (pDomSource)
        {
            domName_array* pDomNames = pDomSource->getName_array();
            if (pDomNames)
            {
                daeStringArray* stringArray = &(pDomNames->getValue());

                // Take a look at the first element in the array to see what kind of interpolation is needed
                // multiple interpolation types inside an animation is not supported
                if (stringArray->getCount() > 0)
                {
                    // Collada interpolation types
                    for (int i = 0; i < interpTypeCount; ++i)
                    {
                        if (!strcmp(interpTypeNames[i].str, (*stringArray)[0]))
                        {
                            interpolationType = interpTypeNames[i].interp;
                            break;
                        }
                    }
                }
                else
                {
                    OSG_WARN << "No names in <Name_array>" <<std::endl;
                    return NULL;
                }
            }
            else
            {
                OSG_WARN << "Unable to find <Name_array> in <source> " << pDomSource->getName() <<std::endl;
                return NULL;
            }
        }
        else
        {
            OSG_WARN << "Could not get animation 'INPUT' source"<<std::endl;
            return NULL;
        }
    }

    //work around for files output by the Autodesk FBX converter.
    if ((interpolationType == INTERPOLATION_BEZIER) &&
        (_authoringTool == FBX_CONVERTER || _authoringTool == MAYA))
    {
        interpolationType = INTERPOLATION_HERMITE;
    }

    osgAnimation::KeyframeContainer* keyframes = NULL;

    switch (arrayType)
    {
    case domSourceReader::Float:
        keyframes = makeKeyframes<float>(pOsgTimesArray,
            sources[output_source].getArray<osg::FloatArray>(),
            sources[output_intangent_source].getArray<osg::FloatArray>(),
            sources[output_outtangent_source].getArray<osg::FloatArray>(),
            interpolationType);
        break;
    case domSourceReader::Vec2:
        keyframes = makeKeyframes<osg::Vec2>(pOsgTimesArray,
            sources[output_source].getArray<osg::Vec2Array>(),
            sources[output_intangent_source].getArray<osg::Vec2Array>(),
            sources[output_outtangent_source].getArray<osg::Vec2Array>(),
            interpolationType);
        break;
    case domSourceReader::Vec3:
        keyframes = makeKeyframes<osg::Vec3>(pOsgTimesArray,
            sources[output_source].getArray<osg::Vec3Array>(),
            sources[output_intangent_source].getArray<osg::Vec3Array>(),
            sources[output_outtangent_source].getArray<osg::Vec3Array>(),
            interpolationType);
        break;
    case domSourceReader::Vec4:
        keyframes = makeKeyframes<osg::Vec4>(pOsgTimesArray,
            sources[output_source].getArray<osg::Vec4Array>(),
            sources[output_intangent_source].getArray<osg::Vec4Array>(),
            sources[output_outtangent_source].getArray<osg::Vec4Array>(),
            interpolationType);
        break;
    case domSourceReader::Vec2d:
        keyframes = makeKeyframes<osg::Vec2d>(pOsgTimesArray,
            sources[output_source].getArray<osg::Vec2dArray>(),
            sources[output_intangent_source].getArray<osg::Vec2dArray>(),
            sources[output_outtangent_source].getArray<osg::Vec2dArray>(),
            interpolationType);
        break;
    case domSourceReader::Vec3d:
        keyframes = makeKeyframes<osg::Vec3d>(pOsgTimesArray,
            sources[output_source].getArray<osg::Vec3dArray>(),
            sources[output_intangent_source].getArray<osg::Vec3dArray>(),
            sources[output_outtangent_source].getArray<osg::Vec3dArray>(),
            interpolationType);
        break;
    case domSourceReader::Vec4d:
        keyframes = makeKeyframes<osg::Vec4d>(pOsgTimesArray,
            sources[output_source].getArray<osg::Vec4dArray>(),
            sources[output_intangent_source].getArray<osg::Vec4dArray>(),
            sources[output_outtangent_source].getArray<osg::Vec4dArray>(),
            interpolationType);
        break;
    case domSourceReader::Matrix:
        keyframes = makeKeyframes<osg::Matrixf>(pOsgTimesArray,
            sources[output_source].getArray<osg::MatrixfArray>(),
            sources[output_intangent_source].getArray<osg::MatrixfArray>(),
            sources[output_outtangent_source].getArray<osg::MatrixfArray>(),
            interpolationType);
        break;
    default:
        ;// Fall through
    }

    if (keyframes)
    {
        ChannelPart* chanPart = new ChannelPart;
        chanPart->keyframes = keyframes;
        chanPart->interpolation = interpolationType;
        chanPart->name = pDomChannel->getTarget();
        return chanPart;
    }

    return NULL;
}

osgAnimation::Target* findChannelTarget(osg::Callback* nc, const std::string& targetName, bool& rotation)
{
    if (osgAnimation::UpdateMatrixTransform* umt = dynamic_cast<osgAnimation::UpdateMatrixTransform*>(nc))
    {
        for (osgAnimation::StackedTransform::const_iterator
            it = umt->getStackedTransforms().begin(), end = umt->getStackedTransforms().end(); it != end; ++it)
        {
            osgAnimation::StackedTransformElement* te = it->get();
            if (te->getName() == targetName)
            {
                rotation = dynamic_cast<osgAnimation::StackedRotateAxisElement*>(te) != NULL;
                return te->getOrCreateTarget();
            }
        }
    }
    else if (!dynamic_cast<osgAnimation::UpdateMorph*>(nc))
    {
        OSG_WARN << "Unrecognised AnimationUpdateCallback" << std::endl;
    }

    return NULL;
}

void convertDegreesToRadians(osgAnimation::KeyframeContainer* pKeyframeContainer)
{
    if (osgAnimation::FloatKeyframeContainer* fkc =
        dynamic_cast<osgAnimation::FloatKeyframeContainer*>(pKeyframeContainer))
    {
        for (size_t i = 0; i < fkc->size(); ++i)
        {
            osgAnimation::FloatKeyframe& fk = (*fkc)[i];
            fk.setValue(osg::DegreesToRadians(fk.getValue()));
        }
    }
    else if (osgAnimation::FloatCubicBezierKeyframeContainer* fcbkc =
        dynamic_cast<osgAnimation::FloatCubicBezierKeyframeContainer*>(pKeyframeContainer))
    {
        for (size_t i = 0; i < fcbkc->size(); ++i)
        {
            osgAnimation::FloatCubicBezierKeyframe& fcbk = (*fcbkc)[i];
            osgAnimation::FloatCubicBezier fcb = fcbk.getValue();
            fcb.setPosition(osg::DegreesToRadians(fcb.getPosition()));
            fcb.setControlPointIn(osg::DegreesToRadians(fcb.getControlPointIn()));
            fcb.setControlPointOut(osg::DegreesToRadians(fcb.getControlPointOut()));
            fcbk.setValue(fcb);
        }
    }
    else
    {
        OSG_WARN << "Warning: rotation keyframes not converted to radians." << std::endl;
    }
}

// Channel connects animation output to parameter to animate
// <channel>
// 1 source
// 1 target
void daeReader::processChannel(domChannel* pDomChannel, SourceMap& sources, TargetChannelPartMap& tcm)
{
    domSampler *pDomSampler = daeSafeCast<domSampler>(getElementFromURI(pDomChannel->getSource()));
    if (pDomSampler)
    {
        ChannelPart* pChannelPart = processSampler(pDomChannel, sources);

        if (pChannelPart)
        {
            domChannelOsgAnimationUpdateCallbackMap::iterator iter = _domChannelOsgAnimationUpdateCallbackMap.find(pDomChannel);
            if (iter != _domChannelOsgAnimationUpdateCallbackMap.end())
            {
                osg::Callback* nc = iter->second.get();

                std::string channelName, targetName, componentName;
                extractTargetName(pDomChannel->getTarget(), channelName, targetName, componentName);
                //assert(targetName == nc->getName());
                bool bRotationChannel = false;
                if (osgAnimation::Target* pTarget = findChannelTarget(nc, channelName, bRotationChannel))
                {
                    if (bRotationChannel)
                    {
                        convertDegreesToRadians(pChannelPart->keyframes.get());
                    }
                    tcm.insert(TargetChannelPartMap::value_type(pTarget, pChannelPart));
                }
                else
                {
                    OSG_WARN << "Target \"" << channelName << "\" not found." << std::endl;
                }
            }
            else
            {
                OSG_WARN << "Could not locate UpdateCallback for <channel> target "  << pDomChannel->getTarget()<< std::endl;
            }
        }
        else
        {
            OSG_WARN << "<channel> source "  << pDomChannel->getSource().getURI() << " has no corresponding osgAnimation::Channel" << std::endl;
        }
    }
    else
    {
        OSG_WARN << "Could not locate <channel> source "  << pDomChannel->getSource().getURI() << std::endl;
    }
}

void daeReader::extractTargetName(const std::string& daeTarget, std::string& channelName, std::string& targetName, std::string& component)
{
    size_t slash = daeTarget.find_last_of("/");
    if (slash != std::string::npos)
    {
        // Handle /translation
        targetName = daeTarget.substr(0, slash);
        channelName = daeTarget.substr(slash+1, std::string::npos);
    }
    else
    {
        size_t parenthesis = daeTarget.find_last_of("(");
        size_t endpos = daeTarget.find_last_of(")");
        if (parenthesis != std::string::npos && endpos != std::string::npos)
        {
            // Handle (1)
            targetName = daeTarget.substr(0, parenthesis);
            channelName = daeTarget.substr(parenthesis+1, endpos - parenthesis - 1);
        }
        else
        {
            OSG_WARN << "Couldn't extract a proper name for <channel> target " << daeTarget << std::endl;
        }
    }

    size_t period = channelName.find_last_of(".");
    if (period != std::string::npos)
    {
        component = channelName.substr(period+1, std::string::npos);
        channelName = channelName.substr(0, period);
    }
    else
    {
        component.clear();

        size_t first_parenthesis = channelName.find_first_of("(");
        if (first_parenthesis != std::string::npos)
        {
            size_t open_parenthesis = first_parenthesis;

            do
            {
                if (open_parenthesis != first_parenthesis) component += ",";

                size_t close_parenthesis = channelName.find_first_of(")", open_parenthesis);
                component += channelName.substr(open_parenthesis+1, close_parenthesis-open_parenthesis-1);
                open_parenthesis = channelName.find_first_of("(", close_parenthesis);
            }
            while (open_parenthesis != std::string::npos);

            channelName = channelName.substr(0, first_parenthesis);
        }
    }
}
