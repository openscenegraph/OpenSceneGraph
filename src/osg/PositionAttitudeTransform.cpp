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

void PositionAttitudeTransform::AnimationPathCallback::operator()(Node* node, NodeVisitor* nv)
{
    PositionAttitudeTransform* pat = dynamic_cast<PositionAttitudeTransform*>(node);
    if (pat &&
        _animationPath.valid() && 
        nv->getVisitorType()==NodeVisitor::APP_VISITOR && 
        nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getReferenceTime();
        if (_firstTime==0.0) _firstTime = time;
        AnimationPath::ControlPoint cp;
        if (_animationPath->getInterpolatedControlPoint(((time-_firstTime)-_timeOffset)*_timeMultiplier,cp))
        {
            pat->setPosition(cp._position);
            pat->setAttitude(cp._rotation);
        }
    }
    // must call any nested node callbacks and continue subgraph traversal.
    traverse(node,nv);
}
