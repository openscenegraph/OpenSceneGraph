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

#include <osgTerrain/Locator>
#include <osg/Notify>

#include <list>

using namespace osgTerrain;

//////////////////////////////////////////////////////////////////////////////
//
// Locator
//
Locator::Locator():
    _coordinateSystemType(PROJECTED),
    _ellipsoidModel(new osg::EllipsoidModel()),
    _minX(0.0),
    _minY(0.0),
    _maxX(0.0),
    _maxY(0.0)
{
}

Locator::Locator(const Locator& locator,const osg::CopyOp& copyop):
    osg::Object(locator,copyop),
    _coordinateSystemType(locator._coordinateSystemType),
    _ellipsoidModel(locator._ellipsoidModel),
    _format(locator._format),
    _cs(locator._cs),
    _minX(locator._minX),
    _minY(locator._minY),
    _maxX(locator._maxX),
    _maxY(locator._maxY)
{
}

Locator::~Locator()
{
}

void Locator::setExtents(double minX, double minY, double maxX, double maxY)
{
    _minX = minX;
    _minY = minY;
    _maxX = maxX;
    _maxY = maxY;
}

bool Locator::computeLocalBounds(Locator& source, osg::Vec3d& bottomLeft, osg::Vec3d& topRight)
{
    typedef std::list<osg::Vec3d> Corners;
    Corners corners;

    osg::Vec3d cornerNDC;
    if (Locator::convertLocalCoordBetween(source, osg::Vec3d(0.0,0.0,0.0), *this, cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (Locator::convertLocalCoordBetween(source, osg::Vec3d(1.0,0.0,0.0), *this, cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (Locator::convertLocalCoordBetween(source, osg::Vec3d(0.0,1.0,0.0), *this, cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (Locator::convertLocalCoordBetween(source, osg::Vec3d(1.0,1.0,0.0), *this, cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (corners.empty()) return false;


    for(Corners::iterator itr = corners.begin();
        itr != corners.end();
        ++itr)
    {
        bottomLeft.x() = osg::minimum( bottomLeft.x(), itr->x());
        bottomLeft.y() = osg::minimum( bottomLeft.y(), itr->y());
        topRight.x() = osg::maximum( topRight.x(), itr->x());
        topRight.y() = osg::maximum( topRight.y(), itr->y());
    }
    
    return true;
}

bool Locator::orientationOpenGL() const
{
    return ((_maxX-_minX) * (_maxY-_minY)) >= 0.0;
}

bool Locator::convertLocalToModel(const osg::Vec3d& local, osg::Vec3d& world) const
{
    switch(_coordinateSystemType)
    {
        case(GEOCENTRIC):
        {        
            double longitude = _minX * (1.0-local.x()) + _maxX * local.x();
            double latitude  = _minY * (1.0-local.y()) + _maxY * local.y();
            double height    = local.z();

            _ellipsoidModel->convertLatLongHeightToXYZ(latitude, longitude, height,
                                                       world.x(), world.y(), world.z());
            return true;      
        }
        case(GEOGRAPHIC):
        {        
            world.x() = _minX * (1.0-local.x()) + _maxX * local.x();
            world.y() = _minY * (1.0-local.y()) + _maxY * local.y();
            world.z() = local.z();
            return true;      
        }
        case(PROJECTED):
        {        
            world.x() = _minX * (1.0-local.x()) + _maxX * local.x();
            world.y() = _minY * (1.0-local.y()) + _maxY * local.y();
            world.z() = local.z();
            return true;      
        }
    }    

    return false;
}

bool Locator::convertModelToLocal(const osg::Vec3d& world, osg::Vec3d& local) const
{
    switch(_coordinateSystemType)
    {
        case(GEOCENTRIC):
        {        
            double longitude, latitude, height;

            _ellipsoidModel->convertXYZToLatLongHeight(world.x(), world.y(), world.z(),
                                                       latitude, longitude, height );


            local.x() = (longitude - _minX) / (_maxX - _minX);
            local.y() = (latitude - _minY) / (_maxY - _minY);
            local.z() = height;

            return true;      
        }
        case(GEOGRAPHIC):
        {        
            local.x() = (world.x() - _minX) / (_maxX - _minX);
            local.y() = (world.y() - _minY) / (_maxY - _minY);
            local.z() = world.z();
            return true;      
        }
        case(PROJECTED):
        {        
            local.x() = (world.x() - _minX) / (_maxX - _minX);
            local.y() = (world.y() - _minY) / (_maxY - _minY);
            local.z() = world.z();
            return true;      
        }
    }    

    return false;
}
