/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
 * applications, as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* file:   src/osg/Uniform.cpp
 * author: Mike Weiblen 2006-05-15
*/

#include <osg/Notify>
#include <osg/Uniform>
#include <osg/Program>
#include <osg/StateSet>

#include <algorithm>

using namespace osg;

///////////////////////////////////////////////////////////////////////////
// osg::Uniform
///////////////////////////////////////////////////////////////////////////

Uniform::Uniform() :
    _type(UNDEFINED), _numElements(0), _modifiedCount(0)
{
    setDataVariance(STATIC);
}


Uniform::Uniform( Type type, const std::string& name, int numElements ) :
    _type(type), _numElements(0), _modifiedCount(0)
{
    setName(name);
    setNumElements(numElements);
    setDataVariance(STATIC);
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
    osg::notify(osg::INFO)<<"Uniform Adding parent"<<std::endl;

    _parents.push_back(object);
}

void Uniform::removeParent(osg::StateSet* object)
{
    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),object);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}

bool Uniform::setType( Type t )
{
    if (_type==t) return true;

    if( _type != UNDEFINED )
    {
        osg::notify(osg::WARN) << "cannot change Uniform type" << std::endl;
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
        osg::notify(osg::WARN) << "cannot change Uniform name" << std::endl;
        return;
    }
    _name = name;
}

void Uniform::setNumElements( unsigned int numElements )
{
    if( numElements < 1 )
    {
        osg::notify(osg::WARN) << "Uniform numElements < 1 is invalid" << std::endl;
        return;
    }

    if (numElements == _numElements) return;

    if( _numElements>0 )
    {
        osg::notify(osg::WARN) << "Warning: Uniform::setNumElements() cannot change Uniform numElements, size already fixed." << std::endl;
        return;
    }

    _numElements = numElements;
    allocateDataArray();
}

void Uniform::allocateDataArray()
{
    // if one array is already allocated, the job is done.
    if( _floatArray.valid() != _intArray.valid() ) return;

    // array cannot be created until _type and _numElements are specified
    int arrayNumElements = getInternalArrayNumElements();
    if( arrayNumElements )
    {
        switch( getInternalArrayType(getType()) )
        {
            case GL_FLOAT:
                _floatArray = new FloatArray(arrayNumElements);
                _intArray = 0;
                return;

            case GL_INT:
                _intArray = new IntArray(arrayNumElements);
                _floatArray = 0;
                return;

            default:
                break;
        }
    }
    _floatArray = 0;
    _intArray = 0;
}

bool Uniform::setArray( FloatArray* array )
{
    if( !array ) return false;

    // incoming array must match configuration of the Uniform
    if( getInternalArrayType(getType())!=GL_FLOAT || getInternalArrayNumElements()!=array->getNumElements() )
    {
        osg::notify(osg::WARN) << "Uniform::setArray : incompatible array" << std::endl;
        return false;
    }

    _floatArray = array;
    _intArray = 0;
    dirty();
    return true;
}

bool Uniform::setArray( IntArray* array )
{
    if( !array ) return false;

    // incoming array must match configuration of the Uniform
    if( getInternalArrayType(getType())!=GL_INT || getInternalArrayNumElements()!=array->getNumElements() )
    {
        osg::notify(osg::WARN) << "Uniform::setArray : incompatible array" << std::endl;
        return false;
    }

    _intArray = array;
    _floatArray = 0;
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

    if( _intArray.valid() )
    {
        if( ! rhs._intArray ) return 1;
        if( _intArray == rhs._intArray ) return 0;
        return memcmp( _intArray->getDataPointer(), rhs._intArray->getDataPointer(),
            _intArray->getTotalDataSize() );
    }
    return -1;  // how got here?
}

void Uniform::copyData(const Uniform& rhs)
{
    // caller must ensure that _type==rhs._type
    _numElements = rhs._numElements;
    if( _floatArray.valid() && rhs._floatArray.valid() ) *_floatArray = *rhs._floatArray;
    if( _intArray.valid() && rhs._intArray.valid() )     *_intArray = *rhs._intArray;
    dirty();
}

bool Uniform::isCompatibleType( Type t ) const
{
    if( (t==UNDEFINED) || (getType()==UNDEFINED) ) return false;
    if( t == getType() ) return true;
    if( getGlApiType(t) == getGlApiType(getType()) ) return true;

    osg::notify(osg::WARN)
        << "Cannot assign between Uniform types " << getTypename(t)
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
    case FLOAT:             return "float";
    case FLOAT_VEC2:        return "vec2";
    case FLOAT_VEC3:        return "vec3";
    case FLOAT_VEC4:        return "vec4";
    case INT:               return "int";
    case INT_VEC2:          return "ivec2";
    case INT_VEC3:          return "ivec3";
    case INT_VEC4:          return "ivec4";
    case BOOL:              return "bool";
    case BOOL_VEC2:         return "bvec2";
    case BOOL_VEC3:         return "bvec3";
    case BOOL_VEC4:         return "bvec4";
    case FLOAT_MAT2:        return "mat2";
    case FLOAT_MAT3:        return "mat3";
    case FLOAT_MAT4:        return "mat4";
    case SAMPLER_1D:        return "sampler1D";
    case SAMPLER_2D:        return "sampler2D";
    case SAMPLER_3D:        return "sampler3D";
    case SAMPLER_CUBE:      return "samplerCube";
    case SAMPLER_1D_SHADOW: return "sampler1DShadow";
    case SAMPLER_2D_SHADOW: return "sampler2DShadow";
    default:                return "UNDEFINED";
    }
}

int Uniform::getTypeNumComponents( Type t )
{
    switch( t )
    {
    case FLOAT:
    case INT:
    case BOOL:
    case SAMPLER_1D:
    case SAMPLER_2D:
    case SAMPLER_3D:
    case SAMPLER_CUBE:
    case SAMPLER_1D_SHADOW:
    case SAMPLER_2D_SHADOW:
        return 1;

    case FLOAT_VEC2:
    case INT_VEC2:
    case BOOL_VEC2:
        return 2;

    case FLOAT_VEC3:
    case INT_VEC3:
    case BOOL_VEC3:
        return 3;

    case FLOAT_VEC4:
    case FLOAT_MAT2:
    case INT_VEC4:
    case BOOL_VEC4:
        return 4;

    case FLOAT_MAT3:
        return 9;

    case FLOAT_MAT4:
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
    if( tname == "int" )             return INT;
    if( tname == "ivec2" )           return INT_VEC2;
    if( tname == "ivec3" )           return INT_VEC3;
    if( tname == "ivec4" )           return INT_VEC4;
    if( tname == "bool" )            return BOOL;
    if( tname == "bvec2" )           return BOOL_VEC2;
    if( tname == "bvec3" )           return BOOL_VEC3;
    if( tname == "bvec4" )           return BOOL_VEC4;
    if( tname == "mat2" )            return FLOAT_MAT2;
    if( tname == "mat3" )            return FLOAT_MAT3;
    if( tname == "mat4" )            return FLOAT_MAT4;
    if( tname == "sampler1D" )       return SAMPLER_1D;
    if( tname == "sampler2D" )       return SAMPLER_2D;
    if( tname == "sampler3D" )       return SAMPLER_3D;
    if( tname == "samplerCube" )     return SAMPLER_CUBE;
    if( tname == "sampler1DShadow" ) return SAMPLER_1D_SHADOW;
    if( tname == "sampler2DShadow" ) return SAMPLER_2D_SHADOW;
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
        return GL_FLOAT;

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
        return GL_INT;

    default:
        return 0;
    }
}


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

Uniform::Uniform( const char* name, const osg::Matrixd& m4 ) :
    _type(FLOAT_MAT4), _numElements(1), _modifiedCount(0)
{
    setName(name);
    allocateDataArray();
    set( m4 );
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

bool Uniform::set( const osg::Matrixd& m4 )
{
    if( getNumElements() == 0 ) setNumElements(1);
    return isScalar() ? setElement(0,m4) : false;
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

bool Uniform::get( osg::Matrixd& m4 ) const
{
    return isScalar() ? getElement(0,m4) : false;
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

bool Uniform::setElement( unsigned int index, const osg::Matrixd& m4 )
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    const Matrixd::value_type* p = m4.ptr();
    for( int i = 0; i < 16; ++i ) (*_floatArray)[j+i] = static_cast<float>(p[i]);
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
    m2.set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrix3& m3 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT3) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m3.set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrixf& m4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m4.set( &((*_floatArray)[j]) );
    return true;
}

bool Uniform::getElement( unsigned int index, osg::Matrixd& m4 ) const
{
    if( index>=getNumElements() || !isCompatibleType(FLOAT_MAT4) ) return false;
    unsigned int j = index * getTypeNumComponents(getType());
    m4.set( &((*_floatArray)[j]) );
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

///////////////////////////////////////////////////////////////////////////

void Uniform::apply(const GL2Extensions* ext, GLint location) const
{
    // osg::notify(osg::NOTICE) << "uniform at "<<location<<" "<<_name<< std::endl;

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

    default:
        osg::notify(osg::FATAL) << "how got here? " __FILE__ ":" << __LINE__ << std::endl;
        break;
    }
}

void Uniform::setUpdateCallback(Callback* uc)
{
    osg::notify(osg::INFO)<<"Uniform::Setting Update callbacks"<<std::endl;

    if (_updateCallback==uc) return;
    
    int delta = 0;
    if (_updateCallback.valid()) --delta;
    if (uc) ++delta;

    _updateCallback = uc;
    
    if (delta!=0)
    {
        osg::notify(osg::INFO)<<"Going to set Uniform parents"<<std::endl;

        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            osg::notify(osg::INFO)<<"   setting Uniform parent"<<std::endl;
            (*itr)->setNumChildrenRequiringUpdateTraversal((*itr)->getNumChildrenRequiringUpdateTraversal()+delta);
        }
    }
}

void Uniform::setEventCallback(Callback* ec)
{
    osg::notify(osg::INFO)<<"Uniform::Setting Event callbacks"<<std::endl;

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


/*EOF*/

