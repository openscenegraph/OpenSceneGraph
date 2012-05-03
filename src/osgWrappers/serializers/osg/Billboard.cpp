#include <osg/Billboard>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkPositionList( const osg::Billboard& node )
{
    return node.getPositionList().size()>0;
}

static bool readPositionList( osgDB::InputStream& is, osg::Billboard& node )
{
    unsigned int size = is.readSize();
    is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Vec3d pos; is >> pos;
        node.setPosition( i, pos );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writePositionList( osgDB::OutputStream& os, const osg::Billboard& node )
{
    const osg::Billboard::PositionList& posList = node.getPositionList();
    os.writeSize(posList.size());
    os<< os.BEGIN_BRACKET << std::endl;
    for ( osg::Billboard::PositionList::const_iterator itr=posList.begin();
          itr!=posList.end(); ++itr )
    {
        os << osg::Vec3d(*itr) << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Billboard,
                         new osg::Billboard,
                         osg::Billboard,
                         "osg::Object osg::Node osg::Geode osg::Billboard" )
{
    BEGIN_ENUM_SERIALIZER( Mode, AXIAL_ROT );
        ADD_ENUM_VALUE( POINT_ROT_EYE );
        ADD_ENUM_VALUE( POINT_ROT_WORLD );
        ADD_ENUM_VALUE( AXIAL_ROT );
    END_ENUM_SERIALIZER();  // _mode

    ADD_VEC3_SERIALIZER( Axis, osg::Vec3f() );  // _axis
    ADD_VEC3_SERIALIZER( Normal, osg::Vec3f() );  // _normal
    ADD_USER_SERIALIZER( PositionList );  // _positionList
}
