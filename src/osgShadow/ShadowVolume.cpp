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
#include <osg/Switch>

using namespace osgShadow;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   ComputeBoundingBoxVisitor
//
class ComputeBoundingBoxVisitor : public osg::NodeVisitor
{
public:
    ComputeBoundingBoxVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }
    
    virtual void reset()
    {
        _matrixStack.clear();
        _bb.init();
    }
    
    osg::BoundingBox& getBoundingBox() { return _bb; }

    void getPolytope(osg::Polytope& polytope, float margin=0.1) const
    {
        float delta = _bb.radius()*margin;
        polytope.add( osg::Plane(0.0, 0.0, 1.0, -(_bb.zMin()-delta)) );
        polytope.add( osg::Plane(0.0, 0.0, -1.0, (_bb.zMax()+delta)) );

        polytope.add( osg::Plane(1.0, 0.0, 0.0, -(_bb.xMin()-delta)) );
        polytope.add( osg::Plane(-1.0, 0.0, 0.0, (_bb.xMax()+delta)) );

        polytope.add( osg::Plane(0.0, 1.0, 0.0, -(_bb.yMin()-delta)) );
        polytope.add( osg::Plane(0.0, -1.0, 0.0, (_bb.yMax()+delta)) );
    }
        
    void getBase(osg::Polytope& polytope, float margin=0.1) const
    {
        float delta = _bb.radius()*margin;
        polytope.add( osg::Plane(0.0, 0.0, 1.0, -(_bb.zMin()-delta)) );
    }
    
    void apply(osg::Node& node)
    {
        traverse(node);
    }
    
    void apply(osg::Transform& transform)
    {
        osg::Matrix matrix;
        if (!_matrixStack.empty()) matrix = _matrixStack.back();
        
        transform.computeLocalToWorldMatrix(matrix,this);
        
        pushMatrix(matrix);
        
        traverse(transform);
        
        popMatrix();
    }
    
    void apply(osg::Geode& geode)
    {
        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            apply(geode.getDrawable(i));
        }
    }
    
    void pushMatrix(osg::Matrix& matrix)
    {
        _matrixStack.push_back(matrix);
    }
    
    void popMatrix()
    {
        _matrixStack.pop_back();
    }

    void apply(osg::Drawable* drawable)
    {
        if (_matrixStack.empty()) _bb.expandBy(drawable->getBound());
        else
        {
            osg::Matrix& matrix = _matrixStack.back();
            const osg::BoundingBox& dbb = drawable->getBound();
            if (dbb.valid())
            {
                _bb.expandBy(dbb.corner(0) * matrix);
                _bb.expandBy(dbb.corner(1) * matrix);
                _bb.expandBy(dbb.corner(2) * matrix);
                _bb.expandBy(dbb.corner(3) * matrix);
                _bb.expandBy(dbb.corner(4) * matrix);
                _bb.expandBy(dbb.corner(5) * matrix);
                _bb.expandBy(dbb.corner(6) * matrix);
                _bb.expandBy(dbb.corner(7) * matrix);
            }
        }
    }
    
protected:
    
    typedef std::vector<osg::Matrix> MatrixStack;

    MatrixStack         _matrixStack;
    osg::BoundingBox    _bb;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   ShadowVolume
//
ShadowVolume::ShadowVolume():
    _drawMode(osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED) 
{
    osg::notify(osg::NOTICE)<<"Warning: osgShadow::ShadowVolume technique in development."<<std::endl;
}

ShadowVolume::ShadowVolume(const ShadowVolume& sv, const osg::CopyOp& copyop):
    ShadowTechnique(sv,copyop),
    _drawMode(sv._drawMode)
{
}

ShadowVolume::~ShadowVolume()
{
}

void ShadowVolume::init()
{
    if (!_shadowedScene) return;
    
    bool updateLightPosition = false;
    
    // get the bounds of the model.    
    ComputeBoundingBoxVisitor cbbv;
    _shadowedScene->osg::Group::traverse(cbbv);


    osg::Vec4 lightpos;
    lightpos.set(0.5f,0.25f,0.8f,0.0f);

    // set up the occluder
    _occluder = new osgShadow::OccluderGeometry;
    _occluder->computeOccluderGeometry(_shadowedScene);
//    cbbv.getPolytope(_occluder->getBoundingPolytope(),0.001);
    cbbv.getBase(_occluder->getBoundingPolytope(),0.001);


    // set up shadow volume
    _shadowVolume = new osgShadow::ShadowVolumeGeometry;
    _shadowVolume->setUseDisplayList(!updateLightPosition);
    _occluder->computeShadowVolumeGeometry(lightpos, *_shadowVolume);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    _shadowedScene->addChild(geode.get());
    int shadowVolumeBin = 1000;

    if (_drawMode == osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED)
    {
        osg::notify(osg::NOTICE)<<"STENCIL_TWO_SIDED seleteced"<<std::endl;

        osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
        ss_sv1->setRenderBinDetails(shadowVolumeBin, "RenderBin");
        geode->addDrawable(_shadowVolume.get());
    }
    else
    {
        osg::notify(osg::NOTICE)<<"STENCIL_TWO_PASSES seleteced"<<std::endl;

        osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
        ss_sv1->setRenderBinDetails(shadowVolumeBin, "RenderBin");
        geode->addDrawable(_shadowVolume.get());
    }
    
    

    {

        // first group, render the depth buffer + ambient light contribution
        {
            _ss1 = new osg::StateSet;

#if 0
            osg::LightModel* lm1 = new osg::LightModel;
            lm1->setAmbientIntensity(ambient);
            _ss1->setAttribute(lm1);

            osg::Light* light1 = new osg::Light;
            light1->setAmbient(ambient);
            light1->setDiffuse(zero_colour);
            light1->setPosition(_lightpos);
            _ss1->setAttributeAndModes(light1, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#endif            
            
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

#if 0
            osg::LightModel* lm1 = new osg::LightModel;
            lm1->setAmbientIntensity(zero_colour);
            _shadowedSceneStateSet->setAttribute(lm1);

            osg::Light* light = new osg::Light;
            light->setAmbient(zero_colour);
            light->setDiffuse(diffuse);
            light->setPosition(_lightpos);

            _shadowedSceneStateSet->setMode(GL_LIGHT0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            _shadowedSceneStateSet->setAttribute(light);
#endif

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
            // osg::notify(osg::NOTICE)<<"Found shadow volume bin, now removing it"<<std::endl;
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
        new_rs->setDrawBuffer(orig_rs->getDrawBuffer());
        new_rs->setReadBuffer(orig_rs->getReadBuffer());
        new_rs->setColorMask(orig_rs->getColorMask());

        new_rs->setPositionalStateContainer(orig_rs->getPositionalStateContainer());

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
    osg::notify(osg::NOTICE)<<className()<<"::cleanSceneGraph()) not implemened yet, but almost."<<std::endl;
}

