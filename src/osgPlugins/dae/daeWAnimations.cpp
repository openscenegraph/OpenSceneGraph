/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */

#include "daeWriter.h"

#include <dom/domCOLLADA.h>

#include <dom/domNode.h>
#include <dom/domConstants.h>
#include <dom/domLibrary_cameras.h>
#include <dom/domLibrary_lights.h>
#include <dae/domAny.h>
//#include <dom/domVisual_scene.h>
//#include <dom/domLibrary_visual_scenes.h>

#include <osgSim/MultiSwitch>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/UpdateMatrixTransform>
#include <osg/Sequence>
#include <osg/Billboard>
#include <osg/CameraView>
#include <osgDB/ConvertUTF>


using namespace osgDAE;


void daeWriter::writeAnimations( osg::Node &wnode )
{
    const std::string nodeNameUTF( _pluginOptions.namesUseCodepage ? osgDB::convertStringFromCurrentCodePageToUTF8(wnode.getName()) : wnode.getName() );
    osg::Callback* ncb = wnode.getUpdateCallback();
    if (ncb)
    {
        osgAnimation::AnimationManagerBase* am = dynamic_cast<osgAnimation::AnimationManagerBase*>(ncb);
        if (am)
        {
            // Create library of animations if not existing yet
            if (!_domLibraryAnimations)
            {
                _domLibraryAnimations = daeSafeCast<domLibrary_animations>( dom->add( COLLADA_ELEMENT_LIBRARY_ANIMATIONS ) );
            }

            osgAnimation::AnimationList animationList = am->getAnimationList();
            for (size_t ai = 0; ai < animationList.size(); ai++)
            {
                domAnimation* pDomAnimation = daeSafeCast< domAnimation >( _domLibraryAnimations->add( COLLADA_ELEMENT_ANIMATION ) );
                domAnimation* pMainDomAnimation = pDomAnimation;

                osg::ref_ptr<osgAnimation::Animation> animation = animationList[ai];
                std::string animationName( animation->getName() );
                if (animationName.empty())
                    animationName = "animation";
                animationName = uniquify( animationName );
                pDomAnimation->setId(animationName.c_str());

                // Handle multiple channels within an animation
                osgAnimation::ChannelList animationChannels = animation->getChannels();
                for (size_t j=0; j < animationChannels.size(); j++)
                {
                    osgAnimation::Channel* channel = animationChannels[j].get();
                    std::string channelName( channel->getName() );
                    std::string channelNameUTF( _pluginOptions.namesUseCodepage ? osgDB::convertStringFromCurrentCodePageToUTF8(channelName) : channelName );

                    // Wrap each animation channel into it's own child <animation> when more than 1 channel
                    if (animationChannels.size() > 1)
                    {
                        pDomAnimation = daeSafeCast< domAnimation >( pMainDomAnimation->add( COLLADA_ELEMENT_ANIMATION ) );

                        if (channelName.empty())
                        {
                            channelName = "channel";
                            channelNameUTF = channelName;
                        }
                        animationName = uniquify( channelName );
                        pDomAnimation->setId(channelNameUTF.c_str());
                    }

                    std::string sourceName( channelNameUTF + "_sampler" );
                    std::string inputSourceName( channelNameUTF + "_input" );
                    std::string outputSourceName( channelNameUTF + "_output" );
                    std::string interpolationSourceName( channelNameUTF + "_interpolation" );

                    // Fill dom sources based on sampler
                    osgAnimation::Sampler* animationSampler = channel->getSampler();
                    osgAnimation::KeyframeContainer* kfc = animationSampler->getKeyframeContainer();
                    int size = kfc->size();

                    float valueStride = 1;

                    domListOfFloats timeValues;
                    domListOfFloats frameValues;
                    domListOfNames interpolationValues;

                    osgAnimation::Vec3KeyframeContainer* v3kc = dynamic_cast<osgAnimation::Vec3KeyframeContainer*>(kfc);
                    if (v3kc)
                    {
                        valueStride = 3;
                        for (size_t i=0; i < v3kc->size(); i++)
                        {
                            timeValues.append((*v3kc)[i].getTime());
                            osg::Vec3 vec = (*v3kc)[i].getValue();

                            // This needs some serious cleanup
                            if (channelNameUTF.find("euler") != std::string::npos)
                            {
                                frameValues.append(osg::RadiansToDegrees(vec.x()));
                                frameValues.append(osg::RadiansToDegrees(vec.y()));
                                frameValues.append(osg::RadiansToDegrees(vec.z()));
                            }
                            else
                            {
                                frameValues.append(vec.x());
                                frameValues.append(vec.y());
                                frameValues.append(vec.z());
                            }
                            interpolationValues.append("LINEAR");
                        }
                    }
                    osgAnimation::FloatKeyframeContainer* fkc = dynamic_cast<osgAnimation::FloatKeyframeContainer*>(kfc);
                    if (fkc)
                    {
                        valueStride = 1;
                        for (size_t i=0; i < fkc->size(); i++)
                        {
                            timeValues.append((*fkc)[i].getTime());
                            frameValues.append((*fkc)[i].getValue());
                            interpolationValues.append("LINEAR");
                        }
                    }

                    // time values
                    domSource* pDomSource = daeSafeCast< domSource >(pDomAnimation->add(COLLADA_ELEMENT_SOURCE));
                    pDomSource->setId(inputSourceName.c_str());

                    domFloat_array* pDomFloatArray = daeSafeCast< domFloat_array >(pDomSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
                    std::string inputArrayName = inputSourceName + "_array";
                    pDomFloatArray->setId(inputArrayName.c_str());
                    pDomFloatArray->setCount(size);
                    pDomFloatArray->setValue(timeValues);

                    domSource::domTechnique_common* pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                    domAccessor* pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                    std::string url = "#" + inputArrayName;
                    pDomAccessor->setSource(url.c_str());
                    pDomAccessor->setCount(size);
                    pDomAccessor->setStride(1);

                    domParam* pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                    pDomParam->setName("TIME");
                    pDomParam->setType(COLLADA_TYPE_FLOAT);

                    // <source> interpolator type
                    pDomSource = daeSafeCast< domSource >(pDomAnimation->add(COLLADA_ELEMENT_SOURCE));
                    pDomSource->setId(interpolationSourceName.c_str());

                    domName_array* pDomNameArray = daeSafeCast< domName_array >(pDomSource->add(COLLADA_ELEMENT_NAME_ARRAY));
                    std::string interpolationArrayName = interpolationSourceName + "_array";
                    pDomNameArray->setId(interpolationArrayName.c_str());
                    pDomNameArray->setCount(size);
                    pDomNameArray->setValue(interpolationValues);

                    // <technique_common>
                    pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                    // <accessor count=size stride="1">
                    pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                    url = "#" + interpolationArrayName;
                    pDomAccessor->setSource(url.c_str());
                    pDomAccessor->setCount(size);
                    pDomAccessor->setStride(1);

                    // <param name="INTERPOLATION" type="name"/>
                    pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                    pDomParam->setName("INTERPOLATION");
                    pDomParam->setType(COLLADA_TYPE_NAME);

                    // Split up access to the euler float array into three sources, because we need to target three separate transforms
                    if (channelNameUTF.find("euler") != std::string::npos)
                    {
                        pDomSource = daeSafeCast< domSource >(pDomAnimation->add(COLLADA_ELEMENT_SOURCE));
                        std::string outputSourceNameX = outputSourceName + "_X";
                        pDomSource->setId(outputSourceNameX.c_str());

                        pDomFloatArray = daeSafeCast< domFloat_array >(pDomSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
                        std::string outputArrayNameX = outputSourceNameX + "_array";
                        pDomFloatArray->setId(outputArrayNameX.c_str());

                        // TODO, flexible handling of different keyframe types, see osg exporter for inspiration
                        pDomFloatArray->setCount(size * valueStride);
                        pDomFloatArray->setValue(frameValues);

                        pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                        pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                        url = "#" + outputArrayNameX;
                        pDomAccessor->setSource(url.c_str());
                        pDomAccessor->setCount(size);
                        pDomAccessor->setStride(valueStride);
                        pDomAccessor->setOffset(0);
                        // <param name="ANGLE" type="float"/>
                        pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                        pDomParam->setName("ANGLE");
                        pDomParam->setType(COLLADA_TYPE_FLOAT);

                        pDomSource = daeSafeCast< domSource >(pDomAnimation->add(COLLADA_ELEMENT_SOURCE));
                        std::string outputSourceNameY = outputSourceName + "_Y";
                        pDomSource->setId(outputSourceNameY.c_str());

                        pDomFloatArray = daeSafeCast< domFloat_array >(pDomSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
                        std::string outputArrayNameY = outputSourceNameY + "_array";
                        pDomFloatArray->setId(outputArrayNameY.c_str());

                        pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                        pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                        url = "#" + outputArrayNameY;
                        pDomAccessor->setSource(url.c_str());
                        pDomAccessor->setCount(size);
                        pDomAccessor->setStride(valueStride);
                        pDomAccessor->setOffset(1);
                        // <param name="ANGLE" type="float"/>
                        pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                        pDomParam->setName("ANGLE");
                        pDomParam->setType(COLLADA_TYPE_FLOAT);

                        pDomSource = daeSafeCast< domSource >(pDomAnimation->add(COLLADA_ELEMENT_SOURCE));
                        std::string outputSourceNameZ = outputSourceName + "_Z";
                        pDomSource->setId(outputSourceNameZ.c_str());

                        pDomFloatArray = daeSafeCast< domFloat_array >(pDomSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
                        std::string outputArrayNameZ = outputSourceNameZ + "_array";
                        pDomFloatArray->setId(outputArrayNameZ.c_str());

                        pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                        pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                        url = "#" + outputArrayNameZ;
                        pDomAccessor->setSource(url.c_str());
                        pDomAccessor->setCount(size);
                        pDomAccessor->setStride(valueStride);
                        pDomAccessor->setOffset(2);
                        // <param name="ANGLE" type="float"/>
                        pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                        pDomParam->setName("ANGLE");
                        pDomParam->setType(COLLADA_TYPE_FLOAT);

                        {
                            domSampler* pDomSampler = daeSafeCast< domSampler >(pDomAnimation->add(COLLADA_ELEMENT_SAMPLER));
                            std::string sourceNameX = sourceName + "_X";
                            pDomSampler->setId(sourceNameX.c_str());

                            // Fill dom sampler based on common semantics
                            {
                                domInputLocal* pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_INPUT);
                                url = "#" + inputSourceName;
                                pDomInput->setSource(url.c_str());

                                pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_OUTPUT);
                                url = "#" + outputSourceNameX;
                                pDomInput->setSource(url.c_str());

                                pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_INTERPOLATION);
                                url = "#" + interpolationSourceName;
                                pDomInput->setSource(url.c_str());
                            }

                            // Set sampler as source
                            domChannel* pDomChannel = daeSafeCast< domChannel >(pDomAnimation->add(COLLADA_ELEMENT_CHANNEL));
                            url = "#" + sourceNameX;
                            pDomChannel->setSource(url.c_str());

                            // targetName contains the name of the updateCallback
                            std::string targetName = channel->getTargetName();

                            // based on the type of updateCallback we can determine the transform element to target eg. "nodeName/translation"
                            osg::Node* node = _animatedNodeCollector.getTargetNode(targetName);
                            if (node)
                            {
                                std::string domChannelTargetName = nodeNameUTF + "/rotateX.ANGLE";
                                pDomChannel->setTarget(domChannelTargetName.c_str());
                            }
                            else
                            {
                                OSG_WARN << "Could not find animation target '" << targetName << "'" << std::endl;
                            }
                        }

                        {
                            domSampler* pDomSampler = daeSafeCast< domSampler >(pDomAnimation->add(COLLADA_ELEMENT_SAMPLER));
                            std::string sourceNameY = sourceName + "_Y";
                            pDomSampler->setId(sourceNameY.c_str());

                            // Fill dom sampler based on common semantics
                            {
                                domInputLocal* pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_INPUT);
                                url = "#" + inputSourceName;
                                pDomInput->setSource(url.c_str());

                                pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_OUTPUT);
                                url = "#" + outputSourceNameY;
                                pDomInput->setSource(url.c_str());

                                pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_INTERPOLATION);
                                url = "#" + interpolationSourceName;
                                pDomInput->setSource(url.c_str());
                            }

                            // Set sampler as source
                            domChannel* pDomChannel = daeSafeCast< domChannel >(pDomAnimation->add(COLLADA_ELEMENT_CHANNEL));
                            url = "#" + sourceNameY;
                            pDomChannel->setSource(url.c_str());

                            // targetName contains the name of the updateCallback
                            std::string targetName = channel->getTargetName();

                            // based on the type of updateCallback we can determine the transform element to target eg. "nodeName/translation"
                            osg::Node* node = _animatedNodeCollector.getTargetNode(targetName);
                            if (node)
                            {
                                std::string domChannelTargetName = nodeNameUTF + "/rotateY.ANGLE";
                                pDomChannel->setTarget(domChannelTargetName.c_str());
                            }
                            else
                            {
                                OSG_WARN << "Could not find animation target '" << targetName << "'" << std::endl;
                            }
                        }

                        {
                            domSampler* pDomSampler = daeSafeCast< domSampler >(pDomAnimation->add(COLLADA_ELEMENT_SAMPLER));
                            std::string sourceNameZ = sourceName + "_Z";
                            pDomSampler->setId(sourceNameZ.c_str());

                            // Fill dom sampler based on common semantics
                            {
                                domInputLocal* pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_INPUT);
                                url = "#" + inputSourceName;
                                pDomInput->setSource(url.c_str());

                                pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_OUTPUT);
                                url = "#" + outputSourceNameZ;
                                pDomInput->setSource(url.c_str());

                                pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                                pDomInput->setSemantic(COMMON_PROFILE_INPUT_INTERPOLATION);
                                url = "#" + interpolationSourceName;
                                pDomInput->setSource(url.c_str());
                            }

                            // Set sampler as source
                            domChannel* pDomChannel = daeSafeCast< domChannel >(pDomAnimation->add(COLLADA_ELEMENT_CHANNEL));
                            url = "#" + sourceNameZ;
                            pDomChannel->setSource(url.c_str());

                            // targetName contains the name of the updateCallback
                            std::string targetName = channel->getTargetName();

                            // based on the type of updateCallback we can determine the transform element to target eg. "nodeName/translation"
                            osg::Node* node = _animatedNodeCollector.getTargetNode(targetName);
                            if (node)
                            {
                                std::string domChannelTargetName = nodeNameUTF + "/rotateZ.ANGLE";
                                pDomChannel->setTarget(domChannelTargetName.c_str());
                            }
                            else
                            {
                                OSG_WARN << "Could not find animation target '" << targetName << "'" << std::endl;
                            }
                        }
                    }
                    else
                    {
                        // values in keyframecontainer
                        pDomSource = daeSafeCast< domSource >(pDomAnimation->add(COLLADA_ELEMENT_SOURCE));
                        pDomSource->setId(outputSourceName.c_str());

                        pDomFloatArray = daeSafeCast< domFloat_array >(pDomSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
                        std::string outputArrayName = outputSourceName + "_array";
                        pDomFloatArray->setId(outputArrayName.c_str());

                        // TODO, flexible handling of different keyframe types, see osg exporter for inspiration
                        pDomFloatArray->setCount(size * valueStride);
                        pDomFloatArray->setValue(frameValues);

                        pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                        pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                        url = "#" + outputArrayName;
                        pDomAccessor->setSource(url.c_str());
                        pDomAccessor->setCount(size);
                        pDomAccessor->setStride(valueStride);

                        if (v3kc)
                        {
                            // <param name="X" type="float"/>
                            pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                            pDomParam->setName("X");
                            pDomParam->setType(COLLADA_TYPE_FLOAT);
                            // <param name="Y" type="float"/>
                            pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                            pDomParam->setName("Y");
                            pDomParam->setType(COLLADA_TYPE_FLOAT);
                            // <param name="Z" type="float"/>
                            pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                            pDomParam->setName("Z");
                            pDomParam->setType(COLLADA_TYPE_FLOAT);
                        }
                        if (fkc)
                        {
                            // <param type="float"/>
                            pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                            pDomParam->setType(COLLADA_TYPE_FLOAT);
                        }

                        domSampler* pDomSampler = daeSafeCast< domSampler >(pDomAnimation->add(COLLADA_ELEMENT_SAMPLER));
                        pDomSampler->setId(sourceName.c_str());

                        // Fill dom sampler based on common semantics
                        {
                            domInputLocal* pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                            pDomInput->setSemantic(COMMON_PROFILE_INPUT_INPUT);
                            url = "#" + inputSourceName;
                            pDomInput->setSource(url.c_str());

                            pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                            pDomInput->setSemantic(COMMON_PROFILE_INPUT_OUTPUT);
                            url = "#" + outputSourceName;
                            pDomInput->setSource(url.c_str());

                            pDomInput = daeSafeCast< domInputLocal >(pDomSampler->add(COLLADA_ELEMENT_INPUT));
                            pDomInput->setSemantic(COMMON_PROFILE_INPUT_INTERPOLATION);
                            url = "#" + interpolationSourceName;
                            pDomInput->setSource(url.c_str());
                        }

                        // Set sampler as source
                        domChannel* pDomChannel = daeSafeCast< domChannel >(pDomAnimation->add(COLLADA_ELEMENT_CHANNEL));
                        url = "#" + sourceName;
                        pDomChannel->setSource(url.c_str());

                        // targetName contains the name of the updateCallback
                        std::string targetName = channel->getTargetName();

                        // based on the type of updateCallback we can determine the transform element to target eg. "nodeName/translation"
                        osg::Node* node = _animatedNodeCollector.getTargetNode(targetName);
                        if (node)
                        {
                            std::string domChannelTargetName = nodeNameUTF;

                            if (channelNameUTF.find("position") != std::string::npos)
                            {
                                domChannelTargetName += "/translate";
                            }
                            else if (channelNameUTF.find("scale") != std::string::npos)
                            {
                                domChannelTargetName += "/scale";
                            }

                            pDomChannel->setTarget(domChannelTargetName.c_str());
                        }
                        else
                        {
                            OSG_WARN << "Could not find animation target '" << targetName << "'" << std::endl;
                        }
                    }
                }
            }
        }
    }
}

