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

#include <osgTerrain/GeometryTechnique>

using namespace osgTerrain;

GeometryTechnique::GeometryTechnique()
{
}

GeometryTechnique::GeometryTechnique(const GeometryTechnique& gt,const osg::CopyOp& copyop):
    TerrainTechnique(gt,copyop)
{
}

GeometryTechnique::~GeometryTechnique()
{
}

void GeometryTechnique::init()
{
    osg::notify(osg::NOTICE)<<"Doing init()"<<std::endl;
    
    _geode = new osg::Geode;
    _geometry = new osg::Geometry;
    _geode->addDrawable(_geometry.get());

    _dirty = false;    
}


void GeometryTechnique::update(osgUtil::UpdateVisitor* nv)
{
    osg::notify(osg::NOTICE)<<"Doing update"<<std::endl;
}


void GeometryTechnique::cull(osgUtil::CullVisitor* nv)
{
    osg::notify(osg::NOTICE)<<"Doing cull"<<std::endl;
    
    if (_geode.valid())
    {
        _geode->accept(*nv);
    }
}

void GeometryTechnique::cleanSceneGraph()
{
    osg::notify(osg::NOTICE)<<"Cleaning scene graph"<<std::endl;
}

void GeometryTechnique::dirty()
{
    TerrainTechnique::dirty();
}
