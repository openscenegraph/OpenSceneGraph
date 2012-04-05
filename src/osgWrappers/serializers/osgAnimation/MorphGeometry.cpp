#include <osgAnimation/MorphGeometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkMorphTargets( const osgAnimation::MorphGeometry& geom )
{
    return geom.getMorphTargetList().size()>0;
}

static bool readMorphTargets( osgDB::InputStream& is, osgAnimation::MorphGeometry& geom )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        float weight = 0.0f;
        is >> is.PROPERTY("MorphTarget") >> weight;
        osg::Geometry* target = dynamic_cast<osg::Geometry*>( is.readObject() );
        if ( target ) geom.addMorphTarget( target, weight );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeMorphTargets( osgDB::OutputStream& os, const osgAnimation::MorphGeometry& geom )
{
    const osgAnimation::MorphGeometry::MorphTargetList& targets = geom.getMorphTargetList();
    os.writeSize(targets.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osgAnimation::MorphGeometry::MorphTargetList::const_iterator itr=targets.begin();
          itr!=targets.end(); ++itr )
    {
        os << os.PROPERTY("MorphTarget") << itr->getWeight() << std::endl;
        os << itr->getGeometry();
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgAnimation_MorphGeometry,
                         new osgAnimation::MorphGeometry,
                         osgAnimation::MorphGeometry,
                         "osg::Object osg::Drawable osg::Geometry osgAnimation::MorphGeometry" )
{
    BEGIN_ENUM_SERIALIZER( Method, NORMALIZED );
        ADD_ENUM_VALUE( NORMALIZED );
        ADD_ENUM_VALUE( RELATIVE );
    END_ENUM_SERIALIZER();  // _method

    ADD_USER_SERIALIZER( MorphTargets );  // _morphTargets
    ADD_BOOL_SERIALIZER( MorphNormals, true );  // _morphNormals
}
