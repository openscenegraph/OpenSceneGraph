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

#include "daeReader.h"
#include <dae.h>
#include <dae/domAny.h>
#include <dom/domCOLLADA.h>

#include <osg/Switch>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/LOD>
#include <osg/Billboard>
#include <osgSim/MultiSwitch>
#include <osg/Sequence>
#include <osg/CameraView>
#include <osg/LightModel>

using namespace osgDAE;

osg::Group* daeReader::processOsgMultiSwitch(domTechnique* teq)
{
    osgSim::MultiSwitch* msw = new osgSim::MultiSwitch;

    domAny* any = daeSafeCast<domAny>(teq->getChild("ActiveSwitchSet"));
    if (any)
    {
        msw->setActiveSwitchSet(parseString<unsigned int>(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'ActiveSwitchSet' not found" << std::endl;
    }

    any = daeSafeCast<domAny>(teq->getChild("ValueLists"));
    if (any)
    {
        unsigned int numChildren = any->getChildren().getCount();
        for (unsigned int currChild = 0; currChild < numChildren; currChild++)
        {
            domAny* child = daeSafeCast<domAny>(any->getChildren()[currChild]);
            if (child)
            {
                if (strcmp(child->getElementName(), "ValueList" ) == 0 )
                {
                    std::list<std::string> stringValues;
                    osgSim::MultiSwitch::ValueList values;

                    cdom::tokenize(child->getValue(), " ", stringValues);
                    cdom::tokenIter iter = stringValues.begin();

                    while (iter != stringValues.end())
                    {
                        values.push_back(parseString<bool>(*iter));
                        ++iter;
                    }

                    msw->setValueList(currChild, values);
                }
                else
                {
                    OSG_WARN << "Child of element 'ValueLists' is not of type 'ValueList'" << std::endl;
                }
            }
            else
            {
                OSG_WARN << "Element 'ValueLists' does not contain expected elements." << std::endl;
            }
        }
    }
    else
    {
        OSG_WARN << "Expected element 'ValueLists' not found" << std::endl;
    }
    return msw;
}

osg::Group* daeReader::processOsgSwitch(domTechnique* teq)
{
    osg::Switch* sw = new osg::Switch;

    domAny* any = daeSafeCast< domAny >(teq->getChild("ValueList"));
    if (any)
    {
        std::list<std::string> stringValues;

        cdom::tokenize(any->getValue(), " ", stringValues);
        cdom::tokenIter iter = stringValues.begin();

        int pos = 0;
        while (iter != stringValues.end())
        {
            sw->setValue(pos++, parseString<bool>(*iter));
            ++iter;
        }
    }
    else
    {
        OSG_WARN << "Expected element 'ValueList' not found" << std::endl;
    }
    return sw;
}

osg::Group* daeReader::processOsgSequence(domTechnique* teq)
{
    osg::Sequence* sq = new osg::Sequence;

    domAny* any = daeSafeCast< domAny >(teq->getChild("FrameTime"));
    if (any)
    {
        std::list<std::string> stringValues;

        cdom::tokenize(any->getValue(), " ", stringValues);
        cdom::tokenIter iter = stringValues.begin();

        int frame = 0;
        while (iter != stringValues.end())
        {
            sq->setTime(frame++, parseString<double>(*iter));
            ++iter;
        }
    }
    else
    {
        OSG_WARN << "Expected element 'FrameTime' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("LastFrameTime"));
    if (any)
    {
        sq->setLastFrameTime(parseString<double>(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'LastFrameTime' not found" << std::endl;
    }

    osg::Sequence::LoopMode loopmode = osg::Sequence::LOOP;
    any = daeSafeCast< domAny >(teq->getChild("LoopMode"));
    if (any)
    {
        loopmode = (osg::Sequence::LoopMode)parseString<int>(any->getValue());
    }
    else
    {
        OSG_WARN << "Expected element 'LoopMode' not found" << std::endl;
    }

    int begin=0;
    any = daeSafeCast< domAny >(teq->getChild("IntervalBegin"));
    if (any)
    {
        begin = parseString<int>(any->getValue());
    }
    else
    {
        OSG_WARN << "Expected element 'IntervalBegin' not found" << std::endl;
    }

    int end=-1;
    any = daeSafeCast< domAny >(teq->getChild("IntervalEnd"));
    if (any)
    {
        end = parseString<int>(any->getValue());
    }
    else
    {
        OSG_WARN << "Expected element 'IntervalEnd' not found" << std::endl;
    }

    sq->setInterval(loopmode, begin, end);

    float speed = 0;
    any = daeSafeCast< domAny >(teq->getChild("DurationSpeed"));
    if (any)
    {
        speed = parseString<float>(any->getValue());
    }
    else
    {
        OSG_WARN << "Expected element 'DurationSpeed' not found" << std::endl;
    }

    int nreps = -1;
    any = daeSafeCast< domAny >(teq->getChild("DurationNReps"));
    if (any)
    {
        nreps = parseString<int>(any->getValue());
    }
    else
    {
        OSG_WARN << "Expected element 'DurationNReps' not found" << std::endl;
    }

    sq->setDuration(speed, nreps);

    any = daeSafeCast< domAny >(teq->getChild("SequenceMode"));
    if (any)
    {
        sq->setMode((osg::Sequence::SequenceMode)parseString<int>(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'SequenceMode' not found" << std::endl;
    }

    return sq;
}


osg::Group* daeReader::processOsgLOD(domTechnique* teq)
{
    osg::LOD* lod = new osg::LOD;

    domAny* any = daeSafeCast< domAny >(teq->getChild("Center"));
    if (any)
    {
        // If a center is specified
        lod->setCenterMode(osg::LOD::USER_DEFINED_CENTER);
        lod->setCenter(parseVec3String(any->getValue()));

        any = daeSafeCast< domAny >(teq->getChild("Radius"));
        if (any)
        {
            lod->setRadius(parseString<osg::LOD::value_type>(any->getValue()));
        }
        else
        {
            OSG_WARN << "Expected element 'Radius' not found" << std::endl;
        }
    }

    any = daeSafeCast< domAny >(teq->getChild("RangeMode"));
    if (any)
    {
        lod->setRangeMode((osg::LOD::RangeMode)parseString<int>(any->getValue()));
    }
    else
    {
        OSG_WARN << "Expected element 'RangeMode' not found" << std::endl;
    }

    any = daeSafeCast< domAny >(teq->getChild("RangeList"));
    if (any)
    {
        osg::LOD::RangeList rangelist;

        unsigned int numChildren = any->getChildren().getCount();
        for (unsigned int currChild = 0; currChild < numChildren; currChild++)
        {
            domAny* child = daeSafeCast<domAny>(any->getChildren()[currChild]);
            if (child)
            {
                if (strcmp(child->getElementName(), "MinMax" ) == 0 )
                {
                    std::list<std::string> stringValues;
                    osg::LOD::MinMaxPair minMaxPair;

                    cdom::tokenize(child->getValue(), " ", stringValues);
                    cdom::tokenIter iter = stringValues.begin();

                    if (iter != stringValues.end())
                    {
                        minMaxPair.first = parseString<float>(*iter);
                        ++iter;
                    }
                    else
                    {
                        OSG_WARN << "'MinMax' does not contain a valid minimum value" << std::endl;
                    }

                    if (iter != stringValues.end())
                    {
                        minMaxPair.second = parseString<float>(*iter);
                    }
                    else
                    {
                        OSG_WARN << "'MinMax' does not contain a valid maximum value" << std::endl;
                    }

                    rangelist.push_back(minMaxPair);
                }
                else
                {
                    OSG_WARN << "Child of element 'RangeList' is not of type 'MinMax'" << std::endl;
                }
            }
            else
            {
                OSG_WARN << "Element 'RangeList' does not contain expected elements." << std::endl;
            }
        }

        lod->setRangeList(rangelist);
    }
    else
    {
        OSG_WARN << "Expected element 'RangeList' not found" << std::endl;
    }

    return lod;
}

// <light>
// attributes:
// id, name
// elements:
// 0..1 <asset>
// 1    <technique_common>
//        1    <ambient>, <directional>, <point>, <spot>
// 0..* <technique>
// 0..* <extra>
osg::Node* daeReader::processLight( domLight *dlight )
{
    if (_numlights >= 7)
    {
        OSG_WARN << "More than 8 lights may not be supported by OpenGL driver." << std::endl;
    }

    //do light processing here.
    domLight::domTechnique_common::domAmbient *ambient;
    domLight::domTechnique_common::domDirectional *directional;
    domLight::domTechnique_common::domPoint *point;
    domLight::domTechnique_common::domSpot *spot;

    if ( dlight->getTechnique_common() == NULL ||
         dlight->getTechnique_common()->getContents().getCount() == 0 )
    {
        OSG_WARN << "Invalid content for light" << std::endl;
        return NULL;
    }

    osg::Light* light = new osg::Light();
    light->setPosition(osg::Vec4(0,0,0,1));
    light->setLightNum(_numlights);

    // Enable OpenGL lighting
    _rootStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    // Enable this OpenGL light
    _rootStateSet->setMode(GL_LIGHT0 + _numlights++, osg::StateAttribute::ON);

    // Set ambient of lightmodel to zero
    // Ambient lights are added as separate lights with only an ambient term
    osg::LightModel* lightmodel = new osg::LightModel;
    lightmodel->setAmbientIntensity(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    _rootStateSet->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);

    osg::LightSource* lightsource = new osg::LightSource();
    lightsource->setLight(light);
    std::string name = dlight->getId() ? dlight->getId() : "";
    if (dlight->getName())
        name = dlight->getName();
    lightsource->setName(name);

    daeElement *el = dlight->getTechnique_common()->getContents()[0];
    ambient = daeSafeCast< domLight::domTechnique_common::domAmbient >( el );
    directional = daeSafeCast< domLight::domTechnique_common::domDirectional >( el );
    point = daeSafeCast< domLight::domTechnique_common::domPoint >( el );
    spot = daeSafeCast< domLight::domTechnique_common::domSpot >( el );
    if ( ambient != NULL )
    {
        if ( ambient->getColor() == NULL )
        {
            OSG_WARN << "Invalid content for ambient light" << std::endl;
            return NULL;
        }

        light->setAmbient(    osg::Vec4(    ambient->getColor()->getValue()[0],
                                        ambient->getColor()->getValue()[1],
                                        ambient->getColor()->getValue()[2], 1.0f ) );
        light->setDiffuse(    osg::Vec4(    0, 0, 0, 0));
        light->setSpecular(    osg::Vec4(    0, 0, 0, 0));

        // Tell OpenGL to make it a directional light (w=0)
        light->setPosition(    osg::Vec4(0,0,0,0));
    }
    else if ( directional != NULL )
    {
        if ( directional->getColor() == NULL )
        {
            OSG_WARN << "Invalid content for directional light" << std::endl;
            return NULL;
        }
        light->setAmbient(    osg::Vec4(    0, 0, 0, 0));
        light->setDiffuse(    osg::Vec4(    directional->getColor()->getValue()[0],
                                        directional->getColor()->getValue()[1],
                                        directional->getColor()->getValue()[2], 1.0f ) );
        light->setSpecular( osg::Vec4(    directional->getColor()->getValue()[0],
                                        directional->getColor()->getValue()[1],
                                        directional->getColor()->getValue()[2], 1.0f ) );

        light->setDirection(osg::Vec3(0,0,-1));

        // Tell OpenGL it is a directional light (w=0)
        light->setPosition(    osg::Vec4(0,0,1,0));
    }
    else if ( point != NULL )
    {
        if ( point->getColor() == NULL )
        {
            OSG_WARN << "Invalid content for point light" << std::endl;
            return NULL;
        }
        light->setAmbient(    osg::Vec4(    0, 0, 0, 0));
        light->setDiffuse(    osg::Vec4(    point->getColor()->getValue()[0],
                                        point->getColor()->getValue()[1],
                                        point->getColor()->getValue()[2], 1.0f ) );
        light->setSpecular( osg::Vec4(    point->getColor()->getValue()[0],
                                        point->getColor()->getValue()[1],
                                        point->getColor()->getValue()[2], 1.0f ) );

        if ( point->getConstant_attenuation() != NULL )
        {
            light->setConstantAttenuation( point->getConstant_attenuation()->getValue() );
        }
        else
        {
            light->setConstantAttenuation( 1.0 );
        }
        if ( point->getLinear_attenuation() != NULL )
        {
            light->setLinearAttenuation( point->getLinear_attenuation()->getValue() );
        }
        else
        {
            light->setLinearAttenuation( 0.0 );
        }
        if ( point->getQuadratic_attenuation() != NULL )
        {
            light->setQuadraticAttenuation( point->getQuadratic_attenuation()->getValue() );
        }
        else
        {
            light->setQuadraticAttenuation( 0.0 );
        }

        // Tell OpenGL this is an omni directional light
        light->setDirection(osg::Vec3(0, 0, 0));
    }
    else if ( spot != NULL )
    {
        if ( spot->getColor() == NULL )
        {
            OSG_WARN << "Invalid content for spot light" << std::endl;
            return NULL;
        }
        light->setAmbient(    osg::Vec4(    0, 0, 0, 0));
        light->setDiffuse(    osg::Vec4(    spot->getColor()->getValue()[0],
                                        spot->getColor()->getValue()[1],
                                        spot->getColor()->getValue()[2], 1.0f ) );
        light->setSpecular( osg::Vec4(    spot->getColor()->getValue()[0],
                                        spot->getColor()->getValue()[1],
                                        spot->getColor()->getValue()[2], 1.0f ) );

        if ( spot->getConstant_attenuation() != NULL )
        {
            light->setConstantAttenuation( spot->getConstant_attenuation()->getValue() );
        }
        else
        {
            light->setConstantAttenuation( 1.0f );
        }
        if ( spot->getLinear_attenuation() != NULL )
        {
            light->setLinearAttenuation( spot->getLinear_attenuation()->getValue() );
        }
        else
        {
            light->setLinearAttenuation( 0.0f );
        }
        if ( spot->getQuadratic_attenuation() != NULL )
        {
            light->setQuadraticAttenuation( spot->getQuadratic_attenuation()->getValue() );
        }
        else
        {
            light->setQuadraticAttenuation( 0.0f );
        }
        // OpenGL range [0, 90] (degrees) or 180 (omni)
        if ( spot->getFalloff_angle() != NULL )
        {
            float falloffAngle = spot->getFalloff_angle()->getValue();
            if (falloffAngle != 180)
            {
                falloffAngle = osg::clampTo<float>(falloffAngle, 0, 90);
            }
            light->setSpotCutoff(falloffAngle);
        }
        else
        {
            light->setSpotCutoff( 180.0f );
        }
        // OpenGL range [0, 128], where 0 means hard edge, and 128 means soft edge
        if ( spot->getFalloff_exponent() != NULL )
        {
            float falloffExponent = spot->getFalloff_exponent()->getValue();
            falloffExponent = osg::clampTo<float>(falloffExponent, 0, 128);
            light->setSpotExponent(falloffExponent);
        }
        else
        {
            light->setSpotExponent( 0.0f );
        }
        light->setDirection(osg::Vec3(0, 0, -1));
    }

    return lightsource;
}

// <camera>
// attributes:
// id, name
// elements:
// 0..1 <asset>
// 1    <optics>
//        1       <technique_common>
//                1        <orthographic>, <perspective>
//        0..*    <technique>
//        0..*    <extra>
// 0..* <imager>
//        1       <technique>
//        0..*    <extra>
// 0..* <extra>
osg::Node* daeReader::processCamera( domCamera * dcamera )
{
    osg::CameraView* pOsgCameraView = new osg::CameraView;
    pOsgCameraView->setName(dcamera->getId());

    domCamera::domOptics::domTechnique_common *pDomTechniqueCommon = dcamera->getOptics()->getTechnique_common();
    domCamera::domOptics::domTechnique_common::domPerspective *pDomPerspective = pDomTechniqueCommon->getPerspective();
    domCamera::domOptics::domTechnique_common::domOrthographic *pDomOrthographic = pDomTechniqueCommon->getOrthographic();
    if (pDomPerspective)
    {
        // <perspective>
        // 1    <xfov>, <yfov>, <xfov> and <yfov>, <xfov> and <aspect_ratio>, <yfov> and <aspect_ratio>
        // 1    <znear>
        // 1    <zfar>
        domTargetableFloat *pXfov = daeSafeCast< domTargetableFloat >(pDomPerspective->getXfov());
        domTargetableFloat *pYfov = daeSafeCast< domTargetableFloat >(pDomPerspective->getYfov());
        domTargetableFloat *pAspectRatio = daeSafeCast< domTargetableFloat >(pDomPerspective->getAspect_ratio());

        if (pXfov)
        {
            if (pYfov)
            {
                // <xfov> and <yfov>
                pOsgCameraView->setFieldOfView(pXfov->getValue());
                pOsgCameraView->setFieldOfViewMode(osg::CameraView::HORIZONTAL);

                if (pAspectRatio)
                {
                    OSG_WARN << "Unexpected <aspectratio> in <camera> '" << dcamera->getId() << "'" << std::endl;
                }
            }
            else if (pAspectRatio)
            {
                // <xfov> and <aspect_ratio>
                pOsgCameraView->setFieldOfView(pXfov->getValue() * pAspectRatio->getValue());
                pOsgCameraView->setFieldOfViewMode(osg::CameraView::HORIZONTAL);
            }
            else
            {
                // <xfov>
                pOsgCameraView->setFieldOfView(pXfov->getValue());
                pOsgCameraView->setFieldOfViewMode(osg::CameraView::HORIZONTAL);
            }
        }
        else if (pYfov)
        {
            if (pAspectRatio)
            {
                // <yfov> and <aspect_ratio>
                pOsgCameraView->setFieldOfView(pYfov->getValue() / pAspectRatio->getValue());
                pOsgCameraView->setFieldOfViewMode(osg::CameraView::VERTICAL);
            }
            else
            {
                // <yfov>
                pOsgCameraView->setFieldOfView(pYfov->getValue());
                pOsgCameraView->setFieldOfViewMode(osg::CameraView::VERTICAL);
            }
        }
        else
        {
            // xfov or yfov expected
            OSG_WARN << "Expected <xfov> or <yfov> in <camera> '" << dcamera->getId() << "'" << std::endl;
        }

        //domTargetableFloat *pZnear = daeSafeCast< domTargetableFloat >(pDomPerspective->getZnear());
        //domTargetableFloat *pZfar = daeSafeCast< domTargetableFloat >(pDomPerspective->getZfar());

        // TODO The current osg::CameraView does not support storage of near far
    }
    else if (pDomOrthographic)
    {
        // <orthographic>
        // 1    <xmag>, <ymag>, <xmag> and <ymag>, <xmag> and <aspect_ratio>, <ymag> and <aspect_ratio>
        // 1    <znear>
        // 1    <zfar>

        //domTargetableFloat *pXmag = daeSafeCast< domTargetableFloat >(pDomOrthographic->getXmag());
        //domTargetableFloat *pYmag = daeSafeCast< domTargetableFloat >(pDomOrthographic->getYmag());
        //domTargetableFloat *pAspectRatio = daeSafeCast< domTargetableFloat >(pDomOrthographic->getAspect_ratio());

        // TODO The current osg::CameraView does not support an orthographic view
        OSG_WARN << "Orthographic in <camera> '" << dcamera->getId() << "' not supported" << std::endl;

        //domTargetableFloat *pZnear = daeSafeCast< domTargetableFloat >(pDomOrthographic->getZnear());
        //domTargetableFloat *pZfar = daeSafeCast< domTargetableFloat >(pDomOrthographic->getZfar());

        // TODO The current osg::CameraView does not support storage of near far
    }

    return pOsgCameraView;
}
