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

#include <osg/CoordinateSystemNode>

using namespace osg;


CoordinateSystemNode::CoordinateSystemNode()
{
}

CoordinateSystemNode::CoordinateSystemNode(const std::string& WKT)
{
    _WKT = WKT;
}

CoordinateSystemNode::CoordinateSystemNode(const CoordinateSystemNode& csn,const osg::CopyOp& copyop):
    Group(csn,copyop)
{
    _WKT = csn._WKT;
    _ellipsoidModel = csn._ellipsoidModel;
}

CoordinateFrame CoordinateSystemNode::computeLocalCoordinateFrame(const Vec3d& position) const
{
    if (_ellipsoidModel.valid())
    {
        Matrixd localToWorld;
    
        _ellipsoidModel->computeLocalToWorldTransformFromXYZ(position.x(),position.y(),position.z(), localToWorld);

        return localToWorld;
    }
    else
    {
        return Matrixd::translate(position);
    }
}

osg::Vec3d CoordinateSystemNode::computeLocalUpVector(const Vec3d& position) const
{
    if (_ellipsoidModel.valid())
    {
        return _ellipsoidModel->computeLocalUpVector(position.x(),position.y(),position.z());
    }
    else
    {
        return osg::Vec3d(0.0f,0.0f,1.0f);
    }
}


