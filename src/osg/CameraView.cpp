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
#include <osg/CameraView>

using namespace osg;

CameraView::CameraView():
    _fieldOfView(60.0),
    _fieldOfViewMode(VERTICAL),
    _focalLength(0.0)
{
}

bool CameraView::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMultTranslate(_position);
        matrix.preMultRotate(_attitude);
    }
    else // absolute
    {
        matrix.makeRotate(_attitude);
        matrix.postMultTranslate(_position);
    }
    return true;
}


bool CameraView::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMultTranslate(-_position);
        matrix.postMultRotate(_attitude.inverse());
    }
    else // absolute
    {
        matrix.makeRotate(_attitude.inverse());
        matrix.preMultTranslate(-_position);
    }
    return true;
}
