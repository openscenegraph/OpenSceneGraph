/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield 
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

#include <osgVolume/FixedFunctionTechnique>

using namespace osgVolume;

FixedFunctionTechnique::FixedFunctionTechnique()
{
}

FixedFunctionTechnique::FixedFunctionTechnique(const FixedFunctionTechnique& fft,const osg::CopyOp& copyop):
    VolumeTechnique(fft,copyop)
{
}

FixedFunctionTechnique::~FixedFunctionTechnique()
{
}

void FixedFunctionTechnique::init()
{
    osg::notify(osg::NOTICE)<<"FixedFunctionTechnique::init()"<<std::endl;
}

void FixedFunctionTechnique::update(osgUtil::UpdateVisitor* nv)
{
    osg::notify(osg::NOTICE)<<"FixedFunctionTechnique:update(osgUtil::UpdateVisitor* nv):"<<std::endl;
}

void FixedFunctionTechnique::cull(osgUtil::CullVisitor* nv)
{
    osg::notify(osg::NOTICE)<<"FixedFunctionTechnique::cull(osgUtil::CullVisitor* nv)"<<std::endl;
}

void FixedFunctionTechnique::cleanSceneGraph()
{
    osg::notify(osg::NOTICE)<<"FixedFunctionTechnique::cleanSceneGraph()"<<std::endl;
}

void FixedFunctionTechnique::traverse(osg::NodeVisitor& nv)
{
    osg::notify(osg::NOTICE)<<"FixedFunctionTechnique::traverse(osg::NodeVisitor& nv)"<<std::endl;
}
    
