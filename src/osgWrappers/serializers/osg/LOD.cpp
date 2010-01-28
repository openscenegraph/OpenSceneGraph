#include <osg/LOD>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _userDefinedCenter, _radius
static bool checkUserCenter( const osg::LOD& node )
{
    return node.getCenterMode()==osg::LOD::USER_DEFINED_CENTER;
}

static bool readUserCenter( osgDB::InputStream& is, osg::LOD& node )
{
    osg::Vec3d center; double radius;
    is >> center >> radius;
    node.setCenter( center ); node.setRadius( radius );
    return true;
}

static bool writeUserCenter( osgDB::OutputStream& os, const osg::LOD& node )
{
    os << osg::Vec3d(node.getCenter()) << (double)node.getRadius() << std::endl;
    return true;
}

// _rangeList
static bool checkRangeList( const osg::LOD& node )
{
    return node.getNumRanges()>0;
}

static bool readRangeList( osgDB::InputStream& is, osg::LOD& node )
{
    unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        float min, max;
        is >> min >> max;
        node.setRange( i, min, max );
    }
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeRangeList( osgDB::OutputStream& os, const osg::LOD& node )
{
    const osg::LOD::RangeList& ranges = node.getRangeList();
    os << ranges.size() << osgDB::BEGIN_BRACKET << std::endl;
    for ( osg::LOD::RangeList::const_iterator itr=ranges.begin();
          itr!=ranges.end(); ++itr )
    {
        os << itr->first << itr->second << std::endl;
    }
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( LOD,
                         new osg::LOD,
                         osg::LOD,
                         "osg::Object osg::Node osg::Group osg::LOD" )
{
    BEGIN_ENUM_SERIALIZER( CenterMode, USE_BOUNDING_SPHERE_CENTER );
        ADD_ENUM_VALUE( USE_BOUNDING_SPHERE_CENTER );
        ADD_ENUM_VALUE( USER_DEFINED_CENTER );
    END_ENUM_SERIALIZER();  // _centerMode
    
    ADD_USER_SERIALIZER( UserCenter );  // _userDefinedCenter, _radius
    
    BEGIN_ENUM_SERIALIZER( RangeMode, DISTANCE_FROM_EYE_POINT );
        ADD_ENUM_VALUE( DISTANCE_FROM_EYE_POINT );
        ADD_ENUM_VALUE( PIXEL_SIZE_ON_SCREEN );
    END_ENUM_SERIALIZER();  // _rangeMode
    
    ADD_USER_SERIALIZER( RangeList );  // _rangeList
}
