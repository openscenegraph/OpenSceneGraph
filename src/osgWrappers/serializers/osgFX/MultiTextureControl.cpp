#include <osgFX/MultiTextureControl>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgFX_MultiTextureControl,
                         new osgFX::MultiTextureControl,
                         osgFX::MultiTextureControl,
                         "osg::Object osg::Node osg::Group osgFX::MultiTextureControl" )
{
    ADD_VECTOR_SERIALIZER( TextureWeights, osgFX::MultiTextureControl::TextureWeightList, osgDB::BaseSerializer::RW_FLOAT, 1.0f );
    {
        UPDATE_TO_VERSION_SCOPED( 116 )
        ADD_BOOL_SERIALIZER( UseTexEnvCombine, true );
        ADD_BOOL_SERIALIZER( UseTextureWeightsUniform, true );
    }
}
