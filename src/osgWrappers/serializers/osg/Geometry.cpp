#include <osg/Geometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

BEGIN_USER_TABLE( AttributeBinding, osg::Geometry );
    ADD_USER_VALUE( BIND_OFF );
    ADD_USER_VALUE( BIND_OVERALL );
    ADD_USER_VALUE( BIND_PER_PRIMITIVE_SET );
    ADD_USER_VALUE( BIND_PER_PRIMITIVE );
    ADD_USER_VALUE( BIND_PER_VERTEX );
END_USER_TABLE()

USER_READ_FUNC( AttributeBinding, readAttributeBinding )
USER_WRITE_FUNC( AttributeBinding, writeAttributeBinding )

static void readArrayData( osgDB::InputStream& is, osg::Geometry::ArrayData& data )
{
    bool hasArray = false;
    is >> is.PROPERTY("Array") >> hasArray;
    if ( hasArray ) data.array = is.readArray();

    bool hasIndices = false;
    is >> is.PROPERTY("Indices") >> hasIndices;
    if ( hasIndices ) data.indices = dynamic_cast<osg::IndexArray*>( is.readArray() );

    is >> is.PROPERTY("Binding");
    data.binding = (osg::Geometry::AttributeBinding)readAttributeBinding(is);

    int normalizeValue = 0;
    is >> is.PROPERTY("Normalize") >> normalizeValue;
    data.normalize = normalizeValue;
}

static void writeArrayData( osgDB::OutputStream& os, const osg::Geometry::ArrayData& data )
{
    os << os.PROPERTY("Array") << data.array.valid();
    if ( data.array.valid() ) os << data.array.get();
    else os << std::endl;

    os << os.PROPERTY("Indices") << data.indices.valid();
    if ( data.indices.valid() ) os << data.indices.get();
    else os << std::endl;

    os << os.PROPERTY("Binding"); writeAttributeBinding(os, data.binding); os << std::endl;
    os << os.PROPERTY("Normalize") << (int)data.normalize << std::endl;
}

#define ADD_ARRAYDATA_FUNCTIONS( PROP ) \
    static bool check##PROP( const osg::Geometry& geom ) \
    { return geom.get##PROP().array.valid(); } \
    static bool read##PROP( osgDB::InputStream& is, osg::Geometry& geom ) { \
        osg::Geometry::ArrayData data; \
        is >> is.BEGIN_BRACKET; readArrayData(is, data); \
        is >> is.END_BRACKET; \
        geom.set##PROP(data); \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::Geometry& geom ) { \
        os << os.BEGIN_BRACKET << std::endl; \
        writeArrayData(os, geom.get##PROP()); \
        os << os.END_BRACKET << std::endl; \
        return true; \
    }

ADD_ARRAYDATA_FUNCTIONS( VertexData )
ADD_ARRAYDATA_FUNCTIONS( NormalData )
ADD_ARRAYDATA_FUNCTIONS( ColorData )
ADD_ARRAYDATA_FUNCTIONS( SecondaryColorData )
ADD_ARRAYDATA_FUNCTIONS( FogCoordData )

#define ADD_ARRAYLIST_FUNCTIONS( PROP, LISTNAME ) \
    static bool check##PROP( const osg::Geometry& geom ) \
    { return geom.get##LISTNAME().size()>0; } \
    static bool read##PROP( osgDB::InputStream& is, osg::Geometry& geom ) { \
        unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET; \
        for ( unsigned int i=0; i<size; ++i ) { \
            osg::Geometry::ArrayData data; \
            is >> is.PROPERTY("Data") >> is.BEGIN_BRACKET; \
            readArrayData(is, data); \
            is >> is.END_BRACKET; geom.set##PROP(i, data); } \
        is >> is.END_BRACKET; \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::Geometry& geom ) { \
        const osg::Geometry::ArrayDataList& LISTNAME = geom.get##LISTNAME(); \
        os.writeSize(LISTNAME.size()); os << os.BEGIN_BRACKET << std::endl; \
        for ( osg::Geometry::ArrayDataList::const_iterator itr=LISTNAME.begin(); \
              itr!=LISTNAME.end(); ++itr ) { \
            os << os.PROPERTY("Data") << os.BEGIN_BRACKET << std::endl; \
            writeArrayData(os, *itr); os << os.END_BRACKET << std::endl; \
        } \
        os << os.END_BRACKET << std::endl; \
        return true; \
    }

ADD_ARRAYLIST_FUNCTIONS( TexCoordData, TexCoordArrayList )
ADD_ARRAYLIST_FUNCTIONS( VertexAttribData, VertexAttribArrayList )

struct GeometryFinishedObjectReadCallback : public osgDB::FinishedObjectReadCallback
{
    virtual void objectRead(osgDB::InputStream&, osg::Object& obj)
    {
        osg::Geometry& geometry = static_cast<osg::Geometry&>(obj);
        if (geometry.getUseVertexBufferObjects())
        {
            geometry.setUseVertexBufferObjects(false);
            geometry.setUseVertexBufferObjects(true);
        }
    }
};

REGISTER_OBJECT_WRAPPER( Geometry,
                         new osg::Geometry,
                         osg::Geometry,
                         "osg::Object osg::Drawable osg::Geometry" )
{
    ADD_LIST_SERIALIZER( PrimitiveSetList, osg::Geometry::PrimitiveSetList );  // _primitives
    ADD_USER_SERIALIZER( VertexData );  // _vertexData
    ADD_USER_SERIALIZER( NormalData );  // _normalData
    ADD_USER_SERIALIZER( ColorData );  // _colorData
    ADD_USER_SERIALIZER( SecondaryColorData );  // _secondaryColorData
    ADD_USER_SERIALIZER( FogCoordData );  // _fogCoordData
    ADD_USER_SERIALIZER( TexCoordData );  // _texCoordList
    ADD_USER_SERIALIZER( VertexAttribData );  // _vertexAttribList
    ADD_BOOL_SERIALIZER( FastPathHint, true );  // _fastPathHint
    //ADD_OBJECT_SERIALIZER( InternalOptimizedGeometry, osg::Geometry, NULL );  // _internalOptimizedGeometry

    wrapper->addFinishedObjectReadCallback( new GeometryFinishedObjectReadCallback() );
}
