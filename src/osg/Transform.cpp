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
#include <osg/Transform>

using namespace osg;

Transform::Transform()
{
    _referenceFrame = RELATIVE_TO_PARENTS;
}

Transform::Transform(const Transform& transform,const CopyOp& copyop):
    Group(transform,copyop),
    _computeTransformCallback(transform._computeTransformCallback),
    _referenceFrame(transform._referenceFrame)
{    
}

Transform::~Transform()
{
}

void Transform::setReferenceFrame(ReferenceFrame rf)
{
    if (_referenceFrame == rf) return;
    
    _referenceFrame = rf;
    
    // switch off culling if transform is absolute.
    if (_referenceFrame==RELATIVE_TO_ABSOLUTE) setCullingActive(false);
    else setCullingActive(true);
}

bool Transform::computeBound() const
{
    if (!Group::computeBound()) return false;
    
    // note, NULL pointer for NodeVisitor, so compute's need
    // to handle this case gracefully, normally this should not be a problem.
    Matrix l2w;

    getLocalToWorldMatrix(l2w,NULL);

    Vec3 xdash = _bsphere._center;
    xdash.x() += _bsphere._radius;
    xdash = xdash*l2w;

    Vec3 ydash = _bsphere._center;
    ydash.y() += _bsphere._radius;
    ydash = ydash*l2w;

    Vec3 zdash = _bsphere._center;
    zdash.z() += _bsphere._radius;
    zdash = zdash*l2w;

    _bsphere._center = _bsphere._center*l2w;

    xdash -= _bsphere._center;
    float len_xdash = xdash.length();

    ydash -= _bsphere._center;
    float len_ydash = ydash.length();

    zdash -= _bsphere._center;
    float len_zdash = zdash.length();

    _bsphere._radius = len_xdash;
    if (_bsphere._radius<len_ydash) _bsphere._radius = len_ydash;
    if (_bsphere._radius<len_zdash) _bsphere._radius = len_zdash;

    return true;

}
