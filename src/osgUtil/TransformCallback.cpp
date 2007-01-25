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
#include <osg/MatrixTransform>

#include <osgUtil/TransformCallback>

using namespace osgUtil;

TransformCallback::TransformCallback(const osg::Vec3& pivot,const osg::Vec3& axis,float angularVelocity)
{
    _pivot = pivot;
    _axis = axis;
    _angular_velocity = angularVelocity;
    
    _previousTraversalNumber = -1;
    _previousTime = -1.0;
    
    _pause = false;
}

void TransformCallback::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
    osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(node);
    if (nv && transform)
    {
        
        const osg::FrameStamp* fs = nv->getFrameStamp();
        if (!fs) return; // not frame stamp, no handle on the time so can't move.
        
        double newTime = fs->getSimulationTime();

        // ensure that we do not operate on this node more than
        // once during this traversal.  This is an issue since node
        // can be shared between multiple parents.
        if (!_pause && nv->getTraversalNumber()!=_previousTraversalNumber)
        {
            float delta_angle = _angular_velocity*(newTime-_previousTime);

            osg::Matrix mat = osg::Matrix::translate(-_pivot)*
                              osg::Matrix::rotate(delta_angle,_axis)*
                              osg::Matrix::translate(_pivot);


            // update the specified transform
            transform->preMult(mat);
            
            _previousTraversalNumber = nv->getTraversalNumber();
        }

        _previousTime = newTime; 

    }

    // must call any nested node callbacks and continue subgraph traversal.
    traverse(node,nv);

}
