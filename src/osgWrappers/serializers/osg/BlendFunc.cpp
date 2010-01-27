#include <osg/BlendFunc>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( BlendFunc,
                         new osg::BlendFunc,
                         osg::BlendFunc,
                         "osg::Object osg::StateAttribute osg::BlendFunc" )
{
    ADD_GLENUM_SERIALIZER( SourceRGB, GLenum, GL_NONE );  // _source_factor
    ADD_GLENUM_SERIALIZER( SourceAlpha, GLenum, GL_NONE );  // _source_factor_alpha
    ADD_GLENUM_SERIALIZER( DestinationRGB, GLenum, GL_NONE );  // _destination_factor
    ADD_GLENUM_SERIALIZER( DestinationAlpha, GLenum, GL_NONE );  // _destination_factor_alpha
}
