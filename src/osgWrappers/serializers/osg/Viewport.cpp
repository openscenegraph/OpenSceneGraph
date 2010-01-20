#include <osg/Viewport>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkArea( const osg::Viewport& attr )
{
    return true;
}

static bool readArea( osgDB::InputStream& is, osg::Viewport& attr )
{
    double x, y, w, h;
    is >> x >> y >> w >> h;
    attr.setViewport( x, y, w, h );
    return true;
}

static bool writeArea( osgDB::OutputStream& os, const osg::Viewport& attr )
{
    os << attr.x() << attr.y() << attr.width() << attr.height() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Viewport,
                         new osg::Viewport,
                         osg::Viewport,
                         "osg::Object osg::StateAttribute osg::Viewport" )
{
    ADD_USER_SERIALIZER( Area );  // _x, _y, _width, _height
}
