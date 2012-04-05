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
        is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            double time = 0.0;
            osg::Vec3d pos, scale;
            osg::Quat rot;
            is >> is.PROPERTY("Time") >> time >> is.BEGIN_BRACKET;
            is >> is.PROPERTY("Position") >> pos;
            is >> is.PROPERTY("Rotation") >> rot;
            is >> is.PROPERTY("Scale") >> scale;
            is >> is.END_BRACKET;
            path.insert( time, osg::AnimationPath::ControlPoint(pos, rot, scale) );
        }
        is >> is.END_BRACKET;
    }
    return true;
}

static bool writeTimeControlPointMap( osgDB::OutputStream& os, const osg::AnimationPath& path )
{
    const osg::AnimationPath::TimeControlPointMap& map = path.getTimeControlPointMap();
    os.writeSize(map.size());
    if ( map.size()>0 )
    {
        os << os.BEGIN_BRACKET << std::endl;
        for ( osg::AnimationPath::TimeControlPointMap::const_iterator itr=map.begin();
              itr!=map.end(); ++itr )
        {
            const osg::AnimationPath::ControlPoint& pt = itr->second;
            os << os.PROPERTY("Time") << itr->first << os.BEGIN_BRACKET << std::endl;
            os << os.PROPERTY("Position") << pt.getPosition() << std::endl;
            os << os.PROPERTY("Rotation") << pt.getRotation() << std::endl;
            os << os.PROPERTY("Scale") << pt.getScale() << std::endl;
            os << os.END_BRACKET << std::endl;
        }
        os << os.END_BRACKET;
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

    BEGIN_ENUM_SERIALIZER( LoopMode, LOOP );
        ADD_ENUM_VALUE( SWING );
        ADD_ENUM_VALUE( LOOP );
        ADD_ENUM_VALUE( NO_LOOPING );
    END_ENUM_SERIALIZER();  //_loopMode
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
