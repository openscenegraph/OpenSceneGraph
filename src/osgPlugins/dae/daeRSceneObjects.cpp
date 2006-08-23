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
#include <dom/domCOLLADA.h>

#include <osg/Switch>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/Switch>
#include <osg/ShapeDrawable>

using namespace osgdae;

osg::Node* daeReader::processLight( domLight *dlight )
{
    //do light processing here.
    domLight::domTechnique_common::domAmbient *ambient;
    domLight::domTechnique_common::domDirectional *directional;
    domLight::domTechnique_common::domPoint *point;
    domLight::domTechnique_common::domSpot *spot;

    if ( dlight->getTechnique_common() == NULL || 
        dlight->getTechnique_common()->getContents().getCount() == 0 )
    {
        osg::notify( osg::WARN ) << "Invalid content for light" << std::endl;
        return NULL;
    }

    osg::Node* node = new osg::Switch();

    osg::Light* light = new osg::Light();

    osg::LightSource* lightsource = new osg::LightSource();

    lightsource->setLight( light );
    light->setPosition(osg::Vec4(0,0,0,1));
    light->setLightNum( m_numlights );

    daeElement *el = dlight->getTechnique_common()->getContents()[0];
    ambient = daeSafeCast< domLight::domTechnique_common::domAmbient >( el );
    directional = daeSafeCast< domLight::domTechnique_common::domDirectional >( el );
    point = daeSafeCast< domLight::domTechnique_common::domPoint >( el );
    spot = daeSafeCast< domLight::domTechnique_common::domSpot >( el );
    if ( ambient != NULL )
    {
        if ( ambient->getColor() == NULL ) 
        {
            osg::notify( osg::WARN ) << "Invalid content for ambient light" << std::endl;
            return NULL;
        }
        light->setAmbient( osg::Vec4( ambient->getColor()->getValue()[0], ambient->getColor()->getValue()[1], 
            ambient->getColor()->getValue()[2], 1.0f ) );
    }
    else if ( directional != NULL )
    {
        if ( directional->getColor() == NULL ) 
        {
            osg::notify( osg::WARN ) << "Invalid content for ambient light" << std::endl;
            return NULL;
        }
        light->setDiffuse( osg::Vec4( directional->getColor()->getValue()[0], directional->getColor()->getValue()[1], 
            directional->getColor()->getValue()[2], 1.0f ) );
        light->setSpecular( osg::Vec4( directional->getColor()->getValue()[0], directional->getColor()->getValue()[1], 
            directional->getColor()->getValue()[2], 1.0f ) );
        
        light->setDirection( osg::Vec3( 0, 0, -1 ) );
    }
    else if ( point != NULL )
    {
        if ( point->getColor() == NULL ) 
        {
            osg::notify( osg::WARN ) << "Invalid content for ambient light" << std::endl;
            return NULL;
        }
        light->setDiffuse( osg::Vec4( point->getColor()->getValue()[0], point->getColor()->getValue()[1], 
            point->getColor()->getValue()[2], 1.0f ) );
        light->setSpecular( osg::Vec4( point->getColor()->getValue()[0], point->getColor()->getValue()[1], 
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

    }
    else if ( spot != NULL )
    {
        if ( spot->getColor() == NULL ) 
        {
            osg::notify( osg::WARN ) << "Invalid content for ambient light" << std::endl;
            return NULL;
        }
        light->setDiffuse( osg::Vec4( spot->getColor()->getValue()[0], spot->getColor()->getValue()[1], 
            spot->getColor()->getValue()[2], 1.0f ) );
        light->setSpecular( osg::Vec4( spot->getColor()->getValue()[0], spot->getColor()->getValue()[1], 
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

        if ( spot->getFalloff_angle() != NULL )
        {
            light->setSpotCutoff( spot->getFalloff_angle()->getValue() );
        }
        else
        {
            light->setSpotCutoff( 180.0f );
        }

        if ( spot->getFalloff_exponent() != NULL )
        {
            light->setSpotExponent( spot->getFalloff_exponent()->getValue() );
        }
        else
        {
            light->setSpotExponent( 0.0f );
        }

    }

    osg::Switch* svitch = static_cast<osg::Switch*>(node);

    // Switched of by default to avoid excessively large scene bound
    svitch->addChild(lightsource,false);

    m_numlights++;

    return node;
}

osg::Node* daeReader::processCamera( domCamera* /*dcamera*/ )
{
    //TODO: Make the camera actually make a camera to view from. Not just draw a cone.
    osg::Node *node = new osg::Switch();

    osg::Cone* cone = new osg::Cone();

    osg::ShapeDrawable* sd = new osg::ShapeDrawable(cone);

    cone->setRadius(0.3);
    cone->setHeight(1.0);

    osg::Geode* geode = new osg::Geode();

    geode->addDrawable(sd);
    geode->setName("camera");

    osg::Switch* svitch = static_cast<osg::Switch*>(node);

    // Switched of by default to avoid excessively large scene bound
    svitch->addChild(geode,false);

    return node;
}
