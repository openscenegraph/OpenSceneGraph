#include <osg/PolygonStipple>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkMask( const osg::PolygonStipple& attr )
{
    return true;
}

static bool readMask( osgDB::InputStream& is, osg::PolygonStipple& attr )
{
    char mask[128] = {0};
    if ( is.isBinary() )
    {
        unsigned int size; is >> size;
        is.readCharArray( mask, size );
    }
    else
    {
        is >> osgDB::BEGIN_BRACKET;
        for ( unsigned int i=0; i<128; ++i )
        {
            is >> std::hex >> mask[i] >> std::dec;
        }
        is >> osgDB::END_BRACKET;
    }
    attr.setMask( (GLubyte*)mask );
    return true;
}

static bool writeMask( osgDB::OutputStream& os, const osg::PolygonStipple& attr )
{
    if ( os.isBinary() )
    {
        os << (unsigned int)128;
        os.writeCharArray( (char*)attr.getMask(), 128 );
    }
    else
    {
        const GLubyte* mask = attr.getMask();
        os << osgDB::BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<128; ++i )
        {
            os << std::hex << mask[i] << std::dec << std::endl;
        }
        os << osgDB::END_BRACKET << std::endl;
    }
    return true;
}

REGISTER_OBJECT_WRAPPER( PolygonStipple,
                         new osg::PolygonStipple,
                         osg::PolygonStipple,
                         "osg::Object osg::StateAttribute osg::PolygonStipple" )
{
    ADD_USER_SERIALIZER( Mask );  // _mask
}
