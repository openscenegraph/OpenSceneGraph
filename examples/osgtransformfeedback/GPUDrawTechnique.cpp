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

#include "GPUDrawTechnique"
#include "GPUScene"
#include <osg/Notify>
#include <osg/io_utils>

//using namespace osgShadow;

GPUDrawTechnique::CameraCullCallback::CameraCullCallback(GPUDrawTechnique* st):
    _GPUDrawTechnique(st)
{
}

void GPUDrawTechnique::CameraCullCallback::operator()(osg::Node*, osg::NodeVisitor* nv)
{
    if (_GPUDrawTechnique->getGPUScene())
    {
        _GPUDrawTechnique->getGPUScene()->osg::Group::traverse(*nv);
    }
}

GPUDrawTechnique::GPUDrawTechnique():
    _GPUScene(0),
    _dirty(true)
{
}

GPUDrawTechnique::GPUDrawTechnique(const GPUDrawTechnique& copy, const osg::CopyOp& copyop):
    osg::Object(copy,copyop),
    _GPUScene(0),
    _dirty(true)
{
}

GPUDrawTechnique::~GPUDrawTechnique()
{
}

void GPUDrawTechnique::init()
{
    OSG_NOTICE<<className()<<"::init() not implemented yet"<<std::endl;

    _dirty = false;
}

void GPUDrawTechnique::update(osg::NodeVisitor& nv)
{
    OSG_NOTICE<<className()<<"::update(osg::NodeVisitor&) not implemented yet."<<std::endl;
     _GPUScene->osg::Group::traverse(nv);
}

void GPUDrawTechnique::cull(osgUtil::CullVisitor& cv)
{
    OSG_NOTICE<<className()<<"::cull(osgUtl::CullVisitor&) not implemented yet."<<std::endl;
    _GPUScene->osg::Group::traverse(cv);
}

void GPUDrawTechnique::cleanSceneGraph()
{
    OSG_NOTICE<<className()<<"::cleanSceneGraph()) not implemented yet."<<std::endl;
}

void GPUDrawTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_GPUScene) return;

    if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
       if (_dirty) init();

        update(nv);
    }
    else if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = nv.asCullVisitor();
        if (cv) cull(*cv);
        else _GPUScene->osg::Group::traverse(nv);
    }
    else
    {
        _GPUScene->osg::Group::traverse(nv);
    }
}

osg::Vec3 GPUDrawTechnique::computeOrthogonalVector(const osg::Vec3& direction) const
{
    float length = direction.length();
    osg::Vec3 orthogonalVector = direction ^ osg::Vec3(0.0f, 1.0f, 0.0f);
    if (orthogonalVector.normalize()<length*0.5f)
    {
        orthogonalVector = direction ^ osg::Vec3(0.0f, 0.0f, 1.0f);
        orthogonalVector.normalize();
    }
    return orthogonalVector;
}
