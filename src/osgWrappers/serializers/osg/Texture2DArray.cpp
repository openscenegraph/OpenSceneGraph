#include <osg/Texture2DArray>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkImages( const osg::Texture2DArray& tex )
{
    return tex.getNumImages()>0;
}

static bool readImages( osgDB::InputStream& is, osg::Texture2DArray& tex )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    tex.setTextureDepth(size);
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Image* image = is.readImage();
        if ( image ) tex.setImage( i, image );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeImages( osgDB::OutputStream& os, const osg::Texture2DArray& tex )
{
    unsigned int size = tex.getNumImages();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << tex.getImage(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Texture2DArray,
                         new osg::Texture2DArray,
                         osg::Texture2DArray,
                         "osg::Object osg::StateAttribute osg::Texture osg::Texture2DArray" )
{
    ADD_USER_SERIALIZER( Images );  // _images
    ADD_INT_SERIALIZER( TextureWidth, 0 );  // _textureWidth
    ADD_INT_SERIALIZER( TextureHeight, 0 );  // _textureHeight
    ADD_INT_SERIALIZER( TextureDepth, 0 );  // _textureDepth
}
