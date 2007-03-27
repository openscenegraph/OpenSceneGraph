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


//////////////////////////////////////////////////////////////////////////////
//
// EllipsoidLocator
//
EllipsoidLocator::EllipsoidLocator(double longitude, double latitude, double deltaLongitude, double deltaLatitude, double height):
    _longitude(longitude),
    _latitude(latitude),
    _deltaLongitude(deltaLongitude),
    _deltaLatitude(deltaLatitude),
    _height(height)

{
    _em = new osg::EllipsoidModel;
}


void EllipsoidLocator::setExtents(double longitude, double latitude, double deltaLongitude, double deltaLatitude, double height)
{
    _longitude = longitude;
    _latitude = latitude;
    _deltaLongitude = deltaLongitude;
    _deltaLatitude = deltaLatitude;
    _height = height;
}

bool EllipsoidLocator::convertLocalToModel(const osg::Vec3d& local, osg::Vec3d& world) const
{
    double longitude = _longitude + local.x() * _deltaLongitude;
    double latitude = _latitude + local.y() * _deltaLatitude;
    double height = _height + local.z();

    _em->convertLatLongHeightToXYZ(latitude, longitude, height,
                                   world.x(), world.y(), world.z());

    return true;
}

bool EllipsoidLocator::convertModelToWorld(const osg::Vec3d& world, osg::Vec3d& local) const
{
    double longitude, latitude, height;

    _em->convertXYZToLatLongHeight(world.x(), world.y(), world.z(),
                                   latitude, longitude, height );

    local.x() = (longitude-_longitude)/_deltaLongitude;
    local.y() = (latitude-_latitude)/_deltaLatitude;
    local.z() = height-_height;

    return true;
}
