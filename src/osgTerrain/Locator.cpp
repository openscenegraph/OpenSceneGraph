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

#include <list>

using namespace osgTerrain;

//////////////////////////////////////////////////////////////////////////////
//
// Locator
//
Locator::Locator()
{
}

Locator::Locator(const Locator& Locator,const osg::CopyOp& copyop):
    osg::Object(Locator,copyop)
{
}

Locator::~Locator()
{
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

//////////////////////////////////////////////////////////////////////////////
//
// EllipsoidLocator
//
EllipsoidLocator::EllipsoidLocator(double longitude, double latitude, double deltaLongitude, double deltaLatitude, double height, double heightScale, double radiusEquator, double radiusPolar)
{
    setExtents(longitude, latitude, deltaLongitude, deltaLatitude, height, heightScale);
    _em = new osg::EllipsoidModel(radiusEquator, radiusPolar);
}

void EllipsoidLocator::setExtents(double longitude, double latitude, double deltaLongitude, double deltaLatitude, double height, double heightScale)
{
    _longitude = longitude;
    _latitude = latitude;
    _deltaLongitude = deltaLongitude;
    _deltaLatitude = deltaLatitude;
    _height = height;
    _heightScale = heightScale;
}

bool EllipsoidLocator::orientationOpenGL() const
{
    return (_deltaLongitude * _deltaLatitude) >= 0.0;
}

bool EllipsoidLocator::convertLocalToModel(const osg::Vec3d& local, osg::Vec3d& world) const
{
    double longitude = _longitude + local.x() * _deltaLongitude;
    double latitude = _latitude + local.y() * _deltaLatitude;
    double height = _height + local.z() * _heightScale;

    _em->convertLatLongHeightToXYZ(latitude, longitude, height,
                                   world.x(), world.y(), world.z());

    return true;
}

bool EllipsoidLocator::convertModelToLocal(const osg::Vec3d& world, osg::Vec3d& local) const
{
    double longitude, latitude, height;

    _em->convertXYZToLatLongHeight(world.x(), world.y(), world.z(),
                                   latitude, longitude, height );

    local.x() = (longitude-_longitude)/_deltaLongitude;
    local.y() = (latitude-_latitude)/_deltaLatitude;
    local.z() = (height-_height)/_heightScale;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// CartizianLocator
//

CartizianLocator::CartizianLocator(double originX, double originY, double lengthX, double lengthY, double height, double heightScale)
{
    setExtents(originX, originY, lengthY, lengthY, height, heightScale);
}

void CartizianLocator::setExtents(double originX, double originY, double lengthX, double lengthY, double height, double heightScale)
{
    _originX = originX;
    _originY = originY;
    _lengthX = lengthX;
    _lengthY = lengthY;
    _height = height;
    _heightScale = heightScale;
}

bool CartizianLocator::orientationOpenGL() const
{
    return (_lengthX * _lengthY) >= 0.0;
}

bool CartizianLocator::convertLocalToModel(const osg::Vec3d& local, osg::Vec3d& world) const
{
    world.x() = _originX + local.x() * _lengthX;
    world.y() = _originY + local.y() * _lengthY;
    world.z() = _height + local.z() * _heightScale;

    return true;
}

bool CartizianLocator::convertModelToLocal(const osg::Vec3d& world, osg::Vec3d& local) const
{
    local.x() = (world.x() - _originX)/_lengthX;
    local.y() = (world.y() - _originY)/_lengthY;
    local.z() = (world.z() - _height)/_heightScale;

    return true;
}
