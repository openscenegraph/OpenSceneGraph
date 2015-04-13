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

#include <osgShadow/ShadowTechnique>
#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osgShadow;

ShadowTechnique::CameraCullCallback::CameraCullCallback(ShadowTechnique* st):
    _shadowTechnique(st)
{
}

void ShadowTechnique::CameraCullCallback::operator()(osg::Node*, osg::NodeVisitor* nv)
{
    if (_shadowTechnique->getShadowedScene())
    {
        _shadowTechnique->getShadowedScene()->osg::Group::traverse(*nv);
    }
}

ShadowTechnique::ShadowTechnique():
    _shadowedScene(0),
    _dirty(true)
{
}

ShadowTechnique::ShadowTechnique(const ShadowTechnique& copy, const osg::CopyOp& copyop):
    osg::Object(copy,copyop),
    _shadowedScene(0),
    _dirty(true)
{
}

ShadowTechnique::~ShadowTechnique()
{
}

void ShadowTechnique::init()
{
    OSG_NOTICE<<className()<<"::init() not implemented yet"<<std::endl;

    _dirty = false;
}

void ShadowTechnique::update(osg::NodeVisitor& nv)
{
    OSG_NOTICE<<className()<<"::update(osg::NodeVisitor&) not implemented yet."<<std::endl;
     _shadowedScene->osg::Group::traverse(nv);
}

void ShadowTechnique::cull(osgUtil::CullVisitor& cv)
{
    OSG_NOTICE<<className()<<"::cull(osgUtl::CullVisitor&) not implemented yet."<<std::endl;
    _shadowedScene->osg::Group::traverse(cv);
}

void ShadowTechnique::cleanSceneGraph()
{
    OSG_NOTICE<<className()<<"::cleanSceneGraph()) not implemented yet."<<std::endl;
}

void ShadowTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_shadowedScene) return;

    if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
       if (_dirty) init();

        update(nv);
    }
    else if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv) cull(*cv);
        else _shadowedScene->osg::Group::traverse(nv);
    }
    else
    {
        _shadowedScene->osg::Group::traverse(nv);
    }
}

osg::Vec3 ShadowTechnique::computeOrthogonalVector(const osg::Vec3& direction) const
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
