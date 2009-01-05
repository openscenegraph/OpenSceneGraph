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
    _definedInFile(false),
    _transformScaledByResolution(false)
{
}

Locator::Locator(const Locator& locator,const osg::CopyOp& copyop):
    osg::Object(locator,copyop),
    _coordinateSystemType(locator._coordinateSystemType),
    _format(locator._format),
    _cs(locator._cs),
    _ellipsoidModel(locator._ellipsoidModel),
    _transform(locator._transform),
    _definedInFile(locator._definedInFile),
    _transformScaledByResolution(locator._transformScaledByResolution)
{
}

Locator::~Locator()
{
}

void Locator::setTransformAsExtents(double minX, double minY, double maxX, double maxY)
{
    _transform.set(maxX-minX, 0.0,       0.0, 0.0,
                   0.0,       maxY-minY, 0.0, 0.0,
                   0.0,       0.0,       1.0, 0.0,
                   minX,      minY,      0.0, 1.0); 

    _inverse.invert(_transform);
}

bool Locator::computeLocalBounds(Locator& source, osg::Vec3d& bottomLeft, osg::Vec3d& topRight) const
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
    return _transform(0,0) * _transform(1,1) >= 0.0;
}

bool Locator::convertLocalToModel(const osg::Vec3d& local, osg::Vec3d& world) const
{
    switch(_coordinateSystemType)
    {
        case(GEOCENTRIC):
        {
            osg::Vec3d geographic = local * _transform;
                
            _ellipsoidModel->convertLatLongHeightToXYZ(geographic.y(), geographic.x(), geographic.z(),
                                                       world.x(), world.y(), world.z());
            return true;      
        }
        case(GEOGRAPHIC):
        {        
            world = local * _transform;
            return true;      
        }
        case(PROJECTED):
        {        
            world = local * _transform;
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

            local = osg::Vec3d(longitude, latitude, height) * _inverse;

            return true;      
        }
        case(GEOGRAPHIC):
        {        
            local = world * _inverse;

            return true;      
        }
        case(PROJECTED):
        {        
            local = world * _inverse;
            return true;      
        }
    }    

    return false;
}
