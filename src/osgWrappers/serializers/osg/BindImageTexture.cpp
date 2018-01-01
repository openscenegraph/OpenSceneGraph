#include <osg/BindImageTexture>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define ADD_GLBOOL_SERIALIZER(PROP, DEF) \
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLboolean >( \
        #PROP, ((int)(DEF)), &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_BOOL )

REGISTER_OBJECT_WRAPPER( BindImageTexture,
                         new osg::BindImageTexture,
                         osg::BindImageTexture,
                         "osg::Object osg::StateAttribute osg::BindImageTexture" )
{

ADD_OBJECT_SERIALIZER( Texture, osg::Texture, NULL);
ADD_UINT_SERIALIZER(ImageUnit,0);
ADD_GLINT_SERIALIZER(Level,0);
ADD_GLBOOL_SERIALIZER(IsLayered,GL_FALSE);
ADD_GLINT_SERIALIZER(Layer,0);
BEGIN_ENUM_SERIALIZER( Access, NOT_USED );
      ADD_ENUM_VALUE( NOT_USED );
      ADD_ENUM_VALUE( READ_ONLY );
      ADD_ENUM_VALUE( WRITE_ONLY );
      ADD_ENUM_VALUE( READ_WRITE );
END_ENUM_SERIALIZER();
ADD_GLENUM_SERIALIZER(Format,GLenum,GL_RGBA8);

}

