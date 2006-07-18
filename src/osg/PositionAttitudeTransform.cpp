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
    _scale(1.0f,1.0f,1.0f)
{
}

bool PositionAttitudeTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMult(osg::Matrix::translate(-_pivotPoint)*
                       osg::Matrix::scale(_scale)*
                       osg::Matrix::rotate(_attitude)*
                       osg::Matrix::translate(_position));
    }
    else // absolute
    {
        matrix = osg::Matrix::translate(-_pivotPoint)*
                 osg::Matrix::scale(_scale)*
                 osg::Matrix::rotate(_attitude)*
                 osg::Matrix::translate(_position);
    }
    return true;
}


bool PositionAttitudeTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMult(osg::Matrix::translate(-_position)*
                        osg::Matrix::rotate(_attitude.inverse())*
                        osg::Matrix::scale(1.0f/_scale.x(),1.0f/_scale.y(),1.0f/_scale.z())*
                        osg::Matrix::translate(_pivotPoint));
    }
    else // absolute
    {
        matrix = osg::Matrix::translate(-_position)*
                 osg::Matrix::rotate(_attitude.inverse())*
                 osg::Matrix::scale(1.0f/_scale.x(),1.0f/_scale.y(),1.0f/_scale.z())*
                 osg::Matrix::translate(_pivotPoint);
    }
    return true;
}
