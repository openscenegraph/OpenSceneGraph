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
        osg::ref_ptr<osg::Geometry> target = is.readObjectOfType<osg::Geometry>();
        if ( target ) geom.addMorphTarget( target.get(), weight );
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

#define ADD_ARRAYDATA_FUNCTIONS( ORIGINAL_PROP, PROP ) \
    static bool check##ORIGINAL_PROP( const osgAnimation::MorphGeometry& geom ) \
    { return geom.get##PROP()!=0; } \
    static bool read##ORIGINAL_PROP( osgDB::InputStream& is, osgAnimation::MorphGeometry& geom ) { \
        is >> is.BEGIN_BRACKET; \
        osg::ref_ptr<osg::Array> array = is.readArray(); \
        geom.set##PROP(dynamic_cast<osg::Vec3Array*>(array.get())); \
        is >> is.END_BRACKET; \
        return true; \
    } \
    static bool write##ORIGINAL_PROP( osgDB::OutputStream& os, const osgAnimation::MorphGeometry& geom ) { \
        os << os.BEGIN_BRACKET << std::endl; \
        os.writeArray( geom.get##PROP()); \
        os << os.END_BRACKET << std::endl; \
        return true; \
    }
ADD_ARRAYDATA_FUNCTIONS( VertexData, VertexSource )
ADD_ARRAYDATA_FUNCTIONS( NormalData, NormalSource )

struct FinishedObjectReadFillSourceIfRequiredCallback : public osgDB::FinishedObjectReadCallback
{
    virtual void objectRead(osgDB::InputStream&, osg::Object& obj)
    {

        osgAnimation::MorphGeometry& geometry = static_cast<osgAnimation::MorphGeometry&>(obj);
        if((!geometry.getVertexSource() ||geometry.getVertexSource()->getNumElements()==0)
           && dynamic_cast<osg::Vec3Array* >(geometry.getVertexArray())){
            geometry.setVertexSource((osg::Vec3Array* )geometry.getVertexArray()->clone(osg::CopyOp::DEEP_COPY_ALL));
        }
        if((!geometry.getNormalSource() ||geometry.getNormalSource()->getNumElements()==0)
           && geometry.getNormalArray()){
            geometry.setNormalSource((osg::Vec3Array* )geometry.getNormalArray()->clone(osg::CopyOp::DEEP_COPY_ALL));
        }
}

};

REGISTER_OBJECT_WRAPPER( osgAnimation_MorphGeometry,
                         new osgAnimation::MorphGeometry,
                         osgAnimation::MorphGeometry,
                         "osg::Object osg::Node osg::Drawable osg::Geometry osgAnimation::MorphGeometry" )
{
    BEGIN_ENUM_SERIALIZER( Method, NORMALIZED );
        ADD_ENUM_VALUE( NORMALIZED );
        ADD_ENUM_VALUE( RELATIVE );
    END_ENUM_SERIALIZER();  // _method

    ADD_USER_SERIALIZER( MorphTargets );  // _morphTargets
    ADD_BOOL_SERIALIZER( MorphNormals, true );  // _morphNormals
    ADD_USER_SERIALIZER( VertexData );  // VertexSource
    ADD_USER_SERIALIZER( NormalData );  // NormalSource


    {
            UPDATE_TO_VERSION_SCOPED( 147 )
            ADD_OBJECT_SERIALIZER( MorphTransformImplementation, osgAnimation::MorphTransform, NULL );  // _geometry
    }

    wrapper->addFinishedObjectReadCallback( new FinishedObjectReadFillSourceIfRequiredCallback() );
}
