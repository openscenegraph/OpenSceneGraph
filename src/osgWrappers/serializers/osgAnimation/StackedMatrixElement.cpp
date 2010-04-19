#include <osgAnimation/StackedMatrixElement>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_StackedMatrixElement,
                         new osgAnimation::StackedMatrixElement,
                         osgAnimation::StackedMatrixElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedMatrixElement" )
{
    ADD_MATRIX_SERIALIZER( Matrix, osg::Matrix() );  // _matrix
}
