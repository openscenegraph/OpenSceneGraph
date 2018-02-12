#include <osg/TextureCubeMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define FACE_IMAGE_FUNCTION( PROP, FACE ) \
    static bool check##PROP( const osg::TextureCubeMap& tex ) { return true; } \
    static bool read##PROP( osgDB::InputStream& is, osg::TextureCubeMap& tex ) { \
        bool hasImage; is >> hasImage; \
        if ( hasImage ) { \
            is >> is.BEGIN_BRACKET; tex.setImage(FACE, is.readImage()); \
            is >> is.END_BRACKET; \
        } \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::TextureCubeMap& tex ) { \
        const osg::Image* image = tex.getImage(FACE); \
        os << (image!=NULL); \
        if ( image!=NULL ) { \
            os << os.BEGIN_BRACKET << std::endl << image; \
            os << os.END_BRACKET; \
        } \
        os << std::endl; \
        return true; \
    }

FACE_IMAGE_FUNCTION( PosX, osg::TextureCubeMap::POSITIVE_X )
FACE_IMAGE_FUNCTION( NegX, osg::TextureCubeMap::NEGATIVE_X )
FACE_IMAGE_FUNCTION( PosY, osg::TextureCubeMap::POSITIVE_Y )
FACE_IMAGE_FUNCTION( NegY, osg::TextureCubeMap::NEGATIVE_Y )
FACE_IMAGE_FUNCTION( PosZ, osg::TextureCubeMap::POSITIVE_Z )
FACE_IMAGE_FUNCTION( NegZ, osg::TextureCubeMap::NEGATIVE_Z )

struct TextureCubeGetValue : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        unsigned int index = 0;
        osg::ValueObject* indexObject = inputParameters[0]->asValueObject();
        if (indexObject) indexObject->getScalarValue(index);

        osg::TextureCubeMap* tcm = reinterpret_cast<osg::TextureCubeMap*>(objectPtr);
        outputParameters.push_back(tcm->getImage(index));

        return true;
    }
};


struct TextureCubeSetValue : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.size()<2) return false;

        unsigned int index = 0;
        osg::ValueObject* indexObject = inputParameters[0]->asValueObject();
        if (indexObject) indexObject->getScalarValue(index);

        osg::Image* image = inputParameters[1]->asImage();
        if (!image) return false;

        osg::TextureCubeMap* tcm = reinterpret_cast<osg::TextureCubeMap*>(objectPtr);
        tcm->setImage(index, image);

        return true;
    }
};

REGISTER_OBJECT_WRAPPER( TextureCubeMap,
                         new osg::TextureCubeMap,
                         osg::TextureCubeMap,
                         "osg::Object osg::StateAttribute osg::Texture osg::TextureCubeMap" )
{
    ADD_USER_SERIALIZER( PosX );
    ADD_USER_SERIALIZER( NegX );
    ADD_USER_SERIALIZER( PosY );
    ADD_USER_SERIALIZER( NegY );
    ADD_USER_SERIALIZER( PosZ );
    ADD_USER_SERIALIZER( NegZ );  // _images

    ADD_INT_SERIALIZER( TextureWidth, 0 );  // _textureWidth
    ADD_INT_SERIALIZER( TextureHeight, 0 );  // _textureHeight

    ADD_METHOD_OBJECT( "getImage", TextureCubeGetValue );
    ADD_METHOD_OBJECT( "setImage", TextureCubeSetValue );
}
