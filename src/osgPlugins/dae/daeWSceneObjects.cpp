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
#include <osg/Sequence>
#include <osg/Billboard>
#include <osg/CameraView>

#ifdef COLLADA_DOM_2_4_OR_LATER
#include <dom/domAny.h>
using namespace ColladaDOM141;
#endif

using namespace osgDAE;

// Write non-standard node data as extra of type "Node" with "OpenSceneGraph" technique
void daeWriter::writeNodeExtra(osg::Node &node)
{
    unsigned int numDesc = node.getDescriptions().size();
    // Only create extra if descriptions are filled in
    if (_pluginOptions.writeExtras && (numDesc > 0))
    {
        // Adds the following to a node

        //<extra type="Node">
        //    <technique profile="OpenSceneGraph">
        //        <Descriptions>
        //            <Description>Some info</Description>
        //        </Descriptions>
        //    </technique>
        //</extra>

        domExtra *extra = daeSafeCast<domExtra>(currentNode->add( COLLADA_ELEMENT_EXTRA ));
        extra->setType("Node");
        domTechnique *teq = daeSafeCast<domTechnique>(extra->add( COLLADA_ELEMENT_TECHNIQUE ) );
        teq->setProfile( "OpenSceneGraph" );
        domAny *descriptions = (domAny*)teq->add( "Descriptions" );

        for (unsigned int currDesc = 0; currDesc < numDesc; currDesc++)
        {
            std::string value = node.getDescription(currDesc);
            if (!value.empty())
            {
                domAny *description = (domAny*)descriptions->add( "Description" );
                description->setValue(value.c_str());
            }
        }
    }
}

void daeWriter::apply( osg::Group &node )
{
    debugPrint( node );
    updateCurrentDaeNode();
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );

    // If a multiswitch node, store it's data as extra "MultiSwitch" data in the "OpenSceneGraph" technique
    osgSim::MultiSwitch* multiswitch = dynamic_cast<osgSim::MultiSwitch*>(&node);
    if (_pluginOptions.writeExtras && multiswitch)
    {
        // Adds the following to a node

        //<extra type="MultiSwitch">
        //    <technique profile="OpenSceneGraph">
        //        <ActiveSwitchSet>0</ActiveSwitchSet>
        //        <ValueLists>
        //            <ValueList>1 0</ValueList>
        //            <ValueList>0 1</ValueList>
        //        </ValueLists>
        //    </technique>
        //</extra>

        domExtra *extra = daeSafeCast<domExtra>(currentNode->add( COLLADA_ELEMENT_EXTRA ));
        extra->setType("MultiSwitch");
        domTechnique *teq = daeSafeCast<domTechnique>(extra->add( COLLADA_ELEMENT_TECHNIQUE ) );
        teq->setProfile( "OpenSceneGraph" );

        domAny *activeSwitchSet = (domAny*)teq->add("ActiveSwitchSet" );
        activeSwitchSet->setValue(toString<unsigned int>(multiswitch->getActiveSwitchSet()).c_str());

        domAny *valueLists = (domAny*)teq->add( "ValueLists" );

        unsigned int pos = 0;
        const osgSim::MultiSwitch::SwitchSetList& switchset = multiswitch->getSwitchSetList();
        for(osgSim::MultiSwitch::SwitchSetList::const_iterator sitr=switchset.begin();
            sitr!=switchset.end();
            ++sitr,++pos)
        {
            domAny *valueList = (domAny*)valueLists->add( "ValueList" );
            std::stringstream fw;
            const osgSim::MultiSwitch::ValueList& values = *sitr;
            for(osgSim::MultiSwitch::ValueList::const_iterator itr=values.begin();
                itr!=values.end();
                ++itr)
            {
                if (itr != values.begin())
                {
                    fw << " ";
                }
                fw << *itr;
            }
            valueList->setValue(fw.str().c_str());
        }
        currentNode->setId(getNodeName(node,"multiswitch").c_str());
    }
    else
    {
        writeAnimations(node);

        currentNode->setId(getNodeName(node,"group").c_str());
    }

    writeNodeExtra(node);

    lastDepth = _nodePath.size();

    traverse( node );
}


void daeWriter::apply( osg::Switch &node )
{
    debugPrint( node );
    updateCurrentDaeNode();
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"switch").c_str());

    if (_pluginOptions.writeExtras)
    {
        // Adds the following to a node

        //<extra type="Switch">
        //    <technique profile="OpenSceneGraph">
        //        <ValueList>1 0</ValueList>
        //    </technique>
        //</extra>

        domExtra *extra = daeSafeCast<domExtra>(currentNode->add( COLLADA_ELEMENT_EXTRA ));
        extra->setType("Switch");
        domTechnique *teq = daeSafeCast<domTechnique>(extra->add( COLLADA_ELEMENT_TECHNIQUE ) );
        teq->setProfile( "OpenSceneGraph" );

        domAny *valueList = (domAny*)teq->add( "ValueList" );

        std::stringstream fw;
        const osg::Switch::ValueList& values = node.getValueList();
        for(osg::Switch::ValueList::const_iterator itr=values.begin();
            itr!=values.end();
            ++itr)
        {
            if (itr != values.begin())
            {
                fw << " ";
            }
            fw << *itr;
        }
        valueList->setValue(fw.str().c_str());
    }

    writeNodeExtra(node);

    lastDepth = _nodePath.size();

    // Process all children
    traverse( node );
}

void daeWriter::apply( osg::Sequence &node )
{
    debugPrint( node );
    updateCurrentDaeNode();
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"sequence").c_str());

    // If a sequence node, store it's data as extra "Sequence" data in the "OpenSceneGraph" technique
    if (_pluginOptions.writeExtras)
    {
        // Adds the following to a node

        //<extra type="Sequence">
        //    <technique profile="OpenSceneGraph">
        //        <FrameTime>0 0</FrameTime>
        //        <LastFrameTime>0</LastFrameTime>
        //        <LoopMode>0</LoopMode>
        //        <IntervalBegin>0</IntervalBegin>
        //        <IntervalEnd>-1</IntervalEnd>
        //        <DurationSpeed>1</DurationSpeed>
        //        <DurationNReps>-1</DurationNReps>
        //        <SequenceMode>0</SequenceMode>
        //    </technique>
        //</extra>

        domExtra *extra = daeSafeCast<domExtra>(currentNode->add( COLLADA_ELEMENT_EXTRA ));
        extra->setType("Sequence");
        domTechnique *teq = daeSafeCast<domTechnique>(extra->add( COLLADA_ELEMENT_TECHNIQUE ) );
        teq->setProfile( "OpenSceneGraph" );

        domAny *frameTime = (domAny*)teq->add("FrameTime");
        std::stringstream fw;
        for (unsigned int i = 0; i < node.getNumChildren(); i++)
        {
            if (i > 0)
            {
                fw << " ";
            }
            fw << node.getTime(i);
        }
        frameTime->setValue(fw.str().c_str());

        domAny *lastFrameTime = (domAny*)teq->add("LastFrameTime");
        lastFrameTime->setValue(toString<double>(node.getLastFrameTime()).c_str());

        // loop mode & interval
        osg::Sequence::LoopMode mode;
        int begin, end;
        node.getInterval(mode, begin, end);
        domAny *loopMode = (domAny*)teq->add("LoopMode");
        loopMode->setValue(toString<osg::Sequence::LoopMode>(mode).c_str());
        domAny *intervalBegin = (domAny*)teq->add("IntervalBegin");
        intervalBegin->setValue(toString<int>(begin).c_str());
        domAny *intervalEnd = (domAny*)teq->add("IntervalEnd");
        intervalEnd->setValue(toString<int>(end).c_str());

        // duration
        float speed;
        int nreps;
        node.getDuration(speed, nreps);
        domAny *durationSpeed = (domAny*)teq->add("DurationSpeed");
        durationSpeed->setValue(toString<float>(speed).c_str());
        domAny *durationNReps = (domAny*)teq->add("DurationNReps");
        durationNReps->setValue(toString<int>(nreps).c_str());

        // sequence mode
        domAny *sequenceMode = (domAny*)teq->add("SequenceMode");
        sequenceMode->setValue(toString<osg::Sequence::SequenceMode>(node.getMode()).c_str());
    }

    writeNodeExtra(node);

    lastDepth = _nodePath.size();

    traverse( node );
}

void daeWriter::apply( osg::LOD &node )
{
    debugPrint( node );
    updateCurrentDaeNode();
    lastDepth = _nodePath.size();
    currentNode = daeSafeCast< domNode >(currentNode->add( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"LOD").c_str());

    if (_pluginOptions.writeExtras)
    {
        // Store LOD data as extra "LOD" data in the "OpenSceneGraph" technique
        // Adds the following to a node

        //<extra type="LOD">
        //    <technique profile="OpenSceneGraph">
        //        <Center>1 2 3</Center> (optional )
        //        <Radius>-1</Radius> (required if Center is available)
        //        <RangeMode>0</RangeMode>
        //        <RangeList>
        //            <MinMax>0 300</MinMax>
        //            <MinMax>300 600</MinMax>
        //        </RangeList>
        //    </technique>
        //</extra>

        domExtra *extra = daeSafeCast<domExtra>(currentNode->add( COLLADA_ELEMENT_EXTRA ));
        extra->setType("LOD");
        domTechnique *teq = daeSafeCast<domTechnique>(extra->add( COLLADA_ELEMENT_TECHNIQUE ) );
        teq->setProfile( "OpenSceneGraph" );

        if ((node.getCenterMode()==osg::LOD::USER_DEFINED_CENTER)||(node.getCenterMode()==osg::LOD::UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED))
        {
            domAny *center = (domAny*)teq->add("Center");
            center->setValue(toString(node.getCenter()).c_str());

            domAny *radius = (domAny*)teq->add("Radius");
            radius->setValue(toString<osg::LOD::value_type>(node.getRadius()).c_str());
        }

        domAny *rangeMode = (domAny*)teq->add("RangeMode");
        rangeMode->setValue(toString<osg::LOD::RangeMode>(node.getRangeMode()).c_str());

        domAny *valueLists = (domAny*)teq->add("RangeList");

        unsigned int pos = 0;
        const osg::LOD::RangeList& rangelist = node.getRangeList();
        for(osg::LOD::RangeList::const_iterator sitr=rangelist.begin();
            sitr!=rangelist.end();
            ++sitr,++pos)
        {
            domAny *valueList = (domAny*)valueLists->add("MinMax");
            std::stringstream fw;
            fw << sitr->first << " " << sitr->second;
            valueList->setValue(fw.str().c_str());
        }
    }

    writeNodeExtra(node);

    // Process all children
    traverse( node );
}

void daeWriter::apply( osg::ProxyNode &node )
{
    OSG_WARN << "ProxyNode. Missing " << node.getNumChildren() << " children" << std::endl;
}

void daeWriter::apply( osg::LightSource &node )
{
    debugPrint( node );
    updateCurrentDaeNode();

    domInstance_light *il = daeSafeCast< domInstance_light >( currentNode->add( COLLADA_ELEMENT_INSTANCE_LIGHT ) );
    std::string name = node.getName();
    if ( name.empty() )
    {
        name = uniquify( "light" );
    }
    std::string url = "#" + name;
    il->setUrl( url.c_str() );

    if ( lib_lights == NULL )
    {
        lib_lights = daeSafeCast< domLibrary_lights >( dom->add( COLLADA_ELEMENT_LIBRARY_LIGHTS ) );
    }
    domLight *light = daeSafeCast< domLight >( lib_lights->add( COLLADA_ELEMENT_LIGHT ) );
    light->setId( name.c_str() );

    osg::Light* pOsgLight = node.getLight();

    domLight *pDomLight = daeSafeCast< domLight >( lib_lights->add( COLLADA_ELEMENT_LIGHT ) );
    pDomLight->setId( name.c_str() );

    domLight::domTechnique_common *domTechniqueCommon = daeSafeCast<domLight::domTechnique_common>(pDomLight->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

    osg::Vec4 position = pOsgLight->getPosition();
    osg::Vec3 direction = pOsgLight->getDirection();
    osg::Vec4 ambientColor = pOsgLight->getAmbient();
    osg::Vec4 diffuseColor = pOsgLight->getDiffuse();
    //osg::Vec4 specularColor = pOsgLight->getSpecular();

    if (position.w() == 0)
    {
        // Directional light
        domLight::domTechnique_common::domDirectional *domDirectional = daeSafeCast<domLight::domTechnique_common::domDirectional>(domTechniqueCommon->add(COLLADA_ELEMENT_DIRECTIONAL));

        if ((position.x() != 0) || (position.y() != 0) || (position.z() != 0))
        {
            osg::Vec3 dir(-position.x(), -position.y(), -position.z());
            // TODO wrap instance_light in a rotating node to translate default light [0,0,-1] into proper direction
        }

        domFloat3 color;
        color.append3(diffuseColor.r(), diffuseColor.g(), diffuseColor.b());
        domDirectional->add(COLLADA_ELEMENT_COLOR);
        domDirectional->getColor()->setValue(color);
    }
    else if (direction.length() == 0)
    {
        // Omni/point light
        domLight::domTechnique_common::domPoint *domPoint = daeSafeCast<domLight::domTechnique_common::domPoint>(domTechniqueCommon->add(COLLADA_ELEMENT_POINT));
        domPoint->add(COLLADA_ELEMENT_CONSTANT_ATTENUATION);
        domPoint->getConstant_attenuation()->setValue(pOsgLight->getConstantAttenuation());
        domPoint->add(COLLADA_ELEMENT_LINEAR_ATTENUATION);
        domPoint->getLinear_attenuation()->setValue(pOsgLight->getLinearAttenuation());
        domPoint->add(COLLADA_ELEMENT_QUADRATIC_ATTENUATION);
        domPoint->getQuadratic_attenuation()->setValue(pOsgLight->getQuadraticAttenuation());

        if ((position.x() != 0) || (position.y() != 0) || (position.z() != 0))
        {
            // TODO wrap instance_light in a transforming node to translate default light [0,0,0] into proper position
        }

        domFloat3 color;
        color.append3(diffuseColor.r(), diffuseColor.g(), diffuseColor.b());
        domPoint->add(COLLADA_ELEMENT_COLOR);
        domPoint->getColor()->setValue(color);
    }
    else
    {
        // Spot light
        domLight::domTechnique_common::domSpot *domSpot = daeSafeCast<domLight::domTechnique_common::domSpot>(domTechniqueCommon->add(COLLADA_ELEMENT_SPOT));
        domSpot->add(COLLADA_ELEMENT_CONSTANT_ATTENUATION);
        domSpot->getConstant_attenuation()->setValue(pOsgLight->getConstantAttenuation());
        domSpot->add(COLLADA_ELEMENT_LINEAR_ATTENUATION);
        domSpot->getLinear_attenuation()->setValue(pOsgLight->getLinearAttenuation());
        domSpot->add(COLLADA_ELEMENT_QUADRATIC_ATTENUATION);
        domSpot->getQuadratic_attenuation()->setValue(pOsgLight->getQuadraticAttenuation());

        if ((position.x() != 0) || (position.y() != 0) || (position.z() != 0))
        {
            // TODO wrap instance_light in a transforming node to translate default light [0,0,0] into proper position
            // and rotate default direction [0,0,-1] into proper dir
        }

        domFloat3 color;
        color.append3(diffuseColor.r(), diffuseColor.g(), diffuseColor.b());
        domSpot->add(COLLADA_ELEMENT_COLOR);
        domSpot->getColor()->setValue(color);

        domSpot->add(COLLADA_ELEMENT_FALLOFF_ANGLE);
        domSpot->getFalloff_angle()->setValue(pOsgLight->getSpotCutoff());

        domSpot->add(COLLADA_ELEMENT_FALLOFF_EXPONENT);
        domSpot->getFalloff_exponent()->setValue(pOsgLight->getSpotExponent());
    }

    // Write ambient as a separate Collada <ambient> light
    if ((ambientColor.r() != 0) || (ambientColor.g() != 0) || (ambientColor.b() != 0))
    {
        domInstance_light *ambientDomInstanceLight = daeSafeCast< domInstance_light >(currentNode->add( COLLADA_ELEMENT_INSTANCE_LIGHT ));
        name = node.getName();
        if (name.empty())
        {
            name = uniquify( "light-ambient" );
        }
        else
        {
            name += "-ambient";
        }
        url = "#" + name;
        ambientDomInstanceLight->setUrl( url.c_str() );

        domLight *ambientDomLight = daeSafeCast< domLight >( lib_lights->add( COLLADA_ELEMENT_LIGHT ) );
        ambientDomLight->setId(name.c_str());

        domLight::domTechnique_common *ambientDomTechniqueCommon = daeSafeCast<domLight::domTechnique_common>(ambientDomLight->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

        // Ambient light
        domLight::domTechnique_common::domAmbient *domAmbient = daeSafeCast<domLight::domTechnique_common::domAmbient>(ambientDomTechniqueCommon->add(COLLADA_ELEMENT_AMBIENT));

        domFloat3 color;
        color.append3(ambientColor.r(), ambientColor.g(), ambientColor.b());
        domAmbient->add(COLLADA_ELEMENT_COLOR);
        domAmbient->getColor()->setValue(color);
    }

    traverse( node );
}

void daeWriter::apply( osg::Camera &node )
{
    debugPrint( node );
    updateCurrentDaeNode();

    domInstance_camera *ic = daeSafeCast< domInstance_camera >( currentNode->add( COLLADA_ELEMENT_INSTANCE_CAMERA ) );
    std::string name = node.getName();
    if ( name.empty() )
    {
        name = uniquify( "camera" );
    }
    std::string url = "#" + name;
    ic->setUrl( url.c_str() );

    if ( lib_cameras == NULL )
    {
        lib_cameras = daeSafeCast< domLibrary_cameras >( dom->add( COLLADA_ELEMENT_LIBRARY_CAMERAS ) );
    }
    domCamera *cam = daeSafeCast< domCamera >( lib_cameras->add( COLLADA_ELEMENT_CAMERA ) );
    cam->setId( name.c_str() );

    traverse( node );
}

void daeWriter::apply( osg::CameraView &node)
{
    debugPrint( node );
    updateCurrentDaeNode();

    domInstance_camera *ic = daeSafeCast< domInstance_camera >( currentNode->add(COLLADA_ELEMENT_INSTANCE_CAMERA));
    std::string name = node.getName();
    if ( name.empty() )
    {
        name = uniquify( "camera" );
    }
    std::string url = "#" + name;
    ic->setUrl( url.c_str() );

    if ( lib_cameras == NULL )
    {
        lib_cameras = daeSafeCast< domLibrary_cameras >( dom->add( COLLADA_ELEMENT_LIBRARY_CAMERAS ) );
    }
    domCamera *cam = daeSafeCast< domCamera >( lib_cameras->add( COLLADA_ELEMENT_CAMERA ) );
    cam->setId( name.c_str() );

    domCamera::domOptics *optics = daeSafeCast< domCamera::domOptics >( cam->add( COLLADA_ELEMENT_OPTICS ) );
    domCamera::domOptics::domTechnique_common *techniqueCommon = daeSafeCast< domCamera::domOptics::domTechnique_common >( optics->add( COLLADA_ELEMENT_TECHNIQUE_COMMON ) );
    domCamera::domOptics::domTechnique_common::domPerspective *pDomPerspective = daeSafeCast< domCamera::domOptics::domTechnique_common::domPerspective >( techniqueCommon->add( COLLADA_ELEMENT_PERSPECTIVE ) );

    domTargetableFloat *pXfov = NULL;
    domTargetableFloat *pYfov = NULL;
    switch(node.getFieldOfViewMode())
    {
        case(osg::CameraView::UNCONSTRAINED):
            pXfov = daeSafeCast< domTargetableFloat >( pDomPerspective->add( COLLADA_ELEMENT_XFOV ) );
            pXfov->setValue(node.getFieldOfView());
            break;
        case(osg::CameraView::HORIZONTAL):
            pXfov = daeSafeCast< domTargetableFloat >( pDomPerspective->add( COLLADA_ELEMENT_XFOV ) );
            pXfov->setValue(node.getFieldOfView());
            break;
        case(osg::CameraView::VERTICAL):
            pYfov = daeSafeCast< domTargetableFloat >( pDomPerspective->add( COLLADA_ELEMENT_YFOV ) );
            pYfov->setValue(node.getFieldOfView());
            break;
    }

    // Using hardcoded values for <aspect_ratio>, <znear> and <zfar>
    domTargetableFloat *pAspectRatio = daeSafeCast< domTargetableFloat >( pDomPerspective->add( COLLADA_ELEMENT_ASPECT_RATIO ) );
    pAspectRatio->setValue(1.0);

    domTargetableFloat *pNear = daeSafeCast< domTargetableFloat >( pDomPerspective->add( COLLADA_ELEMENT_ZNEAR ) );
    pNear->setValue(1);

    domTargetableFloat *pFar = daeSafeCast< domTargetableFloat >( pDomPerspective->add( COLLADA_ELEMENT_ZFAR ) );
    pFar->setValue(1000);
}
