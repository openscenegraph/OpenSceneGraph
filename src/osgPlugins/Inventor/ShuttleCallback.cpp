#include <osg/MatrixTransform>

#include "ShuttleCallback.h"

ShuttleCallback::ShuttleCallback(const osg::Vec3& startPos,
                                 const osg::Vec3& endPos,
                                 float frequency)
{
    _startPos = startPos;
    _endPos = endPos;
    _frequency = frequency;

    _previousTraversalNumber = -1;
    _previousTime = -1.0;
    _angle = 0.0;
}

void ShuttleCallback::operator() (osg::Node* node, osg::NodeVisitor* nv)
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
        double currentTime = fs->getSimulationTime();
        if (_previousTime == -1.)
            _previousTime = currentTime;
        _angle += (currentTime - _previousTime) * 2 * osg::PI * _frequency;

        double frac = 0.5 - 0.5 * cos(_angle);

        osg::Vec3 position = _startPos * (1.0 - frac) + _endPos * frac;

        // update the specified transform
        transform->setMatrix(osg::Matrix::translate(position));

        _previousTraversalNumber = nv->getTraversalNumber();
        _previousTime = currentTime;
    }

    // must call any nested node callbacks and continue subgraph traversal.
    traverse(node,nv);

}

