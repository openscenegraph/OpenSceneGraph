#include <osg/SampleMaski>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkMasks( const osg::SampleMaski& )
{
    return true;
}

static bool readMasks( osgDB::InputStream& is, osg::SampleMaski& attr )
{
    if ( is.getFileVersion() > 96 )
    {
        unsigned int mask0, mask1;
        is >> mask0 >> mask1;
        attr.setMask( mask0, 0 );
        attr.setMask( mask1, 1 );
    }
    return true;
}

static bool writeMasks( osgDB::OutputStream& os, const osg::SampleMaski& attr )
{
    os << attr.getMask( 0u ) << attr.getMask( 1u ) << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( SampleMaski,
                         new osg::SampleMaski,
                         osg::SampleMaski,
                         "osg::Object osg::StateAttribute osg::SampleMaski" )
{
    ADD_USER_SERIALIZER( Masks );  //
}
