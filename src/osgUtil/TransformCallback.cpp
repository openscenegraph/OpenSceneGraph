#include <osg/Transform>

#include <osgUtil/TransformCallback>

using namespace osgUtil;

TransformCallback::TransformCallback(const osg::Vec3& pivot,const osg::Vec3& axis,float angularVelocity)
{
    _pivot = pivot;
    _axis = axis;
    _angular_velocity = angularVelocity;
    
    _previousTraversalNumber = -1;
    _previousTime = -1.0;
}

void TransformCallback::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
    osg::Transform* transform = dynamic_cast<osg::Transform*>(node);
    if (nv && transform)
    {
        
        const osg::FrameStamp* fs = nv->getFrameStamp();
        if (!fs) return; // not frame stamp, no handle on the time so can't move.

        
        // ensure that we do not operate on this node more than
        // once during this traversal.  This is an issue since node
        // can be shared between multiple parents.
        if (nv->getTraversalNumber()!=_previousTraversalNumber)
        {
            double newTime = fs->getReferenceTime();
            float delta_angle = _angular_velocity*(newTime-_previousTime);

            osg::Matrix mat = osg::Matrix::trans(-_pivot)*
                              osg::Matrix::rotate(delta_angle,_axis)*
                              osg::Matrix::trans(_pivot);


            // update the specified transform
            transform->preMult(mat);
            
            _previousTraversalNumber = nv->getTraversalNumber();
            _previousTime = newTime;
        }
    }

    // must call any nested node callbacks and continue subgraph traversal.
    traverse(node,nv);

}
