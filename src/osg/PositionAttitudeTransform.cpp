#include <osg/PositionAttitudeTransform>

using namespace osg;

PositionAttitudeTransform::PositionAttitudeTransform()
{
}

const bool PositionAttitudeTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
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


const bool PositionAttitudeTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
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
