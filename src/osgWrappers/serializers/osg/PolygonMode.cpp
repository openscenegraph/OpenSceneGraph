#include <osg/PolygonMode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

BEGIN_USER_TABLE( Mode, osg::PolygonMode );
    ADD_USER_VALUE( POINT );
    ADD_USER_VALUE( LINE );
    ADD_USER_VALUE( FILL );
END_USER_TABLE()

USER_READ_FUNC( Mode, readModeValue )
USER_WRITE_FUNC( Mode, writeModeValue )

// _modeFront, _modeBack
static bool checkMode( const osg::PolygonMode& attr )
{
    return true;
}

static bool readMode( osgDB::InputStream& is, osg::PolygonMode& attr )
{
    bool frontAndBack;
    is >> is.PROPERTY("UseFrontAndBack") >> frontAndBack;
    is >> is.PROPERTY("Front"); int value1 = readModeValue(is);
    is >> is.PROPERTY("Back"); int value2 = readModeValue(is);

    if ( frontAndBack )
        attr.setMode( osg::PolygonMode::FRONT_AND_BACK, (osg::PolygonMode::Mode)value1 );
    else
    {
        attr.setMode(osg::PolygonMode::FRONT, (osg::PolygonMode::Mode)value1);
        attr.setMode(osg::PolygonMode::BACK, (osg::PolygonMode::Mode)value2);
    }
    return true;
}

static bool writeMode( osgDB::OutputStream& os, const osg::PolygonMode& attr )
{
    os << os.PROPERTY("UseFrontAndBack") << attr.getFrontAndBack() << std::endl;

    os << os.PROPERTY("Front");
    writeModeValue( os, (int)attr.getMode(osg::PolygonMode::FRONT) );
    os << std::endl;

    os << os.PROPERTY("Back");
    writeModeValue( os, (int)attr.getMode(osg::PolygonMode::BACK) );
    os << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( PolygonMode,
                         new osg::PolygonMode,
                         osg::PolygonMode,
                         "osg::Object osg::StateAttribute osg::PolygonMode" )
{
    ADD_USER_SERIALIZER( Mode );  // _modeFront, _modeBack
}
