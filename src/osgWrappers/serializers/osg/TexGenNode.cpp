#include <osg/TexGenNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( TexGenNode,
                         new osg::TexGenNode,
                         osg::TexGenNode,
                         "osg::Object osg::Node osg::Group osg::TexGenNode" )
{
    ADD_UINT_SERIALIZER( TextureUnit, 0 );  // _textureUnit
    ADD_OBJECT_SERIALIZER( TexGen, osg::TexGen, NULL );  // _texgen

    BEGIN_ENUM_SERIALIZER( ReferenceFrame, RELATIVE_RF );
        ADD_ENUM_VALUE( RELATIVE_RF );
        ADD_ENUM_VALUE( ABSOLUTE_RF );
    END_ENUM_SERIALIZER();  // _referenceFrame
}
