#include <osg/Shape>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define ARRAY_FUNCTIONS( PROP, TYPE ) \
    static bool check##PROP(const osg::TriangleMesh& shape) { \
        return shape.get##PROP()!=NULL; \
    } \
    static bool read##PROP(osgDB::InputStream& is, osg::TriangleMesh& shape) { \
        osg::ref_ptr<osg::Array> array = is.readArray(); \
        shape.set##PROP( dynamic_cast<TYPE*>(array.get()) ); \
        return true; \
    } \
    static bool write##PROP(osgDB::OutputStream& os, const osg::TriangleMesh& shape) { \
        os << shape.get##PROP(); \
        return true; \
    }

ARRAY_FUNCTIONS( Vertices, osg::Vec3Array )
ARRAY_FUNCTIONS( Indices, osg::IndexArray )

REGISTER_OBJECT_WRAPPER( TriangleMesh,
                         new osg::TriangleMesh,
                         osg::TriangleMesh,
                         "osg::Object osg::Shape osg::TriangleMesh" )
{
    ADD_USER_SERIALIZER( Vertices );  // _vertices
    ADD_USER_SERIALIZER( Indices );  // _indices
}
