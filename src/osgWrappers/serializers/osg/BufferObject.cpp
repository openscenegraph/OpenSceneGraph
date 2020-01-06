#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( BufferObject,
                         /*new osg::BufferObject*/NULL,
                         osg::BufferObject,
                         "osg::Object osg::BufferObject" )
{
    ADD_GLENUM_SERIALIZER( Target, GLenum, GL_ARRAY_BUFFER_ARB);  // _type
    ADD_GLENUM_SERIALIZER( Usage, GLenum, GL_STATIC_DRAW_ARB);  // _usage
    ADD_BOOL_SERIALIZER( CopyDataAndReleaseGLBufferObject, false);
    {
        UPDATE_TO_VERSION_SCOPED( 201 )
        ADD_HEXINT_SERIALIZER( MappingBitfield, 0x0 );  // _mappingBitField
    }
}
