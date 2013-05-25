/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2008 Zebra Imaging
 * Copyright (C) 2012 David Callu
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* file:   src/osg/Uniform.cpp
 * author: Mike Weiblen 2008-01-02
*/
#include <string.h>

#include <osg/Notify>
#include <osg/Uniform>
#include <osg/Program>
#include <osg/StateSet>

#include <limits.h>
#include <algorithm>

using namespace osg;

///////////////////////////////////////////////////////////////////////////
// osg::Uniform
///////////////////////////////////////////////////////////////////////////

Uniform::Uniform() :
    _type(UNDEFINED), _numElements(0), _nameID(UINT_MAX), _modifiedCount(0)
{
}


Uniform::Uniform( Type type, const std::string& name, int numElements ) :
    _type(type), _numElements(0), _nameID(UINT_MAX), _modifiedCount(0)
{
    setName(name);
    setNumElements(numElements);
    allocateDataArray();
}

Uniform::Uniform( const Uniform& rhs, const CopyOp& copyop ) :
    Object(rhs,copyop), _type(rhs._type)
{
    copyData( rhs );
}

Uniform::~Uniform()
{
}

void Uniform::addParent(osg::StateSet* object)
{
    OSG_DEBUG_FP<<"Uniform Adding parent"<<std::endl;

    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    _parents.push_back(object);
}

void Uniform::removeParent(osg::StateSet* object)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),object);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}

bool Uniform::setType( Type t )
{
    if (_type==t) return true;

    if( _type != UNDEFINED )
    {
        OSG_WARN << "cannot change Uniform type" << std::endl;
        return false;
    }
    _type = t;
    allocateDataArray();
    return true;
}

void Uniform::setName( const std::string& name )
{
    if( _name != "" )
    {
        OSG_WARN << "cannot change Uniform name" << std::endl;
        return;
    }
    Object::setName(name);
    _nameID = getNameID(_name);
}

void Uniform::setNumElements( unsigned int numElements )
{
    if( numElements < 1 )
    {
        OSG_WARN << "Uniform numElements < 1 is invalid" << std::endl;
        return;
    }

    if (numElements == _numElements) return;

    if( _numElements>0 )
    {
        OSG_WARN << "Warning: Uniform::setNumElements() cannot change Uniform numElements, size already fixed." << std::endl;
        return;
    }

    _numElements = numElements;
    allocateDataArray();
}

void Uniform::allocateDataArray()
{
    // if one array is already allocated, the job is done.
    if( _floatArray.valid() || _doubleArray.valid() || _intArray.valid() || _uintArray.valid() ) return;

    // array cannot be created until _type and _numElements are specified
    int arrayNumElements = getInternalArrayNumElements();
    if( arrayNumElements )
    {
        switch( getInternalArrayType(getType()) )
        {
            case GL_FLOAT:
                _floatArray = new FloatArray(arrayNumElements);
                return;

            case GL_DOUBLE:
                _doubleArray = new DoubleArray(arrayNumElements);
                return;

            case GL_INT:
                _intArray = new IntArray(arrayNumElements);
                return;

            case GL_UNSIGNED_INT:
                _uintArray = new UIntArray(arrayNumElements);
                return;

            default:
                break;
        }
    }
}

bool Uniform::setArray( FloatArray* array )
{
    if( !array ) return false;

    // incoming array must match configuration of the Uniform
    if( getInternalArrayType(getType())!=GL_FLOAT || getInternalArrayNumElements()!=array->getNumElements() )
    {
        OSG_WARN << "Uniform::setArray : incompatible array" << std::endl;
        return false;
    }

    _floatArray = array;
    _doubleArray = 0;
    _intArray = 0;
    _uintArray = 0;
    dirty();
    return true;
}

bool Uniform::setArray( DoubleArray* array )
{
    if( !array ) return false;

    // incoming array must match configuration of the Uniform
    if( getInternalArrayType(getType())!=GL_DOUBLE || getInternalArrayNumElements()!=array->getNumElements() )
    {
        OSG_WARN << "Uniform::setArray : incompatible array" << std::endl;
        return false;
    }

    _doubleArray = array;
    _floatArray = 0;
    _intArray = 0;
    _uintArray = 0;
    dirty();
    return true;
}

bool Uniform::setArray( IntArray* array )
{
    if( !array ) return false;

    // incoming array must match configuration of the Uniform
    if( getInternalArrayType(getType())!=GL_INT || getInternalArrayNumElements()!=array->getNumElements() )
    {
        OSG_WARN << "Uniform::setArray : incompatible array" << std::endl;
        return false;
    }

    _intArray = array;
    _floatArray = 0;
    _doubleArray = 0;
    _uintArray = 0;
    dirty();
    return true;
}

bool Uniform::setArray( UIntArray* array )
{
    if( !array ) return false;

    // incoming array must match configuration of the Uniform
    if( getInternalArrayType(getType())!=GL_UNSIGNED_INT || getInternalArrayNumElements()!=array->getNumElements() )
    {
        OSG_WARN << "Uniform::setArray : incompatible array" << std::endl;
        return false;
    }

    _uintArray = array;
    _floatArray = 0;
    _doubleArray = 0;
    _intArray = 0;
    dirty();
    return true;
}

///////////////////////////////////////////////////////////////////////////

int Uniform::compare(const Uniform& rhs) const
{
    if( this == &rhs ) return 0;

    if( _type < rhs._type ) return -1;
    if( rhs._type < _type ) return 1;

    if( _numElements < rhs._numElements ) return -1;
    if( rhs._numElements < _numElements ) return 1;

    if( _name < rhs._name ) return -1;
    if( rhs._name < _name ) return 1;

    return compareData( rhs );
}

int Uniform::compareData(const Uniform& rhs) const
{
    // caller must ensure that _type==rhs._type

    if( _floatArray.valid() )
    {
        if( ! rhs._floatArray ) return 1;
        if( _floatArray == rhs._floatArray ) return 0;
        return memcmp( _floatArray->getDataPointer(), rhs._floatArray->getDataPointer(),
            _floatArray->getTotalDataSize() );
    }

    else if( _doubleArray.valid() )
    {
        if( ! rhs._doubleArray ) return 1;
        if( _doubleArray == rhs._doubleArray ) return 0;
        return memcmp( _doubleArray->getDataPointer(), rhs._doubleArray->getDataPointer(),
            _doubleArray->getTotalDataSize() );
    }

    else if( _intArray.valid() )
    {
        if( ! rhs._intArray ) return 1;
        if( _intArray == rhs._intArray ) return 0;
        return memcmp( _intArray->getDataPointer(), rhs._intArray->getDataPointer(),
            _intArray->getTotalDataSize() );
    }

    else if( _uintArray.valid() )
    {
        if( ! rhs._uintArray ) return 1;
        if( _uintArray == rhs._uintArray ) return 0;
        return memcmp( _uintArray->getDataPointer(), rhs._uintArray->getDataPointer(),
            _uintArray->getTotalDataSize() );
    }

    return -1;  // how got here?
}

void Uniform::copyData(const Uniform& rhs)
{
    // caller must ensure that _type==rhs._type
    _numElements = rhs._numElements;
    _nameID = rhs._nameID;
    if (rhs._floatArray.valid() || rhs._doubleArray.valid() || rhs._intArray.valid() || rhs._uintArray.valid()) allocateDataArray();
    if( _floatArray.valid()  && rhs._floatArray.valid() )   *_floatArray  = *rhs._floatArray;
    if( _doubleArray.valid() && rhs._doubleArray.valid() )  *_doubleArray = *rhs._doubleArray;
    if( _intArray.valid()    && rhs._intArray.valid() )     *_intArray    = *rhs._intArray;
    if( _uintArray.valid()   && rhs._uintArray.valid() )    *_uintArray   = *rhs._uintArray;
    dirty();
}

bool Uniform::isCompatibleType( Type t ) const
{
    if( (t==UNDEFINED) || (getType()==UNDEFINED) ) return false;
    if( t == getType() ) return true;
    if( getGlApiType(t) == getGlApiType(getType()) ) return true;

    OSG_WARN << "Cannot assign between Uniform types " << getTypename(t)
             << " and " << getTypename(getType()) << std::endl;
    return false;
}

bool Uniform::isCompatibleType( Type t1, Type t2 ) const
{
    if( (t1==UNDEFINED) || (t2==UNDEFINED) || (getType()==UNDEFINED) ) return false;
    if( (t1 == getType()) || (t2 == getType()) ) return true;
    if( getGlApiType(t1) == getGlApiType(getType()) ) return true;
    if( getGlApiType(t2) == getGlApiType(getType()) ) return true;

    OSG_WARN << "Cannot assign between Uniform types " << getTypename(t1) << " or " << getTypename(t2)
             << " and " << getTypename(getType()) << std::endl;
    return false;
}

unsigned int Uniform::getInternalArrayNumElements() const
{
    if( getNumElements()<1 || getType()==UNDEFINED ) return 0;
    return getNumElements() * getTypeNumComponents(getType());
}


///////////////////////////////////////////////////////////////////////////
// static methods

const char* Uniform::getTypename( Type t )
{
    switch( t )
    {
    case FLOAT:      return "float";
    case FLOAT_VEC2: return "vec2";
    case FLOAT_VEC3: return "vec3";
    case FLOAT_VEC4: return "vec4";

    case DOUBLE:      return "double";
    case DOUBLE_VEC2: return "dvec2";
    case DOUBLE_VEC3: return "dvec3";
    case DOUBLE_VEC4: return "dvec4";

    case INT:      return "int";
    case INT_VEC2: return "ivec2";
    case INT_VEC3: return "ivec3";
    case INT_VEC4: return "ivec4";

    case UNSIGNED_INT:      return "uint";
    case UNSIGNED_INT_VEC2: return "uivec2";
    case UNSIGNED_INT_VEC3: return "uivec3";
    case UNSIGNED_INT_VEC4: return "uivec4";

    case BOOL:      return "bool";
    case BOOL_VEC2: return "bvec2";
    case BOOL_VEC3: return "bvec3";
    case BOOL_VEC4: return "bvec4";

    case FLOAT_MAT2:   return "mat2";
    case FLOAT_MAT3:   return "mat3";
    case FLOAT_MAT4:   return "mat4";
    case FLOAT_MAT2x3: return "mat2x3";
    case FLOAT_MAT2x4: return "mat2x4";
    case FLOAT_MAT3x2: return "mat3x2";
    case FLOAT_MAT3x4: return "mat3x4";
    case FLOAT_MAT4x2: return "mat4x2";
    case FLOAT_MAT4x3: return "mat4x3";

    case DOUBLE_MAT2:   return "dmat2";
    case DOUBLE_MAT3:   return "dmat3";
    case DOUBLE_MAT4:   return "dmat4";
    case DOUBLE_MAT2x3: return "dmat2x3";
    case DOUBLE_MAT2x4: return "dmat2x4";
    case DOUBLE_MAT3x2: return "dmat3x2";
    case DOUBLE_MAT3x4: return "dmat3x4";
    case DOUBLE_MAT4x2: return "dmat4x2";
    case DOUBLE_MAT4x3: return "dmat4x3";

    case SAMPLER_1D:                    return "sampler1D";
    case SAMPLER_2D:                    return "sampler2D";
    case SAMPLER_3D:                    return "sampler3D";
    case SAMPLER_CUBE:                  return "samplerCube";
    case SAMPLER_1D_SHADOW:             return "sampler1DShadow";
    case SAMPLER_2D_SHADOW:             return "sampler2DShadow";
    case SAMPLER_1D_ARRAY:              return "sampler1DArray";
    case SAMPLER_2D_ARRAY:              return "sampler2DArray";
    case SAMPLER_CUBE_MAP_ARRAY:        return "samplerCubeMapArray";
    case SAMPLER_1D_ARRAY_SHADOW:       return "sampler1DArrayShadow";
    case SAMPLER_2D_ARRAY_SHADOW:       return "sampler2DArrayShadow";
    case SAMPLER_2D_MULTISAMPLE:        return "sampler2DMS";
    case SAMPLER_2D_MULTISAMPLE_ARRAY:  return "sampler2DMSArray";
    case SAMPLER_CUBE_SHADOW:           return "samplerCubeShadow";
    case SAMPLER_CUBE_MAP_ARRAY_SHADOW: return "samplerCubeMapArrayShadow";
    case SAMPLER_BUFFER:                return "samplerBuffer";
    case SAMPLER_2D_RECT:               return "sampler2DRect";
    case SAMPLER_2D_RECT_SHADOW:        return "sampler2DRectShadow";

    case INT_SAMPLER_1D:                   return "isampler1D";
    case INT_SAMPLER_2D:                   return "isampler2D";
    case INT_SAMPLER_3D:                   return "isampler3D";
    case INT_SAMPLER_CUBE:                 return "isamplerCube";
    case INT_SAMPLER_1D_ARRAY:             return "isampler1DArray";
    case INT_SAMPLER_2D_ARRAY:             return "isampler2DArray";
    case INT_SAMPLER_CUBE_MAP_ARRAY:       return "isamplerCubeMapArray";
    case INT_SAMPLER_2D_MULTISAMPLE:       return "isampler2DMS";
    case INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "isampler2DMSArray";
    case INT_SAMPLER_BUFFER:               return "isamplerBuffer";
    case INT_SAMPLER_2D_RECT:              return "isampler2DRect";

    case UNSIGNED_INT_SAMPLER_1D:                   return "usample1D";
    case UNSIGNED_INT_SAMPLER_2D:                   return "usample2D";
    case UNSIGNED_INT_SAMPLER_3D:                   return "usample3D";
    case UNSIGNED_INT_SAMPLER_CUBE:                 return "usampleCube";
    case UNSIGNED_INT_SAMPLER_1D_ARRAY:             return "usample1DArray";
    case UNSIGNED_INT_SAMPLER_2D_ARRAY:             return "usample2DArray";
    case UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:       return "usampleCubeMapArray";
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:       return "usample2DMS";
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "usample2DMSArray";
    case UNSIGNED_INT_SAMPLER_BUFFER:               return "usampleBuffer";
    case UNSIGNED_INT_SAMPLER_2D_RECT:              return "usample2DRect";

    case IMAGE_1D:                   return "image1D";
    case IMAGE_2D:                   return "image2D";
    case IMAGE_3D:                   return "image3D";
    case IMAGE_2D_RECT:              return "image2DRect";
    case IMAGE_CUBE:                 return "imageCube";
    case IMAGE_BUFFER:               return "imageBuffer";
    case IMAGE_1D_ARRAY:             return "image1DArray";
    case IMAGE_2D_ARRAY:             return "image2DArray";
    case IMAGE_CUBE_MAP_ARRAY:       return "imageCubeArray";
    case IMAGE_2D_MULTISAMPLE:       return "image2DMS";
    case IMAGE_2D_MULTISAMPLE_ARRAY: return "image2DMSArray";

    case INT_IMAGE_1D:                   return "iimage1D";
    case INT_IMAGE_2D:                   return "iimage2D";
    case INT_IMAGE_3D:                   return "iimage3D";
    case INT_IMAGE_2D_RECT:              return "iimage2DRect";
    case INT_IMAGE_CUBE:                 return "iimageCube";
    case INT_IMAGE_BUFFER:               return "iimageBuffer";
    case INT_IMAGE_1D_ARRAY:             return "iimage1DArray";
    case INT_IMAGE_2D_ARRAY:             return "iimage2DArray";
    case INT_IMAGE_CUBE_MAP_ARRAY:       return "iimageCubeArray";
    case INT_IMAGE_2D_MULTISAMPLE:       return "iimage2DMS";
    case INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "iimage2DMSArray";

    case UNSIGNED_INT_IMAGE_1D:                   return "uimage1D";
    case UNSIGNED_INT_IMAGE_2D:                   return "uimage2D";
    case UNSIGNED_INT_IMAGE_3D:                   return "uimage3D";
    case UNSIGNED_INT_IMAGE_2D_RECT:              return "uimage2DRect";
    case UNSIGNED_INT_IMAGE_CUBE:                 return "uimageCube";
    case UNSIGNED_INT_IMAGE_BUFFER:               return "uimageBuffer";
    case UNSIGNED_INT_IMAGE_1D_ARRAY:             return "uimage1DArray";
    case UNSIGNED_INT_IMAGE_2D_ARRAY:             return "uimage2DArray";
    case UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:       return "uimageCubeArray";
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:       return "uimage2DMS";
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "uimage2DMSArray";

    default: return "UNDEFINED";
    }
}

int Uniform::getTypeNumComponents( Type t )
{
    switch( t )
    {
    case FLOAT:
    case DOUBLE:
    case INT:
    case UNSIGNED_INT:
    case BOOL:

    case SAMPLER_1D:
    case SAMPLER_2D:
    case SAMPLER_3D:
    case SAMPLER_CUBE:
    case SAMPLER_1D_SHADOW:
    case SAMPLER_2D_SHADOW:
    case SAMPLER_1D_ARRAY:
    case SAMPLER_2D_ARRAY:
    case SAMPLER_CUBE_MAP_ARRAY:
    case SAMPLER_1D_ARRAY_SHADOW:
    case SAMPLER_2D_ARRAY_SHADOW:
    case SAMPLER_2D_MULTISAMPLE:
    case SAMPLER_2D_MULTISAMPLE_ARRAY:
    case SAMPLER_CUBE_SHADOW:
    case SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    case SAMPLER_BUFFER:
    case SAMPLER_2D_RECT:
    case SAMPLER_2D_RECT_SHADOW:

    case INT_SAMPLER_1D:
    case INT_SAMPLER_2D:
    case INT_SAMPLER_3D:
    case INT_SAMPLER_CUBE:
    case INT_SAMPLER_1D_ARRAY:
    case INT_SAMPLER_2D_ARRAY:
    case INT_SAMPLER_CUBE_MAP_ARRAY:
    case INT_SAMPLER_2D_MULTISAMPLE:
    case INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case INT_SAMPLER_BUFFER:
    case INT_SAMPLER_2D_RECT:

    case UNSIGNED_INT_SAMPLER_1D:
    case UNSIGNED_INT_SAMPLER_2D:
    case UNSIGNED_INT_SAMPLER_3D:
    case UNSIGNED_INT_SAMPLER_CUBE:
    case UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case UNSIGNED_INT_SAMPLER_BUFFER:
    case UNSIGNED_INT_SAMPLER_2D_RECT:

    case IMAGE_1D:
    case IMAGE_2D:
    case IMAGE_3D:
    case IMAGE_2D_RECT:
    case IMAGE_CUBE:
    case IMAGE_BUFFER:
    case IMAGE_1D_ARRAY:
    case IMAGE_2D_ARRAY:
    case IMAGE_CUBE_MAP_ARRAY:
    case IMAGE_2D_MULTISAMPLE:
    case IMAGE_2D_MULTISAMPLE_ARRAY:

    case INT_IMAGE_1D:
    case INT_IMAGE_2D:
    case INT_IMAGE_3D:
    case INT_IMAGE_2D_RECT:
    case INT_IMAGE_CUBE:
    case INT_IMAGE_BUFFER:
    case INT_IMAGE_1D_ARRAY:
    case INT_IMAGE_2D_ARRAY:
    case INT_IMAGE_CUBE_MAP_ARRAY:
    case INT_IMAGE_2D_MULTISAMPLE:
    case INT_IMAGE_2D_MULTISAMPLE_ARRAY:

    case UNSIGNED_INT_IMAGE_1D:
    case UNSIGNED_INT_IMAGE_2D:
    case UNSIGNED_INT_IMAGE_3D:
    case UNSIGNED_INT_IMAGE_2D_RECT:
    case UNSIGNED_INT_IMAGE_CUBE:
    case UNSIGNED_INT_IMAGE_BUFFER:
    case UNSIGNED_INT_IMAGE_1D_ARRAY:
    case UNSIGNED_INT_IMAGE_2D_ARRAY:
    case UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        return 1;

    case FLOAT_VEC2:
    case DOUBLE_VEC2:
    case INT_VEC2:
    case UNSIGNED_INT_VEC2:
    case BOOL_VEC2:
        return 2;

    case FLOAT_VEC3:
    case DOUBLE_VEC3:
    case INT_VEC3:
    case UNSIGNED_INT_VEC3:
    case BOOL_VEC3:
        return 3;

    case FLOAT_VEC4:
    case DOUBLE_VEC4:
    case FLOAT_MAT2:
    case DOUBLE_MAT2:
    case INT_VEC4:
    case UNSIGNED_INT_VEC4:
    case BOOL_VEC4:
        return 4;

    case FLOAT_MAT2x3:
    case FLOAT_MAT3x2:
    case DOUBLE_MAT2x3:
    case DOUBLE_MAT3x2:
        return 6;

    case FLOAT_MAT2x4:
    case FLOAT_MAT4x2:
    case DOUBLE_MAT2x4:
    case DOUBLE_MAT4x2:
        return 8;

    case FLOAT_MAT3:
    case DOUBLE_MAT3:
        return 9;

    case FLOAT_MAT3x4:
    case FLOAT_MAT4x3:
    case DOUBLE_MAT3x4:
    case DOUBLE_MAT4x3:
        return 12;

    case FLOAT_MAT4:
    case DOUBLE_MAT4:
        return 16;

    default:
        return 0;
    }
}

Uniform::Type Uniform::getTypeId( const std::string& tname )
{
    if( tname == "float" )           return FLOAT;
    if( tname == "vec2" )            return FLOAT_VEC2;
    if( tname == "vec3" )            return FLOAT_VEC3;
    if( tname == "vec4" )            return FLOAT_VEC4;

    if( tname == "double" )          return DOUBLE;
    if( tname == "dvec2" )           return DOUBLE_VEC2;
    if( tname == "dvec3" )           return DOUBLE_VEC3;
    if( tname == "dvec4" )           return DOUBLE_VEC4;

    if( tname == "int" )             return INT;
    if( tname == "ivec2" )           return INT_VEC2;
    if( tname == "ivec3" )           return INT_VEC3;
    if( tname == "ivec4" )           return INT_VEC4;

    if( tname == "unsigned int" || tname == "uint" ) return UNSIGNED_INT;
    if( tname == "uvec2" )                           return UNSIGNED_INT_VEC2;
    if( tname == "uvec3" )                           return UNSIGNED_INT_VEC3;
    if( tname == "uvec4" )                           return UNSIGNED_INT_VEC4;

    if( tname == "bool" )            return BOOL;
    if( tname == "bvec2" )           return BOOL_VEC2;
    if( tname == "bvec3" )           return BOOL_VEC3;
    if( tname == "bvec4" )           return BOOL_VEC4;

    if( tname == "mat2" || tname == "mat2x2" ) return FLOAT_MAT2;
    if( tname == "mat3" || tname == "mat3x3" ) return FLOAT_MAT3;
    if( tname == "mat4" || tname == "mat4x4" ) return FLOAT_MAT4;
    if( tname == "mat2x3" ) return FLOAT_MAT2x3;
    if( tname == "mat2x4" ) return FLOAT_MAT2x4;
    if( tname == "mat3x2" ) return FLOAT_MAT3x2;
    if( tname == "mat3x4" ) return FLOAT_MAT3x4;
    if( tname == "mat4x2" ) return FLOAT_MAT4x2;
    if( tname == "mat4x3" ) return FLOAT_MAT4x3;

    if( tname == "mat2d" || tname == "mat2x2d" ) return DOUBLE_MAT2;
    if( tname == "mat3d" || tname == "mat3x3d" ) return DOUBLE_MAT3;
    if( tname == "mat4d" || tname == "mat4x4d" ) return DOUBLE_MAT4;
    if( tname == "mat2x3d" ) return DOUBLE_MAT2x3;
    if( tname == "mat2x4d" ) return DOUBLE_MAT2x4;
    if( tname == "mat3x2d" ) return DOUBLE_MAT3x2;
    if( tname == "mat3x4d" ) return DOUBLE_MAT3x4;
    if( tname == "mat4x2d" ) return DOUBLE_MAT4x2;
    if( tname == "mat4x3d" ) return DOUBLE_MAT4x3;

    if( tname == "sampler1D" )                 return SAMPLER_1D;
    if( tname == "sampler2D" )                 return SAMPLER_2D;
    if( tname == "sampler3D" )                 return SAMPLER_3D;
    if( tname == "samplerCube" )               return SAMPLER_CUBE;
    if( tname == "sampler1DShadow" )           return SAMPLER_1D_SHADOW;
    if( tname == "sampler2DShadow" )           return SAMPLER_2D_SHADOW;
    if( tname == "sampler1DArray" )            return SAMPLER_1D_ARRAY;
    if( tname == "sampler2DArray" )            return SAMPLER_2D_ARRAY;
    if( tname == "samplerCubeMapArray" )       return SAMPLER_CUBE_MAP_ARRAY;
    if( tname == "sampler1DArrayShadow" )      return SAMPLER_1D_ARRAY_SHADOW;
    if( tname == "sampler2DArrayShadow" )      return SAMPLER_2D_ARRAY_SHADOW;
    if( tname == "sampler2DMS" )               return SAMPLER_2D_MULTISAMPLE;
    if( tname == "sampler2DMSArray" )          return SAMPLER_2D_MULTISAMPLE_ARRAY;
    if( tname == "samplerCubeShadow" )         return SAMPLER_CUBE_SHADOW;
    if( tname == "samplerCubeMapArrayShadow" ) return SAMPLER_CUBE_MAP_ARRAY_SHADOW;
    if( tname == "samplerBuffer" )             return SAMPLER_BUFFER;
    if( tname == "sampler2DRect" )             return SAMPLER_2D_RECT;
    if( tname == "sampler2DRectShadow" )       return SAMPLER_2D_RECT_SHADOW;

    if( tname == "isampler1D" )           return INT_SAMPLER_1D;
    if( tname == "isampler2D" )           return INT_SAMPLER_2D;
    if( tname == "isampler3D" )           return INT_SAMPLER_3D;
    if( tname == "isamplerCube" )         return INT_SAMPLER_CUBE;
    if( tname == "isampler1DArray" )      return INT_SAMPLER_1D_ARRAY;
    if( tname == "isampler2DArray" )      return INT_SAMPLER_2D_ARRAY;
    if( tname == "isamplerCubeMapArray" ) return INT_SAMPLER_CUBE_MAP_ARRAY;
    if( tname == "isampler2DMS" )         return INT_SAMPLER_2D_MULTISAMPLE;
    if( tname == "isampler2DMSArray" )    return INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
    if( tname == "isamplerBuffer" )       return INT_SAMPLER_BUFFER;
    if( tname == "isampler2DRect" )       return INT_SAMPLER_2D_RECT;

    if( tname == "usampler1D" )           return UNSIGNED_INT_SAMPLER_1D;
    if( tname == "usampler2D" )           return UNSIGNED_INT_SAMPLER_2D;
    if( tname == "usampler3D" )           return UNSIGNED_INT_SAMPLER_3D;
    if( tname == "usamplerCube" )         return UNSIGNED_INT_SAMPLER_CUBE;
    if( tname == "usampler1DArray" )      return UNSIGNED_INT_SAMPLER_1D_ARRAY;
    if( tname == "usampler2DArray" )      return UNSIGNED_INT_SAMPLER_2D_ARRAY;
    if( tname == "usamplerCubeMapArray" ) return UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY;
    if( tname == "usampler2DMS" )         return UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
    if( tname == "usampler2DMSArray" )    return UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
    if( tname == "usamplerBuffer" )       return UNSIGNED_INT_SAMPLER_BUFFER;
    if( tname == "usampler2DRect" )       return UNSIGNED_INT_SAMPLER_2D_RECT;

    if( tname == "image1D" )        return IMAGE_1D;
    if( tname == "image2D" )        return IMAGE_2D;
    if( tname == "image3D" )        return IMAGE_3D;
    if( tname == "image2DRect" )    return IMAGE_2D_RECT;
    if( tname == "imageCube" )      return IMAGE_CUBE;
    if( tname == "imageBuffer" )    return IMAGE_BUFFER;
    if( tname == "image1DArray" )   return IMAGE_1D_ARRAY;
    if( tname == "image2DArray" )   return IMAGE_2D_ARRAY;
    if( tname == "imageCubeArray" ) return IMAGE_CUBE_MAP_ARRAY;
    if( tname == "image2DMS" )      return IMAGE_2D_MULTISAMPLE;
    if( tname == "image2DMSArray" ) return IMAGE_2D_MULTISAMPLE_ARRAY;

    if( tname == "iimage1D" )        return INT_IMAGE_1D;
    if( tname == "iimage2D" )        return INT_IMAGE_2D;
    if( tname == "iimage3D" )        return INT_IMAGE_3D;
    if( tname == "iimage2DRect" )    return INT_IMAGE_2D_RECT;
    if( tname == "iimageCube" )      return INT_IMAGE_CUBE;
    if( tname == "iimageBuffer" )    return INT_IMAGE_BUFFER;
    if( tname == "iimage1DArray" )   return INT_IMAGE_1D_ARRAY;
    if( tname == "iimage2DArray" )   return INT_IMAGE_2D_ARRAY;
    if( tname == "iimageCubeArray" ) return INT_IMAGE_CUBE_MAP_ARRAY;
    if( tname == "iimage2DMS" )      return INT_IMAGE_2D_MULTISAMPLE;
    if( tname == "iimage2DMSArray" ) return INT_IMAGE_2D_MULTISAMPLE_ARRAY;

    if( tname == "uimage1D" )        return UNSIGNED_INT_IMAGE_1D;
    if( tname == "uimage2D" )        return UNSIGNED_INT_IMAGE_2D;
    if( tname == "uimage3D" )        return UNSIGNED_INT_IMAGE_3D;
    if( tname == "uimage2DRect" )    return UNSIGNED_INT_IMAGE_2D_RECT;
    if( tname == "uimageCube" )      return UNSIGNED_INT_IMAGE_CUBE;
    if( tname == "uimageBuffer" )    return UNSIGNED_INT_IMAGE_BUFFER;
    if( tname == "uimage1DArray" )   return UNSIGNED_INT_IMAGE_1D_ARRAY;
    if( tname == "uimage2DArray" )   return UNSIGNED_INT_IMAGE_2D_ARRAY;
    if( tname == "uimageCubeArray" ) return UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY;
    if( tname == "uimage2DMS" )      return UNSIGNED_INT_IMAGE_2D_MULTISAMPLE;
    if( tname == "uimage2DMSArray" ) return UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY;

    return UNDEFINED;
}

Uniform::Type Uniform::getGlApiType( Type t )
{
    switch( t )
    {
    case BOOL:

    case SAMPLER_1D:
    case SAMPLER_2D:
    case SAMPLER_3D:
    case SAMPLER_CUBE:
    case SAMPLER_1D_SHADOW:
    case SAMPLER_2D_SHADOW:
    case SAMPLER_1D_ARRAY:
    case SAMPLER_2D_ARRAY:
    case SAMPLER_CUBE_MAP_ARRAY:
    case SAMPLER_1D_ARRAY_SHADOW:
    case SAMPLER_2D_ARRAY_SHADOW:
    case SAMPLER_2D_MULTISAMPLE:
    case SAMPLER_2D_MULTISAMPLE_ARRAY:
    case SAMPLER_CUBE_SHADOW:
    case SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    case SAMPLER_BUFFER:
    case SAMPLER_2D_RECT:
    case SAMPLER_2D_RECT_SHADOW:

    case INT_SAMPLER_1D:
    case INT_SAMPLER_2D:
    case INT_SAMPLER_3D:
    case INT_SAMPLER_CUBE:
    case INT_SAMPLER_1D_ARRAY:
    case INT_SAMPLER_2D_ARRAY:
    case INT_SAMPLER_CUBE_MAP_ARRAY:
    case INT_SAMPLER_2D_MULTISAMPLE:
    case INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case INT_SAMPLER_BUFFER:
    case INT_SAMPLER_2D_RECT:

    case UNSIGNED_INT_SAMPLER_1D:
    case UNSIGNED_INT_SAMPLER_2D:
    case UNSIGNED_INT_SAMPLER_3D:
    case UNSIGNED_INT_SAMPLER_CUBE:
    case UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case UNSIGNED_INT_SAMPLER_BUFFER:
    case UNSIGNED_INT_SAMPLER_2D_RECT:

    case IMAGE_1D:
    case IMAGE_2D:
    case IMAGE_3D:
    case IMAGE_2D_RECT:
    case IMAGE_CUBE:
    case IMAGE_BUFFER:
    case IMAGE_1D_ARRAY:
    case IMAGE_2D_ARRAY:
    case IMAGE_CUBE_MAP_ARRAY:
    case IMAGE_2D_MULTISAMPLE:
    case IMAGE_2D_MULTISAMPLE_ARRAY:

    case INT_IMAGE_1D:
    case INT_IMAGE_2D:
    case INT_IMAGE_3D:
    case INT_IMAGE_2D_RECT:
    case INT_IMAGE_CUBE:
    case INT_IMAGE_BUFFER:
    case INT_IMAGE_1D_ARRAY:
    case INT_IMAGE_2D_ARRAY:
    case INT_IMAGE_CUBE_MAP_ARRAY:
    case INT_IMAGE_2D_MULTISAMPLE:
    case INT_IMAGE_2D_MULTISAMPLE_ARRAY:

    case UNSIGNED_INT_IMAGE_1D:
    case UNSIGNED_INT_IMAGE_2D:
    case UNSIGNED_INT_IMAGE_3D:
    case UNSIGNED_INT_IMAGE_2D_RECT:
    case UNSIGNED_INT_IMAGE_CUBE:
    case UNSIGNED_INT_IMAGE_BUFFER:
    case UNSIGNED_INT_IMAGE_1D_ARRAY:
    case UNSIGNED_INT_IMAGE_2D_ARRAY:
    case UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        return INT;

    case BOOL_VEC2:
        return INT_VEC2;

    case BOOL_VEC3:
        return INT_VEC3;

    case BOOL_VEC4:
        return INT_VEC4;

    default:
        return t;
    }
}

GLenum Uniform::getInternalArrayType( Type t )
{
    switch( t )
    {
    case FLOAT:
    case FLOAT_VEC2:
    case FLOAT_VEC3:
    case FLOAT_VEC4:
    case FLOAT_MAT2:
    case FLOAT_MAT3:
    case FLOAT_MAT4:
    case FLOAT_MAT2x3:
    case FLOAT_MAT2x4:
    case FLOAT_MAT3x2:
    case FLOAT_MAT3x4:
    case FLOAT_MAT4x2:
    case FLOAT_MAT4x3:
        return GL_FLOAT;

    case DOUBLE:
    case DOUBLE_VEC2:
    case DOUBLE_VEC3:
    case DOUBLE_VEC4:
    case DOUBLE_MAT2:
    case DOUBLE_MAT3:
    case DOUBLE_MAT4:
    case DOUBLE_MAT2x3:
    case DOUBLE_MAT2x4:
    case DOUBLE_MAT3x2:
    case DOUBLE_MAT3x4:
    case DOUBLE_MAT4x2:
    case DOUBLE_MAT4x3:
        return GL_DOUBLE;

    case INT:
    case INT_VEC2:
    case INT_VEC3:
    case INT_VEC4:
    case BOOL:
    case BOOL_VEC2:
    case BOOL_VEC3:
    case BOOL_VEC4:
    case SAMPLER_1D:
    case SAMPLER_2D:
    case SAMPLER_3D:
    case SAMPLER_CUBE:
    case SAMPLER_1D_SHADOW:
    case SAMPLER_2D_SHADOW:
    case SAMPLER_1D_ARRAY:
    case SAMPLER_2D_ARRAY:
    case SAMPLER_CUBE_MAP_ARRAY:
    case SAMPLER_1D_ARRAY_SHADOW:
    case SAMPLER_2D_ARRAY_SHADOW:
    case SAMPLER_2D_MULTISAMPLE:
    case SAMPLER_2D_MULTISAMPLE_ARRAY:
    case SAMPLER_CUBE_SHADOW:
    case SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    case SAMPLER_BUFFER:
    case SAMPLER_2D_RECT:
    case SAMPLER_2D_RECT_SHADOW:

    case INT_SAMPLER_1D:
    case INT_SAMPLER_2D:
    case INT_SAMPLER_3D:
    case INT_SAMPLER_CUBE:
    case INT_SAMPLER_1D_ARRAY:
    case INT_SAMPLER_2D_ARRAY:
    case INT_SAMPLER_CUBE_MAP_ARRAY:
    case INT_SAMPLER_2D_MULTISAMPLE:
    case INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case INT_SAMPLER_BUFFER:
    case INT_SAMPLER_2D_RECT:

    case UNSIGNED_INT_SAMPLER_1D:
    case UNSIGNED_INT_SAMPLER_2D:
    case UNSIGNED_INT_SAMPLER_3D:
    case UNSIGNED_INT_SAMPLER_CUBE:
    case UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case UNSIGNED_INT_SAMPLER_BUFFER:
    case UNSIGNED_INT_SAMPLER_2D_RECT:

    case IMAGE_1D:
    case IMAGE_2D:
    case IMAGE_3D:
    case IMAGE_2D_RECT:
    case IMAGE_CUBE:
    case IMAGE_BUFFER:
    case IMAGE_1D_ARRAY:
    case IMAGE_2D_ARRAY:
    case IMAGE_CUBE_MAP_ARRAY:
    case IMAGE_2D_MULTISAMPLE:
    case IMAGE_2D_MULTISAMPLE_ARRAY:

    case INT_IMAGE_1D:
    case INT_IMAGE_2D:
    case INT_IMAGE_3D:
    case INT_IMAGE_2D_RECT:
    case INT_IMAGE_CUBE:
    case INT_IMAGE_BUFFER:
    case INT_IMAGE_1D_ARRAY:
    case INT_IMAGE_2D_ARRAY:
    case INT_IMAGE_CUBE_MAP_ARRAY:
    case INT_IMAGE_2D_MULTISAMPLE:
    case INT_IMAGE_2D_MULTISAMPLE_ARRAY:

    case UNSIGNED_INT_IMAGE_1D:
    case UNSIGNED_INT_IMAGE_2D:
    case UNSIGNED_INT_IMAGE_3D:
    case UNSIGNED_INT_IMAGE_2D_RECT:
    case UNSIGNED_INT_IMAGE_CUBE:
    case UNSIGNED_INT_IMAGE_BUFFER:
    case UNSIGNED_INT_IMAGE_1D_ARRAY:
    case UNSIGNED_INT_IMAGE_2D_ARRAY:
    case UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
    case UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        return GL_INT;

    case UNSIGNED_INT:
    case UNSIGNED_INT_VEC2:
    case UNSIGNED_INT_VEC3:
    case UNSIGNED_INT_VEC4:
        return GL_UNSIGNED_INT;

    default:
        return 0;
    }
}


unsigned int Uniform::getNameID(const std::string& name)
{
    typedef std::map<std::string, unsigned int> UniformNameIDMap;
    static OpenThreads::Mutex s_mutex_uniformNameIDMap;
    static UniformNameIDMap s_uniformNameIDMap;
    
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_uniformNameIDMap);
    UniformNameIDMap::iterator it = s_uniformNameIDMap.find(name);
    if (it != s_uniformNameIDMap.end())
    {
        return it->second;
    }
    unsigned int id = s_uniformNameIDMap.size();
    s_uniformNameIDMap.insert(UniformNameIDMap::value_type(name, id));
    return id;
}

// Use a proxy to force the initialization of the static variables in the Unifrom::getNameID() method during static initialization
OSG_INIT_SINGLETON_PROXY(UniformNameIDStaticInitializationProxy, Uniform::getNameID(std::string()))


///////////////////////////////////////////////////////////////////////////
// value constructors for single-element (ie: non-array) uniforms

Uniform::Uniform( const char* name, float f ) :
    _type(FLOAT), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( f );
}

Uniform::Uniform( const char* name, const osg::Vec2& v2 ) :
    _type(FLOAT_VEC2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( v2 );
}

Uniform::Uniform( const char* name, const osg::Vec3& v3 ) :
     _type(FLOAT_VEC3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( v3 );
}

Uniform::Uniform( const char* name, const osg::Vec4& v4 ) :
    _type(FLOAT_VEC4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( v4 );
}

Uniform::Uniform( const char* name, const osg::Matrix2& m2 ) :
    _type(FLOAT_MAT2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m2 );
}

Uniform::Uniform( const char* name, const osg::Matrix3& m3 ) :
    _type(FLOAT_MAT3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m3 );
}

Uniform::Uniform( const char* name, const osg::Matrixf& m4 ) :
    _type(FLOAT_MAT4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m4 );
}

Uniform::Uniform( const char* name, const osg::Matrix2x3& m2x3 ) :
    _type(FLOAT_MAT2x3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m2x3 );
}

Uniform::Uniform( const char* name, const osg::Matrix2x4& m2x4 ) :
    _type(FLOAT_MAT2x4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m2x4 );
}

Uniform::Uniform( const char* name, const osg::Matrix3x2& m3x2 ) :
    _type(FLOAT_MAT3x2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m3x2 );
}

Uniform::Uniform( const char* name, const osg::Matrix3x4& m3x4 ) :
    _type(FLOAT_MAT3x4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m3x4 );
}

Uniform::Uniform( const char* name, const osg::Matrix4x2& m4x2 ) :
    _type(FLOAT_MAT4x2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m4x2 );
}

Uniform::Uniform( const char* name, const osg::Matrix4x3& m4x3 ) :
    _type(FLOAT_MAT4x3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m4x3 );
}

Uniform::Uniform( const char* name, double d ) :
    _type(DOUBLE), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( d );
}

Uniform::Uniform( const char* name, const osg::Vec2d& v2 ) :
    _type(DOUBLE_VEC2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( v2 );
}

Uniform::Uniform( const char* name, const osg::Vec3d& v3 ) :
     _type(DOUBLE_VEC3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( v3 );
}

Uniform::Uniform( const char* name, const osg::Vec4d& v4 ) :
    _type(DOUBLE_VEC4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( v4 );
}

Uniform::Uniform( const char* name, const osg::Matrix2d& m2 ) :
    _type(DOUBLE_MAT2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m2 );
}

Uniform::Uniform( const char* name, const osg::Matrix3d& m3 ) :
    _type(DOUBLE_MAT3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m3 );
}

Uniform::Uniform( const char* name, const osg::Matrixd& m4 ) :
    _type(DOUBLE_MAT4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m4 );
}

Uniform::Uniform( const char* name, const osg::Matrix2x3d& m2x3 ) :
    _type(DOUBLE_MAT2x3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m2x3 );
}

Uniform::Uniform( const char* name, const osg::Matrix2x4d& m2x4 ) :
    _type(DOUBLE_MAT2x4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m2x4 );
}

Uniform::Uniform( const char* name, const osg::Matrix3x2d& m3x2 ) :
    _type(DOUBLE_MAT3x2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m3x2 );
}

Uniform::Uniform( const char* name, const osg::Matrix3x4d& m3x4 ) :
    _type(DOUBLE_MAT3x4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m3x4 );
}

Uniform::Uniform( const char* name, const osg::Matrix4x2d& m4x2 ) :
    _type(DOUBLE_MAT4x2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m4x2 );
}

Uniform::Uniform( const char* name, const osg::Matrix4x3d& m4x3 ) :
    _type(DOUBLE_MAT4x3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m4x3 );
}

Uniform::Uniform( const char* name, int i ) :
    _type(INT), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( i );
}

Uniform::Uniform( const char* name, int i0, int i1 ) :
    _type(INT_VEC2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( i0, i1 );
}

Uniform::Uniform( const char* name, int i0, int i1, int i2 ) :
    _type(INT_VEC3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( i0, i1, i2 );
}

Uniform::Uniform( const char* name, int i0, int i1, int i2, int i3 ) :
    _type(INT_VEC4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( i0, i1, i2, i3 );
}

Uniform::Uniform( const char* name, unsigned int ui ) :
    _type(UNSIGNED_INT), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( ui );
}

Uniform::Uniform( const char* name, unsigned int ui0, unsigned int ui1 ) :
    _type(UNSIGNED_INT_VEC2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( ui0, ui1 );
}

Uniform::Uniform( const char* name, unsigned int ui0, unsigned int ui1, unsigned int ui2 ) :
    _type(UNSIGNED_INT_VEC3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( ui0, ui1, ui2 );
}

Uniform::Uniform( const char* name, unsigned int ui0, unsigned int ui1, unsigned int ui2, unsigned int ui3 ) :
    _type(UNSIGNED_INT_VEC4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( ui0, ui1, ui2, ui3 );
}

Uniform::Uniform( const char* name, bool b ) :
    _type(BOOL), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( b );
}

Uniform::Uniform( const char* name, bool b0, bool b1 ) :
     _type(BOOL_VEC2), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( b0, b1 );
}

Uniform::Uniform( const char* name, bool b0, bool b1, bool b2 ) :
    _type(BOOL_VEC3), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( b0, b1, b2 );
}

Uniform::Uniform( const char* name, bool b0, bool b1, bool b2, bool b3 ) :
    _type(BOOL_VEC4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( b0, b1, b2, b3 );
}

///////////////////////////////////////////////////////////////////////////
// Value assignment for single-element (ie: non-array) uniforms.
// (For backwards compatability, if not already configured, set the
// Uniform's _numElements=1)

bool Uniform::set( float f )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,f) : false;
}

bool Uniform::set( const osg::Vec2& v2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,v2) : false;
}

bool Uniform::set( const osg::Vec3& v3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,v3) : false;
}

bool Uniform::set( const osg::Vec4& v4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,v4) : false;
}

bool Uniform::set( const osg::Matrix2& m2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m2) : false;
}

bool Uniform::set( const osg::Matrix3& m3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m3) : false;
}

bool Uniform::set( const osg::Matrixf& m4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m4) : false;
}

bool Uniform::set( const osg::Matrix2x3& m2x3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m2x3) : false;
}

bool Uniform::set( const osg::Matrix2x4& m2x4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m2x4) : false;
}

bool Uniform::set( const osg::Matrix3x2& m3x2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m3x2) : false;
}

bool Uniform::set( const osg::Matrix3x4& m3x4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m3x4) : false;
}

bool Uniform::set( const osg::Matrix4x2& m4x2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m4x2) : false;
}

bool Uniform::set( const osg::Matrix4x3& m4x3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m4x3) : false;
}

bool Uniform::set( double d )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,d) : false;
}

bool Uniform::set( const osg::Vec2d& v2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,v2) : false;
}

bool Uniform::set( const osg::Vec3d& v3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,v3) : false;
}

bool Uniform::set( const osg::Vec4d& v4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,v4) : false;
}

bool Uniform::set( const osg::Matrix2d& m2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m2) : false;
}

bool Uniform::set( const osg::Matrix3d& m3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m3) : false;
}

bool Uniform::set( const osg::Matrixd& m4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m4) : false;
}

bool Uniform::set( const osg::Matrix2x3d& m2x3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m2x3) : false;
}

bool Uniform::set( const osg::Matrix2x4d& m2x4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m2x4) : false;
}

bool Uniform::set( const osg::Matrix3x2d& m3x2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m3x2) : false;
}

bool Uniform::set( const osg::Matrix3x4d& m3x4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m3x4) : false;
}

bool Uniform::set( const osg::Matrix4x2d& m4x2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m4x2) : false;
}

bool Uniform::set( const osg::Matrix4x3d& m4x3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m4x3) : false;
}

bool Uniform::set( int i )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,i) : false;
}

bool Uniform::set( int i0, int i1 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,i0,i1) : false;
}

bool Uniform::set( int i0, int i1, int i2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,i0,i1,i2) : false;
}

bool Uniform::set( int i0, int i1, int i2, int i3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,i0,i1,i2,i3) : false;
}

bool Uniform::set( unsigned int ui )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,ui) : false;
}

bool Uniform::set( unsigned int ui0, unsigned int ui1 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,ui0,ui1) : false;
}

bool Uniform::set( unsigned int ui0, unsigned int ui1, unsigned int ui2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,ui0,ui1,ui2) : false;
}

bool Uniform::set( unsigned int ui0, unsigned int ui1, unsigned int ui2, unsigned int ui3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,ui0,ui1,ui2,ui3) : false;
}

bool Uniform::set( bool b )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,b) : false;
}

bool Uniform::set( bool b0, bool b1 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,b0,b1) : false;
}

bool Uniform::set( bool b0, bool b1, bool b2 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,b0,b1,b2) : false;
}

bool Uniform::set( bool b0, bool b1, bool b2, bool b3 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,b0,b1,b2,b3) : false;
}

///////////////////////////////////////////////////////////////////////////
// Value query for single-element (ie: non-array) uniforms.

bool Uniform::get( float& f ) const
{
    return isScalar() ? getElement(0,f) : false;
}

bool Uniform::get( osg::Vec2& v2 ) const
{
    return isScalar() ? getElement(0,v2) : false;
}

bool Uniform::get( osg::Vec3& v3 ) const
{
    return isScalar() ? getElement(0,v3) : false;
}

bool Uniform::get( osg::Vec4& v4 ) const
{
    return isScalar() ? getElement(0,v4) : false;
}

bool Uniform::get( osg::Matrix2& m2 ) const
{
    return isScalar() ? getElement(0,m2) : false;
}

bool Uniform::get( osg::Matrix3& m3 ) const
{
    return isScalar() ? getElement(0,m3) : false;
}

bool Uniform::get( osg::Matrixf& m4 ) const
{
    return isScalar() ? getElement(0,m4) : false;
}

bool Uniform::get( osg::Matrix2x3& m2x3 ) const
{
    return isScalar() ? getElement(0,m2x3) : false;
}

bool Uniform::get( osg::Matrix2x4& m2x4 ) const
{
    return isScalar() ? getElement(0,m2x4) : false;
}

bool Uniform::get( osg::Matrix3x2& m3x2 ) const
{
    return isScalar() ? getElement(0,m3x2) : false;
}

bool Uniform::get( osg::Matrix3x4& m3x4 ) const
{
    return isScalar() ? getElement(0,m3x4) : false;
}

bool Uniform::get( osg::Matrix4x2& m4x2 ) const
{
    return isScalar() ? getElement(0,m4x2) : false;
}

bool Uniform::get( osg::Matrix4x3& m4x3 ) const
{
    return isScalar() ? getElement(0,m4x3) : false;
}

bool Uniform::get( double& d ) const
{
    return isScalar() ? getElement(0,d) : false;
}

bool Uniform::get( osg::Vec2d& v2 ) const
{
    return isScalar() ? getElement(0,v2) : false;
}

bool Uniform::get( osg::Vec3d& v3 ) const
{
    return isScalar() ? getElement(0,v3) : false;
}

bool Uniform::get( osg::Vec4d& v4 ) const
{
    return isScalar() ? getElement(0,v4) : false;
}

bool Uniform::get( osg::Matrix2d& m2 ) const
{
    return isScalar() ? getElement(0,m2) : false;
}

bool Uniform::get( osg::Matrix3d& m3 ) const
{
    return isScalar() ? getElement(0,m3) : false;
}

bool Uniform::get( osg::Matrixd& m4 ) const
{
    return isScalar() ? getElement(0,m4) : false;
}

bool Uniform::get( osg::Matrix2x3d& m2x3 ) const
{
    return isScalar() ? getElement(0,m2x3) : false;
}

bool Uniform::get( osg::Matrix2x4d& m2x4 ) const
{
    return isScalar() ? getElement(0,m2x4) : false;
}

bool Uniform::get( osg::Matrix3x2d& m3x2 ) const
{
    return isScalar() ? getElement(0,m3x2) : false;
}

bool Uniform::get( osg::Matrix3x4d& m3x4 ) const
{
    return isScalar() ? getElement(0,m3x4) : false;
}

bool Uniform::get( osg::Matrix4x2d& m4x2 ) const
{
    return isScalar() ? getElement(0,m4x2) : false;
}

bool Uniform::get( osg::Matrix4x3d& m4x3 ) const
{
    return isScalar() ? getElement(0,m4x3) : false;
}

bool Uniform::get( int& i ) const
{
    return isScalar() ? getElement(0,i) : false;
}

bool Uniform::get( int& i0, int& i1 ) const
{
    return isScalar() ? getElement(0,i0,i1) : false;
}

bool Uniform::get( int& i0, int& i1, int& i2 ) const
{
    return isScalar() ? getElement(0,i0,i1,i2) : false;
}

bool Uniform::get( int& i0, int& i1, int& i2, int& i3 ) const
{
    return isScalar() ? getElement(0,i0,i1,i2,i3) : false;
}

bool Uniform::get( unsigned int& ui ) const
{
    return isScalar() ? getElement(0,ui) : false;
}

bool Uniform::get( unsigned int& ui0, unsigned int& ui1 ) const
{
    return isScalar() ? getElement(0,ui0,ui1) : false;
}

bool Uniform::get( unsigned int& ui0, unsigned int& ui1, unsigned int& ui2 ) const
{
    return isScalar() ? getElement(0,ui0,ui1,ui2) : false;
}

bool Uniform::get( unsigned int& ui0, unsigned int& ui1, unsigned int& ui2, unsigned int& ui3 ) const
{
    return isScalar() ? getElement(0,ui0,ui1,ui2,ui3) : false;
}

bool Uniform::get( bool& b ) const
{
    return isScalar() ? getElement(0,b) : false;
}

bool Uniform::get( bool& b0, bool& b1 ) const
{
    return isScalar() ? getElement(0,b0,b1) : false;
}

bool Uniform::get( bool& b0, bool& b1, bool& b2 ) const
{
    return isScalar() ? getElement(0,b0,b1,b2) : false;
}

bool Uniform::get( bool& b0, bool& b1, bool& b2, bool& b3 ) const
{
    return isScalar() ? getElement(0,b0,b1,b2,b3) : false;
}

///////////////////////////////////////////////////////////////////////////
// Value assignment for array uniforms.

bool Uniform::setElement( unsigned int index, float f )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_floatArray)[j] = f;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Vec2& v2 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_floatArray)[j] = v2.x();
    (*_floatArray)[j+1] = v2.y();
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Vec3& v3 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_floatArray)[j] = v3.x();
    (*_floatArray)[j+1] = v3.y();
    (*_floatArray)[j+2] = v3.z();
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Vec4& v4 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_floatArray)[j] = v4.x();
    (*_floatArray)[j+1] = v4.y();
    (*_floatArray)[j+2] = v4.z();
    (*_floatArray)[j+3] = v4.w();
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix2& m2 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 4; ++i ) (*_floatArray)[j+i] = m2[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix3& m3 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 9; ++i ) (*_floatArray)[j+i] = m3[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrixf& m4 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    const Matrixf::value_type* p = m4.ptr();
    for( int i = 0; i < 16; ++i ) (*_floatArray)[j+i] = p[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix2x3& m2x3 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT2x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 6; ++i ) (*_floatArray)[j+i] = m2x3[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix2x4& m2x4 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT2x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 8; ++i ) (*_floatArray)[j+i] = m2x4[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix3x2& m3x2 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT3x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 6; ++i ) (*_floatArray)[j+i] = m3x2[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix3x4& m3x4 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT3x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 12; ++i ) (*_floatArray)[j+i] = m3x4[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix4x2& m4x2 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 8; ++i ) (*_floatArray)[j+i] = m4x2[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix4x3& m4x3 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 12; ++i ) (*_floatArray)[j+i] = m4x3[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, double d )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_doubleArray)[j] = d;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Vec2d& v2 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_doubleArray)[j] = v2.x();
    (*_doubleArray)[j+1] = v2.y();
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Vec3d& v3 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_doubleArray)[j] = v3.x();
    (*_doubleArray)[j+1] = v3.y();
    (*_doubleArray)[j+2] = v3.z();
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Vec4d& v4 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_doubleArray)[j] = v4.x();
    (*_doubleArray)[j+1] = v4.y();
    (*_doubleArray)[j+2] = v4.z();
    (*_doubleArray)[j+3] = v4.w();
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix2d& m2 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 4; ++i ) (*_doubleArray)[j+i] = m2[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix3d& m3 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 9; ++i ) (*_doubleArray)[j+i] = m3[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrixd& m4 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT4, FLOAT_MAT4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());

    if (_type == DOUBLE_MAT4)
    {
        const Matrixd::value_type* p = m4.ptr();
        for( int i = 0; i < 16; ++i ) (*_doubleArray)[j+i] = p[i];
    }
    else //if (_type == FLOAT_MAT4) for backward compatibility only
    {
        const Matrixd::value_type* p = m4.ptr();
        for( int i = 0; i < 16; ++i ) (*_floatArray)[j+i] = static_cast<float>(p[i]);
    }
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix2x3d& m2x3 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT2x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 6; ++i ) (*_doubleArray)[j+i] = m2x3[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix2x4d& m2x4 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT2x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 8; ++i ) (*_doubleArray)[j+i] = m2x4[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix3x2d& m3x2 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT3x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 6; ++i ) (*_doubleArray)[j+i] = m3x2[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix3x4d& m3x4 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT3x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 12; ++i ) (*_doubleArray)[j+i] = m3x4[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix4x2d& m4x2 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT4x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 8; ++i ) (*_doubleArray)[j+i] = m4x2[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, const osg::Matrix4x3d& m4x3 )
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT4x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    for( int i = 0; i < 12; ++i ) (*_doubleArray)[j+i] = m4x3[i];
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, int i )
{
    if( index>=getNumElements() || !isCompatibleType(INT) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = i;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, int i0, int i1 )
{
    if( index>=getNumElements() || !isCompatibleType(INT_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = i0;
    (*_intArray)[j+1] = i1;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, int i0, int i1, int i2 )
{
    if( index>=getNumElements() || !isCompatibleType(INT_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = i0;
    (*_intArray)[j+1] = i1;
    (*_intArray)[j+2] = i2;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, int i0, int i1, int i2, int i3 )
{
    if( index>=getNumElements() || !isCompatibleType(INT_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = i0;
    (*_intArray)[j+1] = i1;
    (*_intArray)[j+2] = i2;
    (*_intArray)[j+3] = i3;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, unsigned int ui )
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_uintArray)[j] = ui;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, unsigned int ui0, unsigned int ui1 )
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_uintArray)[j] = ui0;
    (*_uintArray)[j+1] = ui1;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, unsigned int ui0, unsigned int ui1, unsigned int ui2 )
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_uintArray)[j] = ui0;
    (*_uintArray)[j+1] = ui1;
    (*_uintArray)[j+2] = ui2;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, unsigned int ui0, unsigned int ui1, unsigned int ui2, unsigned int ui3 )
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_uintArray)[j] = ui0;
    (*_uintArray)[j+1] = ui1;
    (*_uintArray)[j+2] = ui2;
    (*_uintArray)[j+3] = ui3;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, bool b )
{
    if( index>=getNumElements() || !isCompatibleType(BOOL) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = b;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, bool b0, bool b1 )
{
    if( index>=getNumElements() || !isCompatibleType(BOOL_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = b0;
    (*_intArray)[j+1] = b1;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, bool b0, bool b1, bool b2 )
{
    if( index>=getNumElements() || !isCompatibleType(BOOL_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = b0;
    (*_intArray)[j+1] = b1;
    (*_intArray)[j+2] = b2;
    dirty();
    return true;
}

bool Uniform::setElement( unsigned int index, bool b0, bool b1, bool b2, bool b3 )
{
    if( index>=getNumElements() || !isCompatibleType(BOOL_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    (*_intArray)[j] = b0;
    (*_intArray)[j+1] = b1;
    (*_intArray)[j+2] = b2;
    (*_intArray)[j+3] = b3;
    dirty();
    return true;
}

///////////////////////////////////////////////////////////////////////////
// Value query for array uniforms.

bool Uniform::getElement( unsigned int index, float& f ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    f = (*_floatArray)[j];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Vec2& v2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    v2.x() = (*_floatArray)[j];
    v2.y() = (*_floatArray)[j+1];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Vec3& v3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    v3.x() = (*_floatArray)[j];
    v3.y() = (*_floatArray)[j+1];
    v3.z() = (*_floatArray)[j+2];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Vec4& v4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    v4.x() = (*_floatArray)[j];
    v4.y() = (*_floatArray)[j+1];
    v4.z() = (*_floatArray)[j+2];
    v4.w() = (*_floatArray)[j+3];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix2& m2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m2.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix3& m3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m3.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrixf& m4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m4.set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix2x3& m2x3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT2x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m2x3.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix2x4& m2x4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT2x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m2x4.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix3x2& m3x2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT3x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m3x2.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix3x4& m3x4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT3x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m3x4.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix4x2& m4x2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m4x2.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix4x3& m4x3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m4x3.base_class::set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, double& d ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    d = (*_doubleArray)[j];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Vec2d& v2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    v2.x() = (*_doubleArray)[j];
    v2.y() = (*_doubleArray)[j+1];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Vec3d& v3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    v3.x() = (*_doubleArray)[j];
    v3.y() = (*_doubleArray)[j+1];
    v3.z() = (*_doubleArray)[j+2];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Vec4d& v4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    v4.x() = (*_doubleArray)[j];
    v4.y() = (*_doubleArray)[j+1];
    v4.z() = (*_doubleArray)[j+2];
    v4.w() = (*_doubleArray)[j+3];
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix2d& m2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m2.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix3d& m3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m3.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrixd& m4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT4, FLOAT_MAT4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());

    if (_type == DOUBLE_MAT4)
      m4.set( &((*_doubleArray)[j]) );
    else // if (_type == FLOAT_MAT4) for backward compatibility only
      m4.set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix2x3d& m2x3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT2x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m2x3.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix2x4d& m2x4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT2x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m2x4.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix3x2d& m3x2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT3x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m3x2.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix3x4d& m3x4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT3x4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m3x4.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix4x2d& m4x2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT4x2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m4x2.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix4x3d& m4x3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(DOUBLE_MAT4x3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m4x3.base_class::set( &((*_doubleArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, int& i ) const
{
    if( index>=getNumElements() || !isCompatibleType(INT) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    i = (*_intArray)[j];
    return true;
}

bool Uniform::getElement( unsigned int index, int& i0, int& i1 ) const
{
    if( index>=getNumElements() || !isCompatibleType(INT_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    i0 = (*_intArray)[j];
    i1 = (*_intArray)[j+1];
    return true;
}

bool Uniform::getElement( unsigned int index, int& i0, int& i1, int& i2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(INT_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    i0 = (*_intArray)[j];
    i1 = (*_intArray)[j+1];
    i2 = (*_intArray)[j+2];
    return true;
}

bool Uniform::getElement( unsigned int index, int& i0, int& i1, int& i2, int& i3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(INT_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    i0 = (*_intArray)[j];
    i1 = (*_intArray)[j+1];
    i2 = (*_intArray)[j+2];
    i3 = (*_intArray)[j+3];
    return true;
}

bool Uniform::getElement( unsigned int index, unsigned int& ui ) const
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    ui = (*_uintArray)[j];
    return true;
}

bool Uniform::getElement( unsigned int index, unsigned int& ui0, unsigned int& ui1 ) const
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    ui0 = (*_uintArray)[j];
    ui1 = (*_uintArray)[j+1];
    return true;
}

bool Uniform::getElement( unsigned int index, unsigned int& ui0, unsigned int& ui1, unsigned int& ui2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    ui0 = (*_uintArray)[j];
    ui1 = (*_uintArray)[j+1];
    ui2 = (*_uintArray)[j+2];
    return true;
}

bool Uniform::getElement( unsigned int index, unsigned int& ui0, unsigned int& ui1, unsigned int& ui2, unsigned int& ui3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(UNSIGNED_INT_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    ui0 = (*_uintArray)[j];
    ui1 = (*_uintArray)[j+1];
    ui2 = (*_uintArray)[j+2];
    ui3 = (*_uintArray)[j+3];
    return true;
}

bool Uniform::getElement( unsigned int index, bool& b ) const
{
    if( index>=getNumElements() || !isCompatibleType(BOOL) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    b = ((*_intArray)[j] != 0);
    return true;
}

bool Uniform::getElement( unsigned int index, bool& b0, bool& b1 ) const
{
    if( index>=getNumElements() || !isCompatibleType(BOOL_VEC2) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    b0 = ((*_intArray)[j] != 0);
    b1 = ((*_intArray)[j+1] != 0);
    return true;
}

bool Uniform::getElement( unsigned int index, bool& b0, bool& b1, bool& b2 ) const
{
    if( index>=getNumElements() || !isCompatibleType(BOOL_VEC3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    b0 = ((*_intArray)[j] != 0);
    b1 = ((*_intArray)[j+1] != 0);
    b2 = ((*_intArray)[j+2] != 0);
    return true;
}

bool Uniform::getElement( unsigned int index, bool& b0, bool& b1, bool& b2, bool& b3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(BOOL_VEC4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    b0 = ((*_intArray)[j] != 0);
    b1 = ((*_intArray)[j+1] != 0);
    b2 = ((*_intArray)[j+2] != 0);
    b3 = ((*_intArray)[j+3] != 0);
    return true;
}

unsigned int Uniform::getNameID() const
{
    return _nameID;
}

///////////////////////////////////////////////////////////////////////////

void Uniform::apply(const GL2Extensions* ext, GLint location) const
{
    // OSG_NOTICE << "uniform at "<<location<<" "<<_name<< std::endl;

    GLsizei num = getNumElements();
    if( num < 1 ) return;

    switch( getGlApiType(getType()) )
    {
    case FLOAT:
        if( _floatArray.valid() ) ext->glUniform1fv( location, num, &_floatArray->front() );
        break;

    case FLOAT_VEC2:
        if( _floatArray.valid() ) ext->glUniform2fv( location, num, &_floatArray->front() );
        break;

    case FLOAT_VEC3:
        if( _floatArray.valid() ) ext->glUniform3fv( location, num, &_floatArray->front() );
        break;

    case FLOAT_VEC4:
        if( _floatArray.valid() ) ext->glUniform4fv( location, num, &_floatArray->front() );
        break;

    case FLOAT_MAT2:
        if( _floatArray.valid() ) ext->glUniformMatrix2fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT3:
        if( _floatArray.valid() ) ext->glUniformMatrix3fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT4:
        if( _floatArray.valid() ) ext->glUniformMatrix4fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT2x3:
        if( _floatArray.valid() ) ext->glUniformMatrix2x3fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT2x4:
        if( _floatArray.valid() ) ext->glUniformMatrix2x4fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT3x2:
        if( _floatArray.valid() ) ext->glUniformMatrix3x2fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT3x4:
        if( _floatArray.valid() ) ext->glUniformMatrix3x4fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT4x2:
        if( _floatArray.valid() ) ext->glUniformMatrix4x2fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case FLOAT_MAT4x3:
        if( _floatArray.valid() ) ext->glUniformMatrix4x3fv( location, num, GL_FALSE, &_floatArray->front() );
        break;

    case DOUBLE:
        if( _doubleArray.valid() ) ext->glUniform1dv( location, num, &_doubleArray->front() );
        break;

    case DOUBLE_VEC2:
        if( _doubleArray.valid() ) ext->glUniform2dv( location, num, &_doubleArray->front() );
        break;

    case DOUBLE_VEC3:
        if( _doubleArray.valid() ) ext->glUniform3dv( location, num, &_doubleArray->front() );
        break;

    case DOUBLE_VEC4:
        if( _doubleArray.valid() ) ext->glUniform4dv( location, num, &_doubleArray->front() );
        break;

    case DOUBLE_MAT2:
        if( _doubleArray.valid() ) ext->glUniformMatrix2dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT3:
        if( _doubleArray.valid() ) ext->glUniformMatrix3dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT4:
        if( _doubleArray.valid() ) ext->glUniformMatrix4dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT2x3:
        if( _doubleArray.valid() ) ext->glUniformMatrix2x3dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT2x4:
        if( _doubleArray.valid() ) ext->glUniformMatrix2x4dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT3x2:
        if( _doubleArray.valid() ) ext->glUniformMatrix3x2dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT3x4:
        if( _doubleArray.valid() ) ext->glUniformMatrix3x4dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT4x2:
        if( _doubleArray.valid() ) ext->glUniformMatrix4x2dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case DOUBLE_MAT4x3:
        if( _doubleArray.valid() ) ext->glUniformMatrix4x3dv( location, num, GL_FALSE, &_doubleArray->front() );
        break;

    case INT:
        if( _intArray.valid() ) ext->glUniform1iv( location, num, &_intArray->front() );
        break;

    case INT_VEC2:
        if( _intArray.valid() ) ext->glUniform2iv( location, num, &_intArray->front() );
        break;

    case INT_VEC3:
        if( _intArray.valid() ) ext->glUniform3iv( location, num, &_intArray->front() );
        break;

    case INT_VEC4:
        if( _intArray.valid() ) ext->glUniform4iv( location, num, &_intArray->front() );
        break;

    case UNSIGNED_INT:
        if( _uintArray.valid() ) ext->glUniform1uiv( location, num, &_uintArray->front() );
        break;

    case UNSIGNED_INT_VEC2:
        if( _uintArray.valid() ) ext->glUniform2uiv( location, num, &_uintArray->front() );
        break;

    case UNSIGNED_INT_VEC3:
        if( _uintArray.valid() ) ext->glUniform3uiv( location, num, &_uintArray->front() );
        break;

    case UNSIGNED_INT_VEC4:
        if( _uintArray.valid() ) ext->glUniform4uiv( location, num, &_uintArray->front() );
        break;

    default:
        OSG_FATAL << "how got here? " __FILE__ ":" << __LINE__ << std::endl;
        break;
    }
}

void Uniform::setUpdateCallback(Callback* uc)
{
    OSG_INFO<<"Uniform::Setting Update callbacks"<<std::endl;

    if (_updateCallback==uc) return;

    int delta = 0;
    if (_updateCallback.valid()) --delta;
    if (uc) ++delta;

    _updateCallback = uc;

    if (delta!=0)
    {
        OSG_INFO<<"Going to set Uniform parents"<<std::endl;

        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            OSG_INFO<<"   setting Uniform parent"<<std::endl;
            (*itr)->setNumChildrenRequiringUpdateTraversal((*itr)->getNumChildrenRequiringUpdateTraversal()+delta);
        }
    }
}

void Uniform::setEventCallback(Callback* ec)
{
    OSG_INFO<<"Uniform::Setting Event callbacks"<<std::endl;

    if (_eventCallback==ec) return;

    int delta = 0;
    if (_eventCallback.valid()) --delta;
    if (ec) ++delta;

    _eventCallback = ec;

    if (delta!=0)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringEventTraversal((*itr)->getNumChildrenRequiringEventTraversal()+delta);
        }
    }
}

