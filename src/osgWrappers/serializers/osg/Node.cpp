#include <osg/Node>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _initialBound
static bool checkInitialBound( const osg::Node& node )
{
    return node.getInitialBound().valid();
}

static bool readInitialBound( osgDB::InputStream& is, osg::Node& node )
{
    osg::Vec3d center;
    double radius;
    is >> osgDB::BEGIN_BRACKET;
    is >> osgDB::PROPERTY("Center") >> center;
    is >> osgDB::PROPERTY("Radius") >> radius;
    is >> osgDB::END_BRACKET;
    node.setInitialBound( osg::BoundingSphere(center, radius) );
    return true;
}

static bool writeInitialBound( osgDB::OutputStream& os, const osg::Node& node )
{
    const osg::BoundingSphere& bs = node.getInitialBound();
    os << osgDB::BEGIN_BRACKET << std::endl;
    os << osgDB::PROPERTY("Center") << osg::Vec3d(bs.center()) << std::endl;
    os << osgDB::PROPERTY("Radius") << double(bs.radius()) << std::endl;
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

// _descriptions
static bool checkDescriptions( const osg::Node& node )
{
    return node.getDescriptions().size()>0;
}

static bool readDescriptions( osgDB::InputStream& is, osg::Node& node )
{
    unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string value;
        is.readWrappedString( value );
        node.addDescription( value );
    }
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeDescriptions( osgDB::OutputStream& os, const osg::Node& node )
{
    const osg::Node::DescriptionList& slist = node.getDescriptions();
    os << slist.size() << osgDB::BEGIN_BRACKET << std::endl;
    for ( osg::Node::DescriptionList::const_iterator itr=slist.begin();
          itr!=slist.end(); ++itr )
    {
        os.writeWrappedString( *itr );
        os << std::endl;
    }
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Node,
                         new osg::Node,
                         osg::Node,
                         "osg::Object osg::Node" )
{
    ADD_USER_SERIALIZER( InitialBound );  // _initialBound
    ADD_OBJECT_SERIALIZER( ComputeBoundingSphereCallback,
                           osg::Node::ComputeBoundingSphereCallback, NULL );  // _computeBoundCallback
    ADD_OBJECT_SERIALIZER( UpdateCallback, osg::NodeCallback, NULL );  // _updateCallback
    ADD_OBJECT_SERIALIZER( EventCallback, osg::NodeCallback, NULL );  // _eventCallback
    ADD_OBJECT_SERIALIZER( CullCallback, osg::NodeCallback, NULL );  // _cullCallback
    ADD_BOOL_SERIALIZER( CullingActive, true );  // _cullingActive
    ADD_HEXINT_SERIALIZER( NodeMask, 0xffffffff );  // _nodeMask
    ADD_USER_SERIALIZER( Descriptions );  // _descriptions
    ADD_OBJECT_SERIALIZER( StateSet, osg::StateSet, NULL );  // _stateset
}
