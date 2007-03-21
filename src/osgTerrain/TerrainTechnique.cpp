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

#include <osgTerrain/TerrainTechnique>

using namespace osgTerrain;

TerrainTechnique::TerrainTechnique():
    _terrainNode(0)
{
}

TerrainTechnique::TerrainTechnique(const TerrainTechnique& TerrainTechnique,const osg::CopyOp& copyop):
    osg::Object(TerrainTechnique,copyop),
    _terrainNode(0)
{
}

TerrainTechnique::~TerrainTechnique()
{
}

void TerrainTechnique::initialize()
{
    osg::notify(osg::NOTICE)<<className()<<"::initialize(..) not implementated yet"<<std::endl;
}

void TerrainTechnique::update(osgUtil::UpdateVisitor*)
{
    osg::notify(osg::NOTICE)<<className()<<"::update(..) not implementated yet"<<std::endl;
}

void TerrainTechnique::cull(osgUtil::CullVisitor*)
{
    osg::notify(osg::NOTICE)<<className()<<"::cull(..) not implementated yet"<<std::endl;
}

void TerrainTechnique::cleanSceneGraph()
{
    osg::notify(osg::NOTICE)<<className()<<"::cleanSceneGraph(..) not implementated yet"<<std::endl;
}

void TerrainTechnique::dirty()
{
    osg::notify(osg::NOTICE)<<className()<<"::dirty(..) not implementated yet"<<std::endl;
}
