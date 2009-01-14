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

#include <osgVolume/Locator>

#include <list>

using namespace osgVolume;

void Locator::setTransformAsExtents(double minX, double minY, double maxX, double maxY, double minZ, double maxZ)
{
    _transform.set(maxX-minX, 0.0,       0.0,       0.0,
                   0.0,       maxY-minY, 0.0,       0.0,
                   0.0,       0.0,       maxZ-minZ, 0.0,
                   minX,      minY,      minZ,      1.0); 

    _inverse.invert(_transform);
}

bool Locator::convertLocalToModel(const osg::Vec3d& local, osg::Vec3d& world) const
{
    world = local * _transform;
    return true;
}

bool Locator::convertModelToLocal(const osg::Vec3d& world, osg::Vec3d& local) const
{
    local = world * _inverse;
    return true;
}

bool Locator::computeLocalBounds(Locator& source, osg::Vec3d& bottomLeft, osg::Vec3d& topRight) const
{
    typedef std::list<osg::Vec3d> Corners;
    Corners corners;

    osg::Vec3d cornerNDC;
    if (convertLocalToModel(osg::Vec3d(0.0,0.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,0.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(0.0,1.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,1.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(0.0,0.0,1.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,0.0,1.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(0.0,1.0,1.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,1.0,1.0), cornerNDC))
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
        bottomLeft.z() = osg::minimum( bottomLeft.z(), itr->z());
        topRight.x() = osg::maximum( topRight.x(), itr->x());
        topRight.y() = osg::maximum( topRight.y(), itr->y());
        topRight.z() = osg::maximum( topRight.z(), itr->z());
    }
    
    return true;
}

bool Locator::computeLocalBounds(osg::Vec3d& bottomLeft, osg::Vec3d& topRight) const
{
    typedef std::list<osg::Vec3d> Corners;
    Corners corners;

    osg::Vec3d cornerNDC;
    if (convertLocalToModel(osg::Vec3d(0.0,0.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,0.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(0.0,1.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,1.0,0.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(0.0,0.0,1.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,0.0,1.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(0.0,1.0,1.0), cornerNDC))
    {
        corners.push_back(cornerNDC);
    }

    if (convertLocalToModel(osg::Vec3d(1.0,1.0,1.0), cornerNDC))
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
        bottomLeft.z() = osg::minimum( bottomLeft.z(), itr->z());
        topRight.x() = osg::maximum( topRight.x(), itr->x());
        topRight.y() = osg::maximum( topRight.y(), itr->y());
        topRight.z() = osg::maximum( topRight.z(), itr->z());
    }
    
    return true;
}
