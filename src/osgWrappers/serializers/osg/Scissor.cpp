#include <osg/Scissor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkArea( const osg::Scissor& attr )
{
    return true;
}

static bool readArea( osgDB::InputStream& is, osg::Scissor& attr )
{
    int x, y, w, h;
    is >> x >> y >> w >> h;
    attr.setScissor( x, y, w, h );
    return true;
}

static bool writeArea( osgDB::OutputStream& os, const osg::Scissor& attr )
{
    os << attr.x() << attr.y() << attr.width() << attr.height() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Scissor,
                         new osg::Scissor,
                         osg::Scissor,
                         "osg::Object osg::StateAttribute osg::Scissor" )
{
    ADD_USER_SERIALIZER( Area );  // _x, _y, _width, _height
}
