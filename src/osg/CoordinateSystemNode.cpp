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

CoordinateFrame CoordinateSystemNode::computeLocalCoordinateFrame(const Vec3& position) const
{
    return computeLocalCoordinateFrame(position.x(), position.y(), position.z());
}

CoordinateFrame CoordinateSystemNode::computeLocalCoordinateFrame(double X, double Y, double Z) const
{
    if (_ellipsoidModel.valid())
    {
        Matrixd localToWorld;
    
        _ellipsoidModel->computeLocalToWorldTransformFromXYZ(X,Y,Z, localToWorld);

        return localToWorld;
    }
    else
    {
        return Matrixd::translate(X,Y,Z);
    }
}

osg::Vec3 CoordinateSystemNode::computeLocalUpVector(const Vec3& position) const
{
    return computeLocalUpVector(position.x(), position.y(), position.z());
}

osg::Vec3 CoordinateSystemNode::computeLocalUpVector(double X, double Y, double Z) const
{
    if (_ellipsoidModel.valid())
    {
        return _ellipsoidModel->computeLocalUpVector(X,Y,Z);
    }
    else
    {
        return osg::Vec3(0.0f,0.0f,1.0f);
    }
}


