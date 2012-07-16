#include <osg/Uniform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkElements( const osg::Uniform& uniform )
{
    return uniform.getNumElements()>0;
}

static bool readElements( osgDB::InputStream& is, osg::Uniform& uniform )
{
    bool hasArray; is >> hasArray;
    if ( hasArray )
    {
        osg::Array* array = is.readArray();
        switch ( array->getType() )
        {
        case osg::Array::FloatArrayType:
            uniform.setArray( static_cast<osg::FloatArray*>(array) ); break;
        case osg::Array::DoubleArrayType:
            uniform.setArray( static_cast<osg::DoubleArray*>(array) ); break;
        case osg::Array::IntArrayType:
            uniform.setArray( static_cast<osg::IntArray*>(array) ); break;
        case osg::Array::UIntArrayType:
            uniform.setArray( static_cast<osg::UIntArray*>(array) ); break;
        default: break;
        }
    }
    return true;
}

static bool writeElements( osgDB::OutputStream& os, const osg::Uniform& uniform )
{
    if ( uniform.getFloatArray()!=NULL )
    {
        os << (uniform.getFloatArray()!=NULL);
        os.writeArray( uniform.getFloatArray() );
    }
    else if ( uniform.getDoubleArray()!=NULL )
    {
        os << (uniform.getDoubleArray()!=NULL);
        os.writeArray( uniform.getDoubleArray() );
    }
    else if ( uniform.getIntArray()!=NULL )
    {
        os << (uniform.getIntArray()!=NULL);
        os.writeArray( uniform.getIntArray() );
    }
    else
    {
        os << (uniform.getUIntArray()!=NULL);
        os.writeArray( uniform.getUIntArray() );
    }
    return true;
}

REGISTER_OBJECT_WRAPPER( Uniform,
                         new osg::Uniform,
                         osg::Uniform,
                         "osg::Object osg::Uniform" )
{
    BEGIN_ENUM_SERIALIZER3( Type, UNDEFINED );
        ADD_ENUM_VALUE( FLOAT );
        ADD_ENUM_VALUE( FLOAT_VEC2 );
        ADD_ENUM_VALUE( FLOAT_VEC3 );
        ADD_ENUM_VALUE( FLOAT_VEC4 );

        ADD_ENUM_VALUE( DOUBLE );
        ADD_ENUM_VALUE( DOUBLE_VEC2 );
        ADD_ENUM_VALUE( DOUBLE_VEC3 );
        ADD_ENUM_VALUE( DOUBLE_VEC4 );

        ADD_ENUM_VALUE( INT );
        ADD_ENUM_VALUE( INT_VEC2 );
        ADD_ENUM_VALUE( INT_VEC3 );
        ADD_ENUM_VALUE( INT_VEC4 );

        ADD_ENUM_VALUE( UNSIGNED_INT );
        ADD_ENUM_VALUE( UNSIGNED_INT_VEC2 );
        ADD_ENUM_VALUE( UNSIGNED_INT_VEC3 );
        ADD_ENUM_VALUE( UNSIGNED_INT_VEC4 );

        ADD_ENUM_VALUE( BOOL );
        ADD_ENUM_VALUE( BOOL_VEC2 );
        ADD_ENUM_VALUE( BOOL_VEC3 );
        ADD_ENUM_VALUE( BOOL_VEC4 );

        ADD_ENUM_VALUE( FLOAT_MAT2 );
        ADD_ENUM_VALUE( FLOAT_MAT3 );
        ADD_ENUM_VALUE( FLOAT_MAT4 );
        ADD_ENUM_VALUE( FLOAT_MAT2x3 );
        ADD_ENUM_VALUE( FLOAT_MAT2x4 );
        ADD_ENUM_VALUE( FLOAT_MAT3x2 );
        ADD_ENUM_VALUE( FLOAT_MAT3x4 );
        ADD_ENUM_VALUE( FLOAT_MAT4x2 );
        ADD_ENUM_VALUE( FLOAT_MAT4x3 );

        ADD_ENUM_VALUE( DOUBLE_MAT2 );
        ADD_ENUM_VALUE( DOUBLE_MAT3 );
        ADD_ENUM_VALUE( DOUBLE_MAT4 );
        ADD_ENUM_VALUE( DOUBLE_MAT2x3 );
        ADD_ENUM_VALUE( DOUBLE_MAT2x4 );
        ADD_ENUM_VALUE( DOUBLE_MAT3x2 );
        ADD_ENUM_VALUE( DOUBLE_MAT3x4 );
        ADD_ENUM_VALUE( DOUBLE_MAT4x2 );
        ADD_ENUM_VALUE( DOUBLE_MAT4x3 );

        ADD_ENUM_VALUE( SAMPLER_1D );
        ADD_ENUM_VALUE( SAMPLER_2D );
        ADD_ENUM_VALUE( SAMPLER_3D );
        ADD_ENUM_VALUE( SAMPLER_CUBE );
        ADD_ENUM_VALUE( SAMPLER_1D_SHADOW );
        ADD_ENUM_VALUE( SAMPLER_2D_SHADOW );
        ADD_ENUM_VALUE( SAMPLER_1D_ARRAY );
        ADD_ENUM_VALUE( SAMPLER_2D_ARRAY );
        ADD_ENUM_VALUE( SAMPLER_CUBE_MAP_ARRAY );
        ADD_ENUM_VALUE( SAMPLER_1D_ARRAY_SHADOW );
        ADD_ENUM_VALUE( SAMPLER_2D_ARRAY_SHADOW );
        ADD_ENUM_VALUE( SAMPLER_2D_MULTISAMPLE );
        ADD_ENUM_VALUE( SAMPLER_2D_MULTISAMPLE_ARRAY );
        ADD_ENUM_VALUE( SAMPLER_CUBE_SHADOW );
        ADD_ENUM_VALUE( SAMPLER_CUBE_MAP_ARRAY_SHADOW );
        ADD_ENUM_VALUE( SAMPLER_BUFFER );
        ADD_ENUM_VALUE( SAMPLER_2D_RECT );
        ADD_ENUM_VALUE( SAMPLER_2D_RECT_SHADOW );

        ADD_ENUM_VALUE( INT_SAMPLER_1D );
        ADD_ENUM_VALUE( INT_SAMPLER_2D );
        ADD_ENUM_VALUE( INT_SAMPLER_3D );
        ADD_ENUM_VALUE( INT_SAMPLER_CUBE );
        ADD_ENUM_VALUE( INT_SAMPLER_1D_ARRAY );
        ADD_ENUM_VALUE( INT_SAMPLER_2D_ARRAY );
        ADD_ENUM_VALUE( INT_SAMPLER_CUBE_MAP_ARRAY );
        ADD_ENUM_VALUE( INT_SAMPLER_2D_MULTISAMPLE );
        ADD_ENUM_VALUE( INT_SAMPLER_2D_MULTISAMPLE_ARRAY );
        ADD_ENUM_VALUE( INT_SAMPLER_BUFFER );
        ADD_ENUM_VALUE( INT_SAMPLER_2D_RECT );

        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_1D );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_2D );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_3D );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_CUBE );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_1D_ARRAY );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_2D_ARRAY );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_BUFFER );
        ADD_ENUM_VALUE( UNSIGNED_INT_SAMPLER_2D_RECT );

        ADD_ENUM_VALUE( IMAGE_1D );
        ADD_ENUM_VALUE( IMAGE_2D );
        ADD_ENUM_VALUE( IMAGE_3D );
        ADD_ENUM_VALUE( IMAGE_2D_RECT );
        ADD_ENUM_VALUE( IMAGE_CUBE );
        ADD_ENUM_VALUE( IMAGE_BUFFER );
        ADD_ENUM_VALUE( IMAGE_1D_ARRAY );
        ADD_ENUM_VALUE( IMAGE_2D_ARRAY );
        ADD_ENUM_VALUE( IMAGE_CUBE_MAP_ARRAY );
        ADD_ENUM_VALUE( IMAGE_2D_MULTISAMPLE );
        ADD_ENUM_VALUE( IMAGE_2D_MULTISAMPLE_ARRAY );

        ADD_ENUM_VALUE( INT_IMAGE_1D );
        ADD_ENUM_VALUE( INT_IMAGE_2D );
        ADD_ENUM_VALUE( INT_IMAGE_3D );
        ADD_ENUM_VALUE( INT_IMAGE_2D_RECT );
        ADD_ENUM_VALUE( INT_IMAGE_CUBE );
        ADD_ENUM_VALUE( INT_IMAGE_BUFFER );
        ADD_ENUM_VALUE( INT_IMAGE_1D_ARRAY );
        ADD_ENUM_VALUE( INT_IMAGE_2D_ARRAY );
        ADD_ENUM_VALUE( INT_IMAGE_CUBE_MAP_ARRAY );
        ADD_ENUM_VALUE( INT_IMAGE_2D_MULTISAMPLE );
        ADD_ENUM_VALUE( INT_IMAGE_2D_MULTISAMPLE_ARRAY );

        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_1D );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_2D );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_3D );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_2D_RECT );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_CUBE );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_BUFFER );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_1D_ARRAY );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_2D_ARRAY );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_2D_MULTISAMPLE );
        ADD_ENUM_VALUE( UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY );

        ADD_ENUM_VALUE( UNDEFINED );
    END_ENUM_SERIALIZER();  // _type

    ADD_UINT_SERIALIZER( NumElements, 0 );  // _numElements
    ADD_USER_SERIALIZER( Elements );  // _floatArray, _doubleArray, _intArray, _uintArray
    ADD_OBJECT_SERIALIZER( UpdateCallback, osg::Uniform::Callback, NULL );  // _updateCallback
    ADD_OBJECT_SERIALIZER( EventCallback, osg::Uniform::Callback, NULL );  // _eventCallback
}
