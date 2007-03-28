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
#include <osgTerrain/TerrainNode>
#include <osg/io_utils>

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
    
    if (!_terrainNode) return;


    osgTerrain::Layer* elevationLayer = _terrainNode->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrainNode->getColorLayer();
    osg::TransferFunction* colorTF = _terrainNode->getColorTransferFunction();
    
    osg::notify(osg::NOTICE)<<"elevationLayer = "<<elevationLayer<<std::endl;
    osg::notify(osg::NOTICE)<<"colorLayer = "<<colorLayer<<std::endl;
    osg::notify(osg::NOTICE)<<"colorTF = "<<colorTF<<std::endl;

    Locator* elevationLocator = elevationLayer ? elevationLayer->getLocator() : 0;
    Locator* colorLocator = colorLayer ? colorLayer->getLocator() : 0;
    
    Locator* masterLocator = elevationLocator ? elevationLocator : colorLocator;
    if (!masterLocator)
    {
        osg::notify(osg::NOTICE)<<"Problem, no locator found in any of the terrain layers"<<std::endl;
        return;
    }
    
    if (!elevationLocator) elevationLocator = masterLocator;
    if (!colorLocator) colorLocator = masterLocator;
    
    osg::Vec3d bottomLeftNDC(DBL_MAX, DBL_MAX, 0.0);
    osg::Vec3d topRightNDC(-DBL_MAX, -DBL_MAX, 0.0);
    
    if (elevationLayer)
    {
        if (elevationLocator!= masterLocator)
        {
            masterLocator->computeLocalBounds(*elevationLocator, bottomLeftNDC, topRightNDC);
        }
        else
        {
            bottomLeftNDC.x() = osg::minimum(bottomLeftNDC.x(), 0.0);
            bottomLeftNDC.y() = osg::minimum(bottomLeftNDC.y(), 0.0);
            topRightNDC.x() = osg::maximum(topRightNDC.x(), 1.0);
            topRightNDC.y() = osg::maximum(topRightNDC.y(), 1.0);
        }
    }

    if (colorLayer)
    {
        if (colorLocator!= masterLocator)
        {
            masterLocator->computeLocalBounds(*colorLocator, bottomLeftNDC, topRightNDC);
        }
        else
        {
            bottomLeftNDC.x() = osg::minimum(bottomLeftNDC.x(), 0.0);
            bottomLeftNDC.y() = osg::minimum(bottomLeftNDC.y(), 0.0);
            topRightNDC.x() = osg::maximum(topRightNDC.x(), 1.0);
            topRightNDC.y() = osg::maximum(topRightNDC.y(), 1.0);
        }
    }

    osg::notify(osg::NOTICE)<<"bottomLeftNDC = "<<bottomLeftNDC<<std::endl;
    osg::notify(osg::NOTICE)<<"topRightNDC = "<<topRightNDC<<std::endl;

    _geode = new osg::Geode;
    _geometry = new osg::Geometry;
    _geode->addDrawable(_geometry.get());

    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec3Array* normals = new osg::Vec3Array;
    osg::Vec2Array* texcoords = new osg::Vec2Array;


    _dirty = false;    
}


void GeometryTechnique::update(osgUtil::UpdateVisitor* nv)
{
}


void GeometryTechnique::cull(osgUtil::CullVisitor* nv)
{
    if (_geode.valid())
    {
        _geode->accept(*nv);
    }
}

void GeometryTechnique::cleanSceneGraph()
{
}

void GeometryTechnique::dirty()
{
    TerrainTechnique::dirty();
}
