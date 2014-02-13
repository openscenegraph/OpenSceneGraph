#include <osg/Array>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace ArrayWrappers {

struct ResizeArray : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Object* indexObject = inputParameters[0].get();

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }
        osg::Array* array = reinterpret_cast<osg::Array*>(objectPtr);
        array->resizeArray(index);

        return true;
    }
};


REGISTER_OBJECT_WRAPPER( Array,
                         0,
                         osg::Array,
                         "osg::Object osg::Array" )
{

    BEGIN_ENUM_SERIALIZER_NO_SET( Type, ArrayType );
        ADD_ENUM_VALUE( ArrayType );

        ADD_ENUM_VALUE( ByteArrayType );
        ADD_ENUM_VALUE( ShortArrayType );
        ADD_ENUM_VALUE( IntArrayType );

        ADD_ENUM_VALUE( UByteArrayType );
        ADD_ENUM_VALUE( UShortArrayType );
        ADD_ENUM_VALUE( UIntArrayType );

        ADD_ENUM_VALUE( FloatArrayType );
        ADD_ENUM_VALUE( DoubleArrayType );

        ADD_ENUM_VALUE( Vec2bArrayType );
        ADD_ENUM_VALUE( Vec3bArrayType );
        ADD_ENUM_VALUE( Vec4bArrayType );

        ADD_ENUM_VALUE( Vec2sArrayType );
        ADD_ENUM_VALUE( Vec3sArrayType );
        ADD_ENUM_VALUE( Vec4sArrayType );

        ADD_ENUM_VALUE( Vec2iArrayType );
        ADD_ENUM_VALUE( Vec3iArrayType );
        ADD_ENUM_VALUE( Vec4iArrayType );

        ADD_ENUM_VALUE( Vec2ubArrayType );
        ADD_ENUM_VALUE( Vec3ubArrayType );
        ADD_ENUM_VALUE( Vec4ubArrayType );

        ADD_ENUM_VALUE( Vec2usArrayType );
        ADD_ENUM_VALUE( Vec3usArrayType );
        ADD_ENUM_VALUE( Vec4usArrayType );

        ADD_ENUM_VALUE( Vec2uiArrayType );
        ADD_ENUM_VALUE( Vec3uiArrayType );
        ADD_ENUM_VALUE( Vec4uiArrayType );

        ADD_ENUM_VALUE( Vec2ArrayType );
        ADD_ENUM_VALUE( Vec3ArrayType );
        ADD_ENUM_VALUE( Vec4ArrayType );

        ADD_ENUM_VALUE( Vec2dArrayType );
        ADD_ENUM_VALUE( Vec3dArrayType );
        ADD_ENUM_VALUE( Vec4dArrayType );

        ADD_ENUM_VALUE( MatrixArrayType );
        ADD_ENUM_VALUE( MatrixdArrayType );
    END_ENUM_SERIALIZER();

    ADD_INT_SERIALIZER_NO_SET( DataSize, 0);

    ADD_GLENUM_SERIALIZER_NO_SET( DataType, GLenum, GL_NONE );

    ADD_UINT_SERIALIZER_NO_SET( ElementSize, 0);
    ADD_UINT_SERIALIZER_NO_SET( TotalDataSize, 0);
    ADD_UINT_SERIALIZER_NO_SET( NumElements, 0);

    BEGIN_ENUM_SERIALIZER( Binding, BIND_UNDEFINED );
        ADD_ENUM_VALUE( BIND_UNDEFINED );
        ADD_ENUM_VALUE( BIND_OFF );
        ADD_ENUM_VALUE( BIND_OVERALL );
        ADD_ENUM_VALUE( BIND_PER_PRIMITIVE_SET );
        ADD_ENUM_VALUE( BIND_PER_VERTEX );
    END_ENUM_SERIALIZER();

    ADD_BOOL_SERIALIZER(Normalize, false);
    ADD_BOOL_SERIALIZER(PreserveDataType, false);

    ADD_METHOD_OBJECT( "resizeArray", ResizeArray );
}

}

#define ARRAY_WRAPPERS( ARRAY ) \
    namespace Wrappers##ARRAY { REGISTER_OBJECT_WRAPPER( ARRAY, new osg::ARRAY, osg::ARRAY, "osg::Object osg::Array osg::"#ARRAY ) {} }

ARRAY_WRAPPERS(FloatArray)
ARRAY_WRAPPERS(Vec2Array)
ARRAY_WRAPPERS(Vec3Array)
ARRAY_WRAPPERS(Vec4Array)

ARRAY_WRAPPERS(DoubleArray)
ARRAY_WRAPPERS(Vec2dArray)
ARRAY_WRAPPERS(Vec3dArray)
ARRAY_WRAPPERS(Vec4dArray)

ARRAY_WRAPPERS(ByteArray)
ARRAY_WRAPPERS(Vec2bArray)
ARRAY_WRAPPERS(Vec3bArray)
ARRAY_WRAPPERS(Vec4bArray)

ARRAY_WRAPPERS(UByteArray)
ARRAY_WRAPPERS(Vec2ubArray)
ARRAY_WRAPPERS(Vec3ubArray)
ARRAY_WRAPPERS(Vec4ubArray)

ARRAY_WRAPPERS(ShortArray)
ARRAY_WRAPPERS(Vec2sArray)
ARRAY_WRAPPERS(Vec3sArray)
ARRAY_WRAPPERS(Vec4sArray)

ARRAY_WRAPPERS(UShortArray)
ARRAY_WRAPPERS(Vec2usArray)
ARRAY_WRAPPERS(Vec3usArray)
ARRAY_WRAPPERS(Vec4usArray)

ARRAY_WRAPPERS(IntArray)
ARRAY_WRAPPERS(Vec2iArray)
ARRAY_WRAPPERS(Vec3iArray)
ARRAY_WRAPPERS(Vec4iArray)

ARRAY_WRAPPERS(UIntArray)
ARRAY_WRAPPERS(Vec2uiArray)
ARRAY_WRAPPERS(Vec3uiArray)
ARRAY_WRAPPERS(Vec4uiArray)
