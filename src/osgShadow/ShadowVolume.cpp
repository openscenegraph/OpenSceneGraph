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

using namespace osgShadow;

ShadowVolume::ShadowVolume()    
{
    osg::notify(osg::NOTICE)<<"Warning: osgShadow::ShadowVolume technique in development."<<std::endl;
}

ShadowVolume::ShadowVolume(const ShadowVolume& copy, const osg::CopyOp& copyop):
    ShadowTechnique(copy,copyop)
{
}

ShadowVolume::~ShadowVolume()
{
}

void ShadowVolume::init()
{
    osg::notify(osg::NOTICE)<<className()<<"::init() not implemened yet, but almost"<<std::endl;
    
    _dirty = false;
}

void ShadowVolume::update(osg::NodeVisitor& nv)
{
    osg::notify(osg::NOTICE)<<className()<<"::update(osg::NodeVisitor&) not implemened yet, but almost."<<std::endl;
    _shadowedScene->osg::Group::traverse(nv);
}

void ShadowVolume::cull(osg::NodeVisitor& nv)
{
    osg::notify(osg::NOTICE)<<className()<<"::cull(osg::NodeVisitor&) not implemened yet, but almost."<<std::endl;
    _shadowedScene->osg::Group::traverse(nv);
}

void ShadowVolume::cleanSceneGraph()
{
    osg::notify(osg::NOTICE)<<className()<<"::cleanSceneGraph()) not implemened yet, but almost."<<std::endl;
}

