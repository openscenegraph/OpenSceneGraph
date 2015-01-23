#include <osgFX/MultiTextureControl>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkTextureWeights( const osgFX::MultiTextureControl& ctrl )
{
    return ctrl.getNumTextureWeights()>0;
}

static bool readTextureWeights( osgDB::InputStream& is, osgFX::MultiTextureControl& ctrl )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        float weight = 0.0f; is >> weight;
        ctrl.setTextureWeight( i, weight );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeTextureWeights( osgDB::OutputStream& os, const osgFX::MultiTextureControl& ctrl )
{
    unsigned int size = ctrl.getNumTextureWeights();
    os.writeSize(size); os << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << ctrl.getTextureWeight(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgFX_MultiTextureControl,
                         new osgFX::MultiTextureControl,
                         osgFX::MultiTextureControl,
                         "osg::Object osg::Node osg::Group osgFX::MultiTextureControl" )
{
    ADD_USER_SERIALIZER( TextureWeights );  // _textureWeightList

    {
        UPDATE_TO_VERSION_SCOPED( 116 )

        REMOVE_SERIALIZER( TextureWeights );  // _textureWeightList

        ADD_OBJECT_SERIALIZER( TextureWeights, osgFX::MultiTextureControl::TextureWeights, NULL );
        ADD_BOOL_SERIALIZER( UseTexEnvCombine, true );
        ADD_BOOL_SERIALIZER( UseTextureWeightsUniform, true );
    }
}
