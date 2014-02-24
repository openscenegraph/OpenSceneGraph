#include <osg/Geometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static void add_user_value_func_AttributeBinding(osgDB::IntLookup* lookup)
{
    lookup->add("BIND_OFF",0);                 // ADD_USER_VALUE("ADD_USER_VALUE( BIND_OFF );
    lookup->add("BIND_OVERALL",1);             // ADD_USER_VALUE( BIND_OVERALL );
    lookup->add("BIND_PER_PRIMITIVE_SET",2);   // ADD_USER_VALUE( BIND_PER_PRIMITIVE_SET );
    lookup->add("BIND_PER_PRIMITIVE",3);       //ADD_USER_VALUE( BIND_PER_PRIMITIVE );
    lookup->add("BIND_PER_VERTEX",4);          // ADD_USER_VALUE( BIND_PER_VERTEX );
 }
static osgDB::UserLookupTableProxy s_user_lookup_table_AttributeBinding(&add_user_value_func_AttributeBinding);

USER_READ_FUNC( AttributeBinding, readAttributeBinding )
USER_WRITE_FUNC( AttributeBinding, writeAttributeBinding )

static osg::Array* readArray( osgDB::InputStream& is)
{
    osg::ref_ptr<osg::Array> array;
    bool hasArray = false;
    is >> is.PROPERTY("Array") >> hasArray;
    if ( hasArray ) array = is.readArray();

    bool hasIndices = false;
    is >> is.PROPERTY("Indices") >> hasIndices;
    if ( hasIndices )
    {
        osg::ref_ptr<osg::Array> indices_array = is.readArray();
        osg::ref_ptr<osg::IndexArray> indices = dynamic_cast<osg::IndexArray*>( indices_array.get() );
        if (array.valid() && indices.valid()) array->setUserData(indices.get());
    }

    is >> is.PROPERTY("Binding");
    int binding = readAttributeBinding(is);
    if (array.valid()) array->setBinding(static_cast<osg::Array::Binding>(binding));

    int normalizeValue = 0;
    is >> is.PROPERTY("Normalize") >> normalizeValue;
    if (array.valid()) array->setNormalize(normalizeValue!=0);

    return array.release();
}

static void writeArray( osgDB::OutputStream& os, const osg::Array* array)
{
    os << os.PROPERTY("Array") << (array!=0);
    if ( array!=0 ) os << array;
    else os << std::endl;

    const osg::IndexArray* indices = (array!=0) ? dynamic_cast<const osg::IndexArray*>(array->getUserData()) : 0;
    os << os.PROPERTY("Indices") << (indices!=0);
    if ( indices!=0 ) os << indices;
    else os << std::endl;

    os << os.PROPERTY("Binding"); writeAttributeBinding(os, osg::getBinding(array)); os << std::endl;
    os << os.PROPERTY("Normalize") << ((array!=0 && array->getNormalize()) ? 1:0) << std::endl;
}

#define ADD_ARRAYDATA_FUNCTIONS( ORIGINAL_PROP, PROP ) \
    static bool check##ORIGINAL_PROP( const osg::Geometry& geom ) \
    { return geom.get##PROP()!=0; } \
    static bool read##ORIGINAL_PROP( osgDB::InputStream& is, osg::Geometry& geom ) { \
        is >> is.BEGIN_BRACKET; \
        osg::Array* array = readArray(is); \
        geom.set##PROP(array); \
        is >> is.END_BRACKET; \
        return true; \
    } \
    static bool write##ORIGINAL_PROP( osgDB::OutputStream& os, const osg::Geometry& geom ) { \
        os << os.BEGIN_BRACKET << std::endl; \
        writeArray(os, geom.get##PROP()); \
        os << os.END_BRACKET << std::endl; \
        return true; \
    }

ADD_ARRAYDATA_FUNCTIONS( VertexData, VertexArray )
ADD_ARRAYDATA_FUNCTIONS( NormalData, NormalArray )
ADD_ARRAYDATA_FUNCTIONS( ColorData, ColorArray )
ADD_ARRAYDATA_FUNCTIONS( SecondaryColorData, SecondaryColorArray )
ADD_ARRAYDATA_FUNCTIONS( FogCoordData, FogCoordArray )

#define ADD_ARRAYLIST_FUNCTIONS( ORIGINAL_PROP, PROP, LISTNAME ) \
    static bool check##ORIGINAL_PROP( const osg::Geometry& geom ) \
    { return geom.get##LISTNAME().size()>0; } \
    static bool read##ORIGINAL_PROP( osgDB::InputStream& is, osg::Geometry& geom ) { \
        unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET; \
        for ( unsigned int i=0; i<size; ++i ) { \
            is >> is.PROPERTY("Data") >> is.BEGIN_BRACKET; \
            osg::Array* array = readArray(is); \
            geom.set##PROP(i, array); \
            is >> is.END_BRACKET; } \
        is >> is.END_BRACKET; \
        return true; \
    } \
    static bool write##ORIGINAL_PROP( osgDB::OutputStream& os, const osg::Geometry& geom ) { \
        const osg::Geometry::ArrayList& LISTNAME = geom.get##LISTNAME(); \
        os.writeSize(LISTNAME.size()); os << os.BEGIN_BRACKET << std::endl; \
        for ( osg::Geometry::ArrayList::const_iterator itr=LISTNAME.begin(); \
              itr!=LISTNAME.end(); ++itr ) { \
            os << os.PROPERTY("Data") << os.BEGIN_BRACKET << std::endl; \
            writeArray(os, itr->get()); os << os.END_BRACKET << std::endl; \
        } \
        os << os.END_BRACKET << std::endl; \
        return true; \
    }

ADD_ARRAYLIST_FUNCTIONS( TexCoordData, TexCoordArray, TexCoordArrayList )
ADD_ARRAYLIST_FUNCTIONS( VertexAttribData, VertexAttribArray, VertexAttribArrayList )

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

// implement backwards compatibility with reading/writing the FastPathHint
static bool checkFastPathHint( const osg::Geometry& geom ) { return false; }
static bool readFastPathHint( osgDB::InputStream& is, osg::Geometry& geom )
{
    // Compatibility info:
    //   Previous Geometry wrapper (before 3.1.8) require a bool fast-path serializer.
    //   It saves "FastPathHint true" in ascii mode and a single [bool] in binary mode.
    //   Becoming a user serializer, the serializer will first read the name "FastPathHint"
    //   or a [bool] in the checking process, then call the reading function as here. So,
    //   we will only need to read one more bool variable in ascii mode; otherwise do nothing
    bool value = false;
    if ( !is.isBinary() ) is >> value;
    return true;
}
static bool writeFastPathHint( osgDB::OutputStream& os, const osg::Geometry& geom )
{
    return true;
}

REGISTER_OBJECT_WRAPPER( Geometry,
                         new osg::Geometry,
                         osg::Geometry,
                         "osg::Object osg::Drawable osg::Geometry" )
{
    //ADD_LIST_SERIALIZER( PrimitiveSetList, osg::Geometry::PrimitiveSetList );  // _primitives
    ADD_VECTOR_SERIALIZER( PrimitiveSetList, osg::Geometry::PrimitiveSetList, osgDB::BaseSerializer::RW_OBJECT, 0 );

    ADD_USER_SERIALIZER( VertexData );  // _vertexData
    ADD_USER_SERIALIZER( NormalData );  // _normalData
    ADD_USER_SERIALIZER( ColorData );  // _colorData
    ADD_USER_SERIALIZER( SecondaryColorData );  // _secondaryColorData
    ADD_USER_SERIALIZER( FogCoordData );  // _fogCoordData
    ADD_USER_SERIALIZER( TexCoordData );  // _texCoordList
    ADD_USER_SERIALIZER( VertexAttribData );  // _vertexAttribList

    ADD_USER_SERIALIZER( FastPathHint );  // _fastPathHint

    {
        UPDATE_TO_VERSION_SCOPED( 112 )
        REMOVE_SERIALIZER( VertexData );
        REMOVE_SERIALIZER( NormalData );
        REMOVE_SERIALIZER( ColorData );
        REMOVE_SERIALIZER( SecondaryColorData );
        REMOVE_SERIALIZER( FogCoordData );
        REMOVE_SERIALIZER( TexCoordData );
        REMOVE_SERIALIZER( VertexAttribData );
        REMOVE_SERIALIZER( FastPathHint );

        ADD_OBJECT_SERIALIZER( VertexArray, osg::Array, NULL );
        ADD_OBJECT_SERIALIZER( NormalArray, osg::Array, NULL );
        ADD_OBJECT_SERIALIZER( ColorArray, osg::Array, NULL );
        ADD_OBJECT_SERIALIZER( SecondaryColorArray, osg::Array, NULL );
        ADD_OBJECT_SERIALIZER( FogCoordArray, osg::Array, NULL );

        ADD_VECTOR_SERIALIZER( TexCoordArrayList, osg::Geometry::ArrayList, osgDB::BaseSerializer::RW_OBJECT, 0 );
        ADD_VECTOR_SERIALIZER( VertexAttribArrayList, osg::Geometry::ArrayList, osgDB::BaseSerializer::RW_OBJECT, 0 );
    }


    wrapper->addFinishedObjectReadCallback( new GeometryFinishedObjectReadCallback() );
}
