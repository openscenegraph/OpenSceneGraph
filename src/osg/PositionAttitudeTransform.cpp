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
#include <osg/PositionAttitudeTransform>

using namespace osg;

PositionAttitudeTransform::PositionAttitudeTransform()
{
}

bool PositionAttitudeTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_TO_PARENTS)
    {
        matrix.preMult(osg::Matrix::translate(-_pivotPoint)*
                       osg::Matrix::rotate(_attitude)*
                       osg::Matrix::translate(_position));
    }
    else // absolute
    {
        matrix = osg::Matrix::translate(-_pivotPoint)*
                 osg::Matrix::rotate(_attitude)*
                 osg::Matrix::translate(_position);
    }
    return true;
}


bool PositionAttitudeTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_TO_PARENTS)
    {
        matrix.postMult(osg::Matrix::translate(-_position)*
                        osg::Matrix::rotate(_attitude.inverse())*
                        osg::Matrix::translate(_pivotPoint));
    }
    else // absolute
    {
        matrix = osg::Matrix::translate(-_position)*
                 osg::Matrix::rotate(_attitude.inverse())*
                 osg::Matrix::translate(_pivotPoint);
    }
    return true;
}
