#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osg/AnimationPath>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkTimeControlPointMap( const osg::AnimationPath& path )
{
    return path.getTimeControlPointMap().size()>0;
}

static bool readTimeControlPointMap( osgDB::InputStream& is, osg::AnimationPath& path )
{
    unsigned int size = is.readSize();
    if ( size>0 )
    {
        is >> osgDB::BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            double time = 0.0;
            osg::Vec3d pos, scale;
            osg::Quat rot;
            is >> osgDB::PROPERTY("Time") >> time >> osgDB::BEGIN_BRACKET;
            is >> osgDB::PROPERTY("Position") >> pos;
            is >> osgDB::PROPERTY("Rotation") >> rot;
            is >> osgDB::PROPERTY("Scale") >> scale;
            is >> osgDB::END_BRACKET;
            path.insert( time, osg::AnimationPath::ControlPoint(pos, rot, scale) );
        }
        is >> osgDB::END_BRACKET;
    }
    return true;
}

static bool writeTimeControlPointMap( osgDB::OutputStream& os, const osg::AnimationPath& path )
{
    const osg::AnimationPath::TimeControlPointMap& map = path.getTimeControlPointMap();
    os.writeSize(map.size());
    if ( map.size()>0 )
    {
        os << osgDB::BEGIN_BRACKET << std::endl;
        for ( osg::AnimationPath::TimeControlPointMap::const_iterator itr=map.begin();
              itr!=map.end(); ++itr )
        {
            const osg::AnimationPath::ControlPoint& pt = itr->second;
            os << osgDB::PROPERTY("Time") << itr->first << osgDB::BEGIN_BRACKET << std::endl;
            os << osgDB::PROPERTY("Position") << pt.getPosition() << std::endl;
            os << osgDB::PROPERTY("Rotation") << pt.getRotation() << std::endl;
            os << osgDB::PROPERTY("Scale") << pt.getScale() << std::endl;
            os << osgDB::END_BRACKET << std::endl;
        }
        os << osgDB::END_BRACKET;
    }
    os << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( AnimationPath,
                         new osg::AnimationPath,
                         osg::AnimationPath,
                         "osg::Object osg::AnimationPath" )
{
    ADD_USER_SERIALIZER( TimeControlPointMap );  // _timeControlPointMap
    
    BEGIN_ENUM_SERIALIZER( LoopMode, NO_LOOPING );
        ADD_ENUM_VALUE( SWING );
        ADD_ENUM_VALUE( LOOP );
        ADD_ENUM_VALUE( NO_LOOPING );
    END_ENUM_SERIALIZER();  //_loopMode
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
