#include <osg/MatrixTransform>

#include "PendulumCallback.h"

PendulumCallback::PendulumCallback(const osg::Vec3& axis, 
                                         float startAngle, float endAngle,
                                         float frequency)
{
    _axis = axis;
    _startAngle = startAngle;
    _endAngle = endAngle;
    _frequency = frequency;
    
    _previousTraversalNumber = -1;
    _previousTime = -1.0;
    _angle = 0.0;
}

void PendulumCallback::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
    if (!nv)
        return;

    osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(node);
    if (!transform)
        return;

    const osg::FrameStamp* fs = nv->getFrameStamp();
    if (!fs) 
        return; 
    
    // ensure that we do not operate on this node more than
    // once during this traversal.  This is an issue since node
    // can be shared between multiple parents.
    if (nv->getTraversalNumber()!=_previousTraversalNumber)
    {
        double currentTime = fs->getReferenceTime();
        _angle += (currentTime - _previousTime) * 2 * osg::PI * _frequency;
        
        double frac = 0.5 + 0.5 * sin(_angle);
        double rotAngle = _endAngle  - _startAngle - osg::PI 
                + (1.0 - frac) * _startAngle + frac * _endAngle;

        // update the specified transform
        transform->setMatrix(osg::Matrix::rotate(rotAngle, _axis));
            
        _previousTraversalNumber = nv->getTraversalNumber();
        _previousTime = currentTime;
    }

    // must call any nested node callbacks and continue subgraph traversal.
    traverse(node,nv);

}
