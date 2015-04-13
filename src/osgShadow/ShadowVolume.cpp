/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgShadow/ShadowVolume>
#include <osgShadow/ShadowedScene>
#include <osg/Notify>

#include <osg/LightModel>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <osg/Stencil>
#include <osg/StencilTwoSided>
#include <osg/CullFace>
#include <osg/Geometry>
#include <osg/ComputeBoundsVisitor>
#include <osg/io_utils>

using namespace osgShadow;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   ShadowVolume
//
ShadowVolume::ShadowVolume():
    _drawMode(osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED),
    _dynamicShadowVolumes(false)
{
    // _drawMode = osgShadow::ShadowVolumeGeometry::GEOMETRY;

    OSG_INFO<<"Warning: osgShadow::ShadowVolume technique is still in development, with current limitations that make it unsuitable for deployment. Please contact the osg-users for an update of developements."<<std::endl;
}

ShadowVolume::ShadowVolume(const ShadowVolume& sv, const osg::CopyOp& copyop):
    ShadowTechnique(sv,copyop),
    _drawMode(sv._drawMode),
    _dynamicShadowVolumes(sv._dynamicShadowVolumes)
{
}

ShadowVolume::~ShadowVolume()
{
}

void ShadowVolume::setDrawMode(osgShadow::ShadowVolumeGeometry::DrawMode drawMode)
{
    if (_drawMode == drawMode) return;

    _drawMode = drawMode;

    dirty();
}

void ShadowVolume::setDynamicShadowVolumes(bool dynamicShadowVolumes)
{
    _dynamicShadowVolumes = dynamicShadowVolumes;
}


void ShadowVolume::init()
{
    if (!_shadowedScene) return;

    // get the bounds of the model.
    osg::ComputeBoundsVisitor cbbv;
    _shadowedScene->osg::Group::traverse(cbbv);


    osg::Vec4 ambient(0.2,0.2,0.2,1.0);
    osg::Vec4 diffuse(0.8,0.8,0.8,1.0);
    osg::Vec4 zero_colour(0.0,0.0,0.0,1.0);

    osg::Vec4 lightpos;
    lightpos.set(0.5f,0.25f,0.8f,0.0f);

    // set up the occluder
    _occluder = new osgShadow::OccluderGeometry;
    _occluder->computeOccluderGeometry(_shadowedScene);
//    cbbv.getPolytope(_occluder->getBoundingPolytope(),0.001);
    cbbv.getBase(_occluder->getBoundingPolytope(),0.001);


    // set up shadow volume
    _shadowVolume = new osgShadow::ShadowVolumeGeometry;
    _shadowVolume->setUseDisplayList(!_dynamicShadowVolumes);
    _shadowVolume->setDrawMode(_drawMode);
    _occluder->computeShadowVolumeGeometry(lightpos, *_shadowVolume);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    _shadowedScene->addChild(geode.get());
    int shadowVolumeBin = 1000;

    if (_drawMode == osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED)
    {
        OSG_NOTICE<<"STENCIL_TWO_SIDED selected"<<std::endl;

        osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
        ss_sv1->setRenderBinDetails(shadowVolumeBin, "RenderBin");
        geode->addDrawable(_shadowVolume.get());
    }
    else
    {
        OSG_NOTICE<<"STENCIL_TWO_PASSES selected"<<std::endl;

        osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
        ss_sv1->setRenderBinDetails(shadowVolumeBin, "RenderBin");
        geode->addDrawable(_shadowVolume.get());
    }

    {

        // first group, render the depth buffer + ambient light contribution
        {
            _ss1 = new osg::StateSet;

            osg::LightModel* lm1 = new osg::LightModel;
            lm1->setAmbientIntensity(ambient);
            _ss1->setAttribute(lm1);

            _ambientLight = new osg::Light;
            _ambientLight->setAmbient(ambient);
            _ambientLight->setDiffuse(zero_colour);
            _ss1->setAttributeAndModes(_ambientLight.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

            _ss1->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        }

        {
            _mainShadowStateSet = new osg::StateSet;

            osg::Depth* depth = new osg::Depth;
            depth->setWriteMask(false);
            depth->setFunction(osg::Depth::LEQUAL);
            _mainShadowStateSet->setAttribute(depth);
            _mainShadowStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        }

        {
            _shadowVolumeStateSet = new osg::StateSet;

            osg::Depth* depth = new osg::Depth;
            depth->setWriteMask(false);
            depth->setFunction(osg::Depth::LEQUAL);
            _shadowVolumeStateSet->setAttribute(depth);
            _shadowVolumeStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
            _shadowVolumeStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);


            if (_drawMode == osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED)
            {
                osg::StencilTwoSided* stencil = new osg::StencilTwoSided;
                stencil->setFunction(osg::StencilTwoSided::BACK, osg::StencilTwoSided::ALWAYS,0,~0u);
                stencil->setOperation(osg::StencilTwoSided::BACK, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::DECR_WRAP);
                stencil->setFunction(osg::StencilTwoSided::FRONT, osg::StencilTwoSided::ALWAYS,0,~0u);
                stencil->setOperation(osg::StencilTwoSided::FRONT, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::INCR_WRAP);

                osg::ColorMask* colourMask = new osg::ColorMask(false, false, false, false);

                _shadowVolumeStateSet->setAttributeAndModes(stencil,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                _shadowVolumeStateSet->setAttribute(colourMask, osg::StateAttribute::OVERRIDE);
                _shadowVolumeStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);


            }
            else
            {
                osg::Stencil* stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::ALWAYS,0,~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);

                osg::ColorMask* colourMask = new osg::ColorMask(false, false, false, false);

                _shadowVolumeStateSet->setAttributeAndModes(stencil,osg::StateAttribute::ON);
                _shadowVolumeStateSet->setAttribute(colourMask);
                _shadowVolumeStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
            }
        }


        {
            _shadowedSceneStateSet = new osg::StateSet;

            osg::Depth* depth = new osg::Depth;
            depth->setWriteMask(false);
            depth->setFunction(osg::Depth::LEQUAL);
            _shadowedSceneStateSet->setAttribute(depth);
            _shadowedSceneStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
            // _shadowedSceneStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

            osg::LightModel* lm1 = new osg::LightModel;
            lm1->setAmbientIntensity(zero_colour);
            _shadowedSceneStateSet->setAttribute(lm1);

            _diffuseLight = new osg::Light;
            _diffuseLight->setAmbient(zero_colour);
            _diffuseLight->setDiffuse(diffuse);

            _shadowedSceneStateSet->setMode(GL_LIGHT0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            _shadowedSceneStateSet->setAttribute(_diffuseLight.get());

            // set up the stencil ops so that only operator on this mirrors stencil value.
            osg::Stencil* stencil = new osg::Stencil;
            stencil->setFunction(osg::Stencil::EQUAL,0,~0u);
            stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
            _shadowedSceneStateSet->setAttributeAndModes(stencil,osg::StateAttribute::ON);

            osg::BlendFunc* blend = new osg::BlendFunc;
            blend->setFunction(osg::BlendFunc::ONE, osg::BlendFunc::ONE);
            _shadowedSceneStateSet->setAttributeAndModes(blend, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            // _shadowedSceneStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        }

        _ss1->setThreadSafeRefUnref(true);
        _mainShadowStateSet->setThreadSafeRefUnref(true);
        _shadowVolumeStateSet->setThreadSafeRefUnref(true);
        _shadowedSceneStateSet->setThreadSafeRefUnref(true);

    }

    _dirty = false;
}

void ShadowVolume::update(osg::NodeVisitor& nv)
{
    _shadowedScene->osg::Group::traverse(nv);
}

void ShadowVolume::cull(osgUtil::CullVisitor& cv)
{

    osg::ref_ptr<osgUtil::RenderBin> original_bin = cv.getCurrentRenderBin();

    osg::ref_ptr<osgUtil::RenderBin> new_bin = original_bin->find_or_insert(0,"RenderBin");

    cv.setCurrentRenderBin(new_bin.get());

    _shadowedScene->osg::Group::traverse(cv);

    cv.setCurrentRenderBin(original_bin.get());

    osgUtil::RenderBin::RenderBinList::iterator itr =  new_bin->getRenderBinList().find(1000);
    osg::ref_ptr<osgUtil::RenderBin> shadowVolumeBin;
    if (itr != new_bin->getRenderBinList().end())
    {
        shadowVolumeBin = itr->second;

        if (shadowVolumeBin.valid())
        {
            //OSG_NOTICE<<"Found shadow volume bin, now removing it"<<std::endl;
            new_bin->getRenderBinList().erase(itr);
        }
    }

    if (shadowVolumeBin.valid())
    {
        original_bin->setStateSet(_ss1.get());

        osgUtil::RenderStage* orig_rs = cv.getRenderStage();
        osgUtil::RenderStage* new_rs = new osgUtil::RenderStage;
        orig_rs->addPostRenderStage(new_rs);

        new_rs->setViewport(orig_rs->getViewport());
        new_rs->setClearColor(orig_rs->getClearColor());
        new_rs->setClearMask(GL_STENCIL_BUFFER_BIT);
        new_rs->setDrawBuffer(orig_rs->getDrawBuffer(), orig_rs->getDrawBufferApplyMask());
        new_rs->setReadBuffer(orig_rs->getReadBuffer(), orig_rs->getReadBufferApplyMask());
        new_rs->setColorMask(orig_rs->getColorMask());

        osg::Vec4 lightpos;

        osg::ref_ptr<osgUtil::PositionalStateContainer> ps = new osgUtil::PositionalStateContainer;
        new_rs->setPositionalStateContainer(ps.get());

        const osg::Light* selectLight = 0;

        osgUtil::PositionalStateContainer::AttrMatrixList& aml = orig_rs->getPositionalStateContainer()->getAttrMatrixList();
        for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
            itr != aml.end();
            ++itr)
        {
            const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
            if (light)
            {
                osg::RefMatrix* matrix = itr->second.get();
                if (matrix) lightpos = light->getPosition() * (*matrix);
                else lightpos = light->getPosition();

                selectLight = light;
            }
            else
            {
                ps->addPositionedAttribute(itr->second.get(), itr->first.get());
            }
        }

        _ambientLight->setPosition(lightpos);

        orig_rs->addPositionedAttribute(0,_ambientLight.get());

        _diffuseLight->setPosition(lightpos);
        if (selectLight)
        {
            _ambientLight->setAmbient(selectLight->getAmbient());

            _diffuseLight->setDiffuse(selectLight->getDiffuse());
            _diffuseLight->setSpecular(selectLight->getSpecular());
            _diffuseLight->setDirection(selectLight->getDirection());
            _diffuseLight->setConstantAttenuation(selectLight->getConstantAttenuation());
            _diffuseLight->setLinearAttenuation(selectLight->getLinearAttenuation());
            _diffuseLight->setQuadraticAttenuation(selectLight->getQuadraticAttenuation());
            _diffuseLight->setSpotExponent(selectLight->getSpotExponent());
            _diffuseLight->setSpotCutoff(selectLight->getSpotCutoff());
        }
        ps->addPositionedAttribute(0, _diffuseLight.get());

        if (_lightpos != lightpos && _dynamicShadowVolumes)
        {
            _lightpos = lightpos;

            osg::Matrix eyeToWorld;
            eyeToWorld.invert(*cv.getModelViewMatrix());

            _occluder->computeShadowVolumeGeometry(lightpos * eyeToWorld, *_shadowVolume);
        }

        if (shadowVolumeBin.valid())
        {
            // new_rs->setStateSet(_mainShadowStateSet.get());
            new_rs->getRenderBinList()[0] = shadowVolumeBin;
            shadowVolumeBin->setStateSet(_shadowVolumeStateSet.get());

            osg::ref_ptr<osgUtil::RenderBin> nested_bin = new_rs->find_or_insert(1,"RenderBin");
            nested_bin->getRenderBinList()[0] = new_bin;
            nested_bin->setStateSet(_shadowedSceneStateSet.get());
        }
    }


}

void ShadowVolume::cleanSceneGraph()
{
    OSG_NOTICE<<className()<<"::cleanSceneGraph()) not implemented yet, but almost."<<std::endl;
}

