#include <osg/TextureBuffer>
#include <osg/Array>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkImage( const osg::TextureBuffer& texture )
{
    return texture.getImage() != NULL;
}

static bool readImage( osgDB::InputStream& is, osg::TextureBuffer& texture )
{
    if ( is.isBinary() )
    {
        osg::ref_ptr<osg::Image> image = is.readImage();
        texture.setImage(image.get());
    }
    else
    {        
        is >> is.BEGIN_BRACKET;
        osg::ref_ptr<osg::Image> image = is.readImage();
        texture.setImage(image.get());
        is >> is.END_BRACKET;    
    }    
    
    return true;
}

static bool writeImage( osgDB::OutputStream& os, const osg::TextureBuffer& texture )
{
    const osg::Image* image = texture.getImage();    

    if ( os.isBinary() )
    {        
        os.writeImage(image);
    }
    else
    {
        os << os.BEGIN_BRACKET << std::endl;
        os.writeImage(image);
        os << os.END_BRACKET << std::endl;
    }

    return true;
}


static bool checkBufferData( const osg::TextureBuffer& texture )
{
    return texture.getBufferData() != NULL && texture.getBufferData()->asArray() != NULL;
}

static bool readBufferData( osgDB::InputStream& is, osg::TextureBuffer& texture )
{
    if ( is.isBinary() )
    {
        osg::ref_ptr<osg::Array> bufferData = is.readArray();
        texture.setBufferData(bufferData);
    }
    else
    {
        is >> is.BEGIN_BRACKET;
        osg::ref_ptr<osg::Array> bufferData = is.readArray();
        texture.setBufferData(bufferData);
        is >> is.END_BRACKET;    
    }
    
    return true;
}

static bool writeBufferData( osgDB::OutputStream& os, const osg::TextureBuffer& texture )
{    
    const osg::Array* array = texture.getBufferData()->asArray();

    if ( os.isBinary() )
    {
        os.writeArray(array);
    }
    else
    {        
        os << os.BEGIN_BRACKET << std::endl;
        os.writeArray(array);
        os << os.END_BRACKET << std::endl;
    }    

    return true;
}

REGISTER_OBJECT_WRAPPER( TextureBuffer,
                         new osg::TextureBuffer,
                         osg::TextureBuffer,
                         "osg::Object osg::StateAttribute osg::Texture osg::TextureBuffer" )
{
    ADD_USER_SERIALIZER(Image);            // _bufferData as osg::Image
    ADD_USER_SERIALIZER(BufferData);       // _bufferData as osg::Array
    ADD_INT_SERIALIZER( TextureWidth, 0 ); // _textureWidth    
}