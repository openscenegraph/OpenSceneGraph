#include <osg/PositionAttitudeTransform>

using namespace osg;

PositionAttitudeTransform::PositionAttitudeTransform()
{
}

const bool PositionAttitudeTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_mode==MODEL || _mode==MODELVIEW)
    {
        matrix.makeRotate(_attitude);
        matrix.setTrans(_position);
        return true;
    }
    else // _mode==VIEW
    {
        matrix.makeTranslate(-_position);
        matrix.postMult(osg::Matrix::rotate(_attitude.inverse()));
        return true;
    }
}


const bool PositionAttitudeTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_mode==MODEL || _mode==MODELVIEW)
    {
        matrix.makeTranslate(-_position);
        matrix.postMult(osg::Matrix::rotate(_attitude.inverse()));
        return true;
    }
    else // _mode==VIEW
    {
        matrix.makeRotate(_attitude);
        matrix.setTrans(_position);
        return true;
    }
}
