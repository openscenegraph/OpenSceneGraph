#include <osg/PositionAttitudeTransform>

using namespace osg;

PositionAttitudeTransform::PositionAttitudeTransform()
{
}

void PositionAttitudeTransform::computeLocalToWorld() const
{
    if (_localToWorldDirty) 
    {
        if (_mode==MODEL)
        {
            _localToWorld->makeRotate(_attitude);
            _localToWorld->setTrans(_position);
        }
        else
        {
            _localToWorld->makeTranslate(-_position);
            _localToWorld->postMult(osg::Matrix::rotate(_attitude.inverse()));
        }
        _localToWorldDirty = false;
    }
}

void PositionAttitudeTransform::computeWorldToLocal() const
{
    if (_worldToLocalDirty) 
    {
        if (_mode==MODEL)
        {
            _worldToLocal->makeTranslate(-_position);
            _worldToLocal->postMult(osg::Matrix::rotate(_attitude.inverse()));
        }
        else
        {
            _worldToLocal->makeRotate(_attitude);
            _worldToLocal->setTrans(_position);
        }
        _worldToLocalDirty = false;
    }
}
