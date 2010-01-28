#include <osg/ProxyNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _filenameList
static bool checkFileNames( const osg::ProxyNode& node )
{
    return node.getNumFileNames()>0;
}

static bool readFileNames( osgDB::InputStream& is, osg::ProxyNode& node )
{
    unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string value;
        is.readWrappedString( value );
        node.setFileName( i, value );
    }
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeFileNames( osgDB::OutputStream& os, const osg::ProxyNode& node )
{
    os << node.getNumFileNames() << osgDB::BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<node.getNumFileNames(); ++i )
    {
        os.writeWrappedString( node.getFileName(i) );
        os << std::endl;
    }
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

// _userDefinedCenter, _radius
static bool checkUserCenter( const osg::ProxyNode& node )
{
    return node.getCenterMode()==osg::ProxyNode::USER_DEFINED_CENTER;
}

static bool readUserCenter( osgDB::InputStream& is, osg::ProxyNode& node )
{
    osg::Vec3d center; double radius;
    is >> center >> radius;
    node.setCenter( center ); node.setRadius( radius );
    return true;
}

static bool writeUserCenter( osgDB::OutputStream& os, const osg::ProxyNode& node )
{
    os << osg::Vec3d(node.getCenter()) << (double)node.getRadius() << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( ProxyNode,
                         new osg::ProxyNode,
                         osg::ProxyNode,
                         "osg::Object osg::Node osg::ProxyNode" )
{
    // Note: osg::Group is not in the list to prevent recording dynamic loaded children
    
    ADD_USER_SERIALIZER( FileNames );  // _filenameList
    ADD_STRING_SERIALIZER( DatabasePath, "" );  // _databasePath
    
    BEGIN_ENUM_SERIALIZER( LoadingExternalReferenceMode, LOAD_IMMEDIATELY );
        ADD_ENUM_VALUE( LOAD_IMMEDIATELY );
        ADD_ENUM_VALUE( DEFER_LOADING_TO_DATABASE_PAGER );
        ADD_ENUM_VALUE( NO_AUTOMATIC_LOADING );
    END_ENUM_SERIALIZER();  // _loadingExtReference
    
    BEGIN_ENUM_SERIALIZER( CenterMode, USE_BOUNDING_SPHERE_CENTER );
        ADD_ENUM_VALUE( USE_BOUNDING_SPHERE_CENTER );
        ADD_ENUM_VALUE( USER_DEFINED_CENTER );
    END_ENUM_SERIALIZER();  // _centerMode
    
    ADD_USER_SERIALIZER( UserCenter );  // _userDefinedCenter, _radius
}
