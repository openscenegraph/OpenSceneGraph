#include <osg/BufferIndexBinding>
#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define ADD_GLUINT_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLuint >( \
        #PROP, ((unsigned int)(DEF)), &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_UINT )

#define GLsizei_ptrSERIALIZER(TYPE,XXX) \
static bool check##XXX( const TYPE& node )\
{    return node.get##XXX()>0;}\
static bool read##XXX( osgDB::InputStream& is, TYPE& node )\
{\
    unsigned int size = 0; is >> size ;    node.set##XXX(size);    return true;\
}\
static bool write##XXX( osgDB::OutputStream& os, const TYPE& node )\
{\
    unsigned int size = node.get##XXX();    os << size << std::endl;    return true;\
}

GLsizei_ptrSERIALIZER(osg::BufferIndexBinding,Size)
GLsizei_ptrSERIALIZER(osg::BufferIndexBinding,Offset)


namespace wrap_osgBufferIndexBinding
{

REGISTER_OBJECT_WRAPPER( BufferIndexBinding,
                         /*new osg::BufferIndexBinding*/NULL,
                         osg::BufferIndexBinding,
                         "osg::Object osg::StateAttribute osg::BufferIndexBinding" )
{

    ADD_GLENUM_SERIALIZER( Target,GLenum, GL_BUFFER);  // _target
    ADD_OBJECT_SERIALIZER( BufferData, osg::BufferData, NULL );  // _bufferObject
    ADD_GLUINT_SERIALIZER( Index, 0);  // _index

    ADD_USER_SERIALIZER(Size);
    ADD_USER_SERIALIZER(Offset);
}
}
namespace wrap_osgUniformBufferBinding
{

REGISTER_OBJECT_WRAPPER( UniformBufferBinding,
                         new osg::UniformBufferBinding,
                         osg::UniformBufferBinding,
                         "osg::Object osg::StateAttribute osg::BufferIndexBinding osg::UniformBufferBinding" )
{
}
}
namespace wrap_osgTransformFeedbackBufferBinding
{

REGISTER_OBJECT_WRAPPER( TransformFeedbackBufferBinding,
                         new osg::TransformFeedbackBufferBinding,
                         osg::TransformFeedbackBufferBinding,
                         "osg::Object osg::StateAttribute osg::BufferIndexBinding osg::TransformFeedbackBufferBinding" )
{
}
}

namespace wrap_osgAtomicCounterBufferBinding
{

REGISTER_OBJECT_WRAPPER( AtomicCounterBufferBinding,
                         new osg::AtomicCounterBufferBinding,
                         osg::AtomicCounterBufferBinding,
                         "osg::Object osg::StateAttribute osg::BufferIndexBinding osg::AtomicCounterBufferBinding" )
{
}
}
namespace wrap_osgShaderStorageBufferBinding
{

REGISTER_OBJECT_WRAPPER( ShaderStorageBufferBinding,
                         new osg::ShaderStorageBufferBinding,
                         osg::ShaderStorageBufferBinding,
                         "osg::Object osg::StateAttribute osg::BufferIndexBinding osg::ShaderStorageBufferBinding" )
{
}
}


