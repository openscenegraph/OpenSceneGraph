#include <osg/PositionAttitudeTransform>

using namespace osg;

PositionAttitudeTransform::PositionAttitudeTransform()
{
}

const bool PositionAttitudeTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_TO_PARENTS)
    {
        osg::Matrix tmp;
        tmp.makeRotate(_attitude);
        tmp.setTrans(_position);

        matrix.preMult(tmp);
    }
    else // absolute
    {
        matrix.makeRotate(_attitude);
        matrix.setTrans(_position);
    }
    return true;
}


const bool PositionAttitudeTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_TO_PARENTS)
    {
        osg::Matrix tmp;
        tmp.makeTranslate(-_position);
        tmp.postMult(osg::Matrix::rotate(_attitude.inverse()));
        matrix.postMult(tmp);
    }
    else // absolute
    {
        matrix.makeTranslate(-_position);
        matrix.postMult(osg::Matrix::rotate(_attitude.inverse()));
    }
    return true;
}
