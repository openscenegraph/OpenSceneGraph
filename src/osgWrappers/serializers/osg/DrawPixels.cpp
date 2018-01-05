#include <osg/DrawPixels>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkArea( const osg::DrawPixels& drawable )
{
    return drawable.getUseSubImage();
}

static bool readArea( osgDB::InputStream& is, osg::DrawPixels& drawable )
{
    unsigned int x, y, w, h;
    is >> x >> y >> w >> h;
    drawable.setSubImageDimensions( x, y, w, h );
    return true;
}

static bool writeArea( osgDB::OutputStream& os, const osg::DrawPixels& drawable )
{
    unsigned int x, y, w, h;
    drawable.getSubImageDimensions( x, y, w, h );
    os << x << y << w << h << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( DrawPixels,
                         new osg::DrawPixels,
                         osg::DrawPixels,
                         "osg::Object osg::Node osg::Drawable osg::DrawPixels" )
{
    {
         UPDATE_TO_VERSION_SCOPED( 154 )
         ADDED_ASSOCIATE("osg::Node")
    }

    ADD_VEC3_SERIALIZER( Position, osg::Vec3() );  // _position
    ADD_IMAGE_SERIALIZER( Image, osg::Image, NULL );  // _image
    ADD_BOOL_SERIALIZER( UseSubImage, false );  // _useSubImage
    ADD_USER_SERIALIZER( Area );  // _offsetX, _offsetY, _width, _height
}
