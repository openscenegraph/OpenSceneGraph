/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <osgTerrain/CoordinateSystem>

using namespace osgTerrain;

CoordinateSystem::CoordinateSystem()
{
}

CoordinateSystem::CoordinateSystem(const std::string& projectionRef):
    _projectionRef(projectionRef)
{
}

CoordinateSystem::CoordinateSystem(const CoordinateSystem& cs,const osg::CopyOp& copyop):
    Object(cs,copyop),
    _projectionRef(cs._projectionRef)
{
}


osg::ref_ptr<CoordinateSystem::CoordinateTransformation>& getCoordinateTransformationPrototypePtr()
{
    static osg::ref_ptr<CoordinateSystem::CoordinateTransformation> s_coordinateSystemPrototype;
    return s_coordinateSystemPrototype;
}


CoordinateSystem::CoordinateTransformation* CoordinateSystem::CoordinateTransformation::createCoordinateTransformation(const CoordinateSystem& source, const CoordinateSystem& destination)
{
    if (getCoordinateTransformationPrototypePtr().valid())
    {
        getCoordinateTransformationPrototypePtr()->cloneCoordinateTransformation(source, destination);
    }
    return 0L;
}

void CoordinateSystem::CoordinateTransformation::setCoordinateTransformationPrototpe(CoordinateTransformation* ct)
{
    getCoordinateTransformationPrototypePtr() = ct;
}
