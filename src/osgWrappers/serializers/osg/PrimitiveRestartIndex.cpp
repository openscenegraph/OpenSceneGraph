#include <osg/PrimitiveRestartIndex>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkRestartIndex( const osg::PrimitiveRestartIndex& )
{
    return true;
}

static bool readRestartIndex( osgDB::InputStream& is, osg::PrimitiveRestartIndex& attr )
{
    if ( is.getFileVersion() > 97 )
    {
        unsigned int restartIndex;
        is >> restartIndex;
        attr.setRestartIndex( restartIndex );
    }
    return true;
}

static bool writeRestartIndex( osgDB::OutputStream& os, const osg::PrimitiveRestartIndex& attr )
{
    os << attr.getRestartIndex() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( PrimitiveRestartIndex,
                         new osg::PrimitiveRestartIndex,
                         osg::PrimitiveRestartIndex,
                         "osg::Object osg::StateAttribute osg::PrimitiveRestartIndex" )
{
    ADD_USER_SERIALIZER( RestartIndex );
}
