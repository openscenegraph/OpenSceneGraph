#include <osg/Array>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace ArrayWrappers {

#if 0
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
#endif

REGISTER_OBJECT_WRAPPER( Array,
                         0,
                         osg::Array,
                         "osg::Object osg::Array" )
{
#if 0
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

    ADD_METHOD_OBJECT( "resizeArray", ResizeArray );
#endif
    BEGIN_ENUM_SERIALIZER( Binding, BIND_UNDEFINED );
        ADD_ENUM_VALUE( BIND_UNDEFINED );
        ADD_ENUM_VALUE( BIND_OFF );
        ADD_ENUM_VALUE( BIND_OVERALL );
        ADD_ENUM_VALUE( BIND_PER_PRIMITIVE_SET );
        ADD_ENUM_VALUE( BIND_PER_VERTEX );
    END_ENUM_SERIALIZER();

    ADD_BOOL_SERIALIZER(Normalize, false);
    ADD_BOOL_SERIALIZER(PreserveDataType, false);

}

}

#define ARRAY_WRAPPERS( ARRAY, ELEMENTTYPE, NUMELEMENTSONROW ) \
    namespace Wrappers##ARRAY { \
        REGISTER_OBJECT_WRAPPER( ARRAY, new osg::ARRAY, osg::ARRAY, "osg::Object osg::Array osg::"#ARRAY) \
        { \
                ADD_ISAVECTOR_SERIALIZER( vector, osgDB::BaseSerializer::ELEMENTTYPE, NUMELEMENTSONROW ); \
        } \
    }

ARRAY_WRAPPERS(FloatArray, RW_FLOAT, 4)
ARRAY_WRAPPERS(Vec2Array, RW_VEC2F, 1)
ARRAY_WRAPPERS(Vec3Array, RW_VEC3F, 1)
ARRAY_WRAPPERS(Vec4Array, RW_VEC4F, 1)

ARRAY_WRAPPERS(DoubleArray, RW_DOUBLE, 4)
ARRAY_WRAPPERS(Vec2dArray, RW_VEC2D, 1)
ARRAY_WRAPPERS(Vec3dArray, RW_VEC3D, 1)
ARRAY_WRAPPERS(Vec4dArray, RW_VEC4D, 1)

ARRAY_WRAPPERS(ByteArray, RW_CHAR, 4)
ARRAY_WRAPPERS(Vec2bArray, RW_VEC2B, 1)
ARRAY_WRAPPERS(Vec3bArray, RW_VEC3B, 1)
ARRAY_WRAPPERS(Vec4bArray, RW_VEC4B, 1)

ARRAY_WRAPPERS(UByteArray, RW_UCHAR, 4)
ARRAY_WRAPPERS(Vec2ubArray, RW_VEC2UB, 1)
ARRAY_WRAPPERS(Vec3ubArray, RW_VEC3UB, 1)
ARRAY_WRAPPERS(Vec4ubArray, RW_VEC4UB, 1)

ARRAY_WRAPPERS(ShortArray, RW_SHORT, 4)
ARRAY_WRAPPERS(Vec2sArray, RW_VEC2S, 1)
ARRAY_WRAPPERS(Vec3sArray, RW_VEC3S, 1)
ARRAY_WRAPPERS(Vec4sArray, RW_VEC4S, 1)

ARRAY_WRAPPERS(UShortArray, RW_USHORT, 4)
ARRAY_WRAPPERS(Vec2usArray, RW_VEC2US, 1)
ARRAY_WRAPPERS(Vec3usArray, RW_VEC3US, 1)
ARRAY_WRAPPERS(Vec4usArray, RW_VEC4US, 1)

ARRAY_WRAPPERS(IntArray, RW_INT, 4)
ARRAY_WRAPPERS(Vec2iArray, RW_VEC2I, 1)
ARRAY_WRAPPERS(Vec3iArray, RW_VEC3I, 1)
ARRAY_WRAPPERS(Vec4iArray, RW_VEC4I, 1)

ARRAY_WRAPPERS(UIntArray, RW_UINT, 4)
ARRAY_WRAPPERS(Vec2uiArray, RW_VEC2UI, 1)
ARRAY_WRAPPERS(Vec3uiArray, RW_VEC3UI, 1)
ARRAY_WRAPPERS(Vec4uiArray, RW_VEC4UI, 1)
