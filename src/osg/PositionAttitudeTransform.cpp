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
#include <osg/PositionAttitudeTransform>

using namespace osg;

PositionAttitudeTransform::PositionAttitudeTransform():
    _scale(1.0,1.0,1.0)
{
}

bool PositionAttitudeTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMultTranslate(_position);
        matrix.preMultRotate(_attitude);
        matrix.preMultScale(_scale);
        matrix.preMultTranslate(-_pivotPoint);
    }
    else // absolute
    {
        matrix.makeRotate(_attitude);
        matrix.postMultTranslate(_position);
        matrix.preMultScale(_scale);
        matrix.preMultTranslate(-_pivotPoint);
    }
    return true;
}


bool PositionAttitudeTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_scale.x() == 0.0 || _scale.y() == 0.0 || _scale.z() == 0.0)
        return false;

    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMultTranslate(-_position);
        matrix.postMultRotate(_attitude.inverse());
        matrix.postMultScale(Vec3d(1.0/_scale.x(), 1.0/_scale.y(), 1.0/_scale.z()));
        matrix.postMultTranslate(_pivotPoint);
    }
    else // absolute
    {
        matrix.makeRotate(_attitude.inverse());
        matrix.preMultTranslate(-_position);
        matrix.postMultScale(Vec3d(1.0/_scale.x(), 1.0/_scale.y(), 1.0/_scale.z()));
        matrix.postMultTranslate(_pivotPoint);
    }
    return true;
}
