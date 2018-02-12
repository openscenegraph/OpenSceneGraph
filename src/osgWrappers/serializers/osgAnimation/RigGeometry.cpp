#include <osgAnimation/RigGeometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace wrap_osgAnimationRigGeometry{
static bool checkInfluenceMap( const osgAnimation::RigGeometry& geom )
{
    return geom.getInfluenceMap()->size()>0;
}

static bool readInfluenceMap( osgDB::InputStream& is, osgAnimation::RigGeometry& geom )
{
    osg::ref_ptr<osgAnimation::VertexInfluenceMap> map = new osgAnimation::VertexInfluenceMap;
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string bonename;
        unsigned int viSize = 0;
        is >> is.PROPERTY("VertexInfluence");
        is.readWrappedString(bonename);
        viSize = is.readSize(); is >> is.BEGIN_BRACKET;
        osgAnimation::VertexInfluence vi;
        vi.setName( bonename );
        vi.reserve( viSize );
        for ( unsigned int j=0; j<viSize; ++j )
        {
            int index = 0;
            float weight = 0.0f;
            is >> index >> weight;
            vi.push_back( osgAnimation::VertexIndexWeight(index, weight) );
        }
        (*map)[bonename] = vi;
        is >> is.END_BRACKET;
    }
    is >> is.END_BRACKET;

    if ( !map->empty() ) geom.setInfluenceMap( map.get() );
    return true;
}

static bool writeInfluenceMap( osgDB::OutputStream& os, const osgAnimation::RigGeometry& geom )
{
    const osgAnimation::VertexInfluenceMap* map = geom.getInfluenceMap();
    os.writeSize(map->size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osgAnimation::VertexInfluenceMap::const_iterator itr=map->begin();
          itr!=map->end(); ++itr )
    {
        std::string name = itr->first;
        const osgAnimation::VertexInfluence& vi = itr->second;
        if ( name.empty() ) name = "Empty";

        os << os.PROPERTY("VertexInfluence");
        os.writeWrappedString(name);
        os.writeSize(vi.size()) ; os << os.BEGIN_BRACKET << std::endl;

        for ( osgAnimation::VertexInfluence::const_iterator vitr=vi.begin();
              vitr != vi.end(); ++vitr )
        {
            os << vitr->first << vitr->second << std::endl;
        }
        os << os.END_BRACKET << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgAnimation_RigGeometry,
                         new osgAnimation::RigGeometry,
                         osgAnimation::RigGeometry,
                         "osg::Object osg::Node osg::Drawable osg::Geometry osgAnimation::RigGeometry" )
{
    {
         UPDATE_TO_VERSION_SCOPED( 154 )
         ADDED_ASSOCIATE("osg::Node")
    }

    ADD_USER_SERIALIZER( InfluenceMap );  // _vertexInfluenceMap
    ADD_OBJECT_SERIALIZER( SourceGeometry, osg::Geometry, NULL );  // _geometry

    {
        UPDATE_TO_VERSION_SCOPED( 145 )
        ADD_OBJECT_SERIALIZER( RigTransformImplementation, osgAnimation::RigTransform, NULL );  // _geometry
    }

}
}
