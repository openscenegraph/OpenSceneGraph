#include <osg/Drawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkInitialBound( const osg::Drawable& drawable )
{
    return drawable.getInitialBound().valid();
}

static bool readInitialBound( osgDB::InputStream& is, osg::Drawable& drawable )
{
    osg::Vec3d min, max;
    is >> is.BEGIN_BRACKET;
    is >> is.PROPERTY("Minimum") >> min;
    is >> is.PROPERTY("Maximum") >> max;
    is >> is.END_BRACKET;
    drawable.setInitialBound( osg::BoundingBox(min, max) );
    return true;
}

static bool writeInitialBound( osgDB::OutputStream& os, const osg::Drawable& drawable )
{
    const osg::BoundingBox& bb = drawable.getInitialBound();
    os << os.BEGIN_BRACKET << std::endl;
    os << os.PROPERTY("Minimum") << osg::Vec3d(bb._min) << std::endl;
    os << os.PROPERTY("Maximum") << osg::Vec3d(bb._max) << std::endl;
    os << os.END_BRACKET;
    os << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Drawable,
                         /*new osg::Drawable*/NULL,
                         osg::Drawable,
                         "osg::Object osg::Drawable" )
{
    ADD_OBJECT_SERIALIZER( StateSet, osg::StateSet, NULL );  // _stateset
    ADD_USER_SERIALIZER( InitialBound );  // _initialBound
    ADD_OBJECT_SERIALIZER( ComputeBoundingBoxCallback,
                           osg::Drawable::ComputeBoundingBoxCallback, NULL );  // _computeBoundCallback
    ADD_OBJECT_SERIALIZER( Shape, osg::Shape, NULL );  // _shape
    ADD_BOOL_SERIALIZER( SupportsDisplayList, true );  // _supportsDisplayList
    ADD_BOOL_SERIALIZER( UseDisplayList, true );  // _useDisplayList
    ADD_BOOL_SERIALIZER( UseVertexBufferObjects, false );  // _useVertexBufferObjects
    ADD_OBJECT_SERIALIZER( UpdateCallback, osg::Drawable::UpdateCallback, NULL );  // _updateCallback
    ADD_OBJECT_SERIALIZER( EventCallback, osg::Drawable::EventCallback, NULL );  // _eventCallback
    ADD_OBJECT_SERIALIZER( CullCallback, osg::Drawable::CullCallback, NULL );  // _cullCallback
    ADD_OBJECT_SERIALIZER( DrawCallback, osg::Drawable::DrawCallback, NULL );  // _drawCallback
}
