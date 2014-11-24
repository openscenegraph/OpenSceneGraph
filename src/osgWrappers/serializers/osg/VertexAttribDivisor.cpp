#include <osg/VertexAttribDivisor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( VertexAttribDivisor,
                         new osg::VertexAttribDivisor,
                         osg::VertexAttribDivisor,
                         "osg::Object osg::StateAttribute osg::VertexAttribDivisor" )
{
    ADD_UINT_SERIALIZER( Index, 0 );
    ADD_UINT_SERIALIZER( Divisor, 0 );
}
