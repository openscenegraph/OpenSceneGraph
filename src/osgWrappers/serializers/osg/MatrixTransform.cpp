#include <osg/MatrixTransform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( MatrixTransform,
                         new osg::MatrixTransform,
                         osg::MatrixTransform,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform" )
{
    ADD_MATRIX_SERIALIZER( Matrix, osg::Matrix() );  // _matrix
}
