/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

/* file:	src/osg/Uniform.cpp
 * author:	Mike Weiblen 2005-02-20
*/

// NOTICE: This code is CLOSED during construction and/or renovation!
// It is in active development, so DO NOT yet use in application code.
// This notice will be removed when the code is open for business.
// For development plan and status see:
// http://www.openscenegraph.org/index.php?page=Community.DevelopmentWork


#include <osg/Notify>
#include <osg/Uniform>

using namespace osg;


///////////////////////////////////////////////////////////////////////////
// osg::Uniform::Value
///////////////////////////////////////////////////////////////////////////

Uniform::Value::Value( const char* name, Type type ) :
    _name(name), _type(type), _isValid(false)
{}


Uniform::Value::Value( const Value& rhs ) :
    _name(rhs._name), _type(rhs._type), _isValid(false)
{
    if( rhs._isValid ) copyData(rhs);
}


const char* Uniform::Value::getTypename( Type t )
{
    switch( t )
    {
	case FLOAT:		return "float";
	case FLOAT_VEC2:	return "vec2";
	case FLOAT_VEC3:	return "vec3";
	case FLOAT_VEC4:	return "vec4";
	case INT:		return "int";
	case INT_VEC2:		return "ivec2";
	case INT_VEC3:		return "ivec3";
	case INT_VEC4:		return "ivec4";
	case BOOL:		return "bool";
	case BOOL_VEC2:		return "bvec2";
	case BOOL_VEC3:		return "bvec3";
	case BOOL_VEC4:		return "bvec4";
	case FLOAT_MAT2:	return "mat2";
	case FLOAT_MAT3:	return "mat3";
	case FLOAT_MAT4:	return "mat4";
	case SAMPLER_1D:	return "sampler1D";
	case SAMPLER_2D:	return "sampler2D";
	case SAMPLER_3D:	return "sampler3D";
	case SAMPLER_CUBE:	return "samplerCube";
	case SAMPLER_1D_SHADOW:	return "sampler1DShadow";
	case SAMPLER_2D_SHADOW:	return "sampler2DShadow";
	default:		return "UNDEFINED";
    }
}


int Uniform::Value::compare(const Value& rhs) const
{
    if( this == &rhs ) return 0;

    if( _type < rhs._type ) return -1;
    if( rhs._type < _type ) return 1;

    // consider invalid values "less than" valid values
    if( !_isValid && rhs._isValid ) return -1;
    if( _isValid && !rhs._isValid ) return 1;

    if( _name < rhs._name ) return -1;
    if( rhs._name < _name ) return 1;

    if( isValid() ) return compareData( rhs );

    return 0;
}



void Uniform::Value:: set( float f )
{
    _data.f1 = f;
    _isValid = true;
}

void Uniform::Value:: set( const osg::Vec2& v2 )
{
    _data.f2[0] = v2.x();
    _data.f2[1] = v2.y();
    _isValid = true;
}

void Uniform::Value:: set( const osg::Vec3& v3 )
{
    _data.f3[0] = v3.x();
    _data.f3[1] = v3.y();
    _data.f3[2] = v3.z();
    _isValid = true;
}

void Uniform::Value:: set( const osg::Vec4& v4 )
{
    _data.f4[0] = v4.x();
    _data.f4[1] = v4.y();
    _data.f4[2] = v4.z();
    _data.f4[3] = v4.w();
    _isValid = true;
}

//TODO void Uniform::Value:: set( const osg::Matrix2& m2 )

//TODO void Uniform::Value:: set( const osg::Matrix3& m3 )

void Uniform::Value:: set( const osg::Matrix& m4 )
{	// TODO verify if needs to be transposed
    int n = 0;
    for(int row=0; row<4; ++row)
    {
	for(int col=0; col<4; ++col)
	{
	    _data.f16[n++] = m4(row,col);
	}
    }
    _isValid = true;
}

void Uniform::Value:: set( int i )
{
    _data.i1 = i;
    _isValid = true;
}

//TODO void Uniform::Value:: set( int i0, int i1 )

//TODO void Uniform::Value:: set( int i0, int i1, int i2 )

//TODO void Uniform::Value:: set( int i0, int i1, int i2, int i3 )

//TODO void Uniform::Value:: set( bool b )

//TODO void Uniform::Value:: set( bool b0, bool b1 )

//TODO void Uniform::Value:: set( bool b0, bool b1, bool b2 )

//TODO void Uniform::Value:: set( bool b0, bool b1, bool b2, bool b3 )



int Uniform::Value::compareData(const Value& rhs) const
{
    // caller is responsible for ensuring that
    // _type==rhs._type && _isValid && rhs._isValid

    switch( getType() )
    {
	case FLOAT:
	{
	    if( _data.f1 < rhs._data.f1 ) return -1;
	    if( _data.f1 > rhs._data.f1 ) return  1;
	    return 0;
	}

	case FLOAT_VEC2:
	{
	    if( _data.f2[0] < rhs._data.f2[0] ) return -1;
	    if( _data.f2[0] > rhs._data.f2[0] ) return  1;
	    if( _data.f2[1] < rhs._data.f2[1] ) return -1;
	    if( _data.f2[1] > rhs._data.f2[1] ) return  1;
	    return 0;
	}

	case FLOAT_VEC3:
	{
	    if( _data.f3[0] < rhs._data.f3[0] ) return -1;
	    if( _data.f3[0] > rhs._data.f3[0] ) return  1;
	    if( _data.f3[1] < rhs._data.f3[1] ) return -1;
	    if( _data.f3[1] > rhs._data.f3[1] ) return  1;
	    if( _data.f3[2] < rhs._data.f3[2] ) return -1;
	    if( _data.f3[2] > rhs._data.f3[2] ) return  1;
	    return 0;
	}

	case FLOAT_VEC4:
	case FLOAT_MAT2:
	{
	    if( _data.f4[0] < rhs._data.f4[0] ) return -1;
	    if( _data.f4[0] > rhs._data.f4[0] ) return  1;
	    if( _data.f4[1] < rhs._data.f4[1] ) return -1;
	    if( _data.f4[1] > rhs._data.f4[1] ) return  1;
	    if( _data.f4[2] < rhs._data.f4[2] ) return -1;
	    if( _data.f4[2] > rhs._data.f4[2] ) return  1;
	    if( _data.f4[3] < rhs._data.f4[3] ) return -1;
	    if( _data.f4[3] > rhs._data.f4[3] ) return  1;
	    return 0;
	}

	case FLOAT_MAT3:	return memcmp(_data.f9, rhs._data.f9, sizeof(_data.f9));

	case FLOAT_MAT4:	return memcmp(_data.f16, rhs._data.f16, sizeof(_data.f16));

	case INT:
	case BOOL:
	case SAMPLER_1D:
	case SAMPLER_2D:
	case SAMPLER_3D:
	case SAMPLER_CUBE:
	case SAMPLER_1D_SHADOW:
	case SAMPLER_2D_SHADOW:
	{
	    if( _data.i1 < rhs._data.i1 ) return -1;
	    if( _data.i1 > rhs._data.i1 ) return  1;
	    return 0;
	}

	case INT_VEC2:
	case BOOL_VEC2:
	{
	    if( _data.i2[0] < rhs._data.i2[0] ) return -1;
	    if( _data.i2[0] > rhs._data.i2[0] ) return  1;
	    if( _data.i2[1] < rhs._data.i2[1] ) return -1;
	    if( _data.i2[1] > rhs._data.i2[1] ) return  1;
	    return 0;
	}

	case INT_VEC3:
	case BOOL_VEC3:
	{
	    if( _data.i3[0] < rhs._data.i3[0] ) return -1;
	    if( _data.i3[0] > rhs._data.i3[0] ) return  1;
	    if( _data.i3[1] < rhs._data.i3[1] ) return -1;
	    if( _data.i3[1] > rhs._data.i3[1] ) return  1;
	    if( _data.i3[2] < rhs._data.i3[2] ) return -1;
	    if( _data.i3[2] > rhs._data.i3[2] ) return  1;
	    return 0;
	}

	case INT_VEC4:
	case BOOL_VEC4:
	{
	    if( _data.i4[0] < rhs._data.i4[0] ) return -1;
	    if( _data.i4[0] > rhs._data.i4[0] ) return  1;
	    if( _data.i4[1] < rhs._data.i4[1] ) return -1;
	    if( _data.i4[1] > rhs._data.i4[1] ) return  1;
	    if( _data.i4[2] < rhs._data.i4[2] ) return -1;
	    if( _data.i4[2] > rhs._data.i4[2] ) return  1;
	    if( _data.i4[3] < rhs._data.i4[3] ) return -1;
	    if( _data.i4[3] > rhs._data.i4[3] ) return  1;
	    return 0;
	}

	default:
	    osg::notify(osg::INFO) << "how got here?" << std::endl;
	    return 0;
    }
}


void Uniform::Value::copyData(const Value& rhs)
{
    // caller is responsible for ensuring that
    // _type==rhs._type && rhs._isValid

    _isValid = true;
    switch( getType() )
    {
	case FLOAT:
	    _data.f1 = rhs._data.f1;
	    break;

	case FLOAT_VEC2:
	    _data.f2[0] = rhs._data.f2[0];
	    _data.f2[1] = rhs._data.f2[1];
	    break;

	case FLOAT_VEC3:
	    _data.f3[0] = rhs._data.f3[0];
	    _data.f3[1] = rhs._data.f3[1];
	    _data.f3[2] = rhs._data.f3[2];
	    break;

	case FLOAT_VEC4:
	case FLOAT_MAT2:
	    _data.f4[0] = rhs._data.f4[0];
	    _data.f4[1] = rhs._data.f4[1];
	    _data.f4[2] = rhs._data.f4[2];
	    _data.f4[3] = rhs._data.f4[3];
	    break;

	case FLOAT_MAT3:
	    for(int i=0;i<9;++i) _data.f9[i]=rhs._data.f9[i];
	    break;

	case FLOAT_MAT4:
	    for(int i=0;i<16;++i) _data.f16[i]=rhs._data.f16[i];
	    break;

	case INT:
	case BOOL:
	case SAMPLER_1D:
	case SAMPLER_2D:
	case SAMPLER_3D:
	case SAMPLER_CUBE:
	case SAMPLER_1D_SHADOW:
	case SAMPLER_2D_SHADOW:
	    _data.i1 = rhs._data.i1;
	    break;

	case INT_VEC2:
	case BOOL_VEC2:
	    _data.i2[0] = rhs._data.i2[0];
	    _data.i2[1] = rhs._data.i2[1];
	    break;

	case INT_VEC3:
	case BOOL_VEC3:
	    _data.i3[0] = rhs._data.i3[0];
	    _data.i3[1] = rhs._data.i3[1];
	    _data.i3[2] = rhs._data.i3[2];
	    break;

	case INT_VEC4:
	case BOOL_VEC4:
	    _data.i4[0] = rhs._data.i4[0];
	    _data.i4[1] = rhs._data.i4[1];
	    _data.i4[2] = rhs._data.i4[2];
	    _data.i4[3] = rhs._data.i4[3];
	    break;

	default:
	    osg::notify(osg::INFO) << "how got here?" << std::endl;
	    break;
    }
}


///////////////////////////////////////////////////////////////////////////
// osg::Uniform
///////////////////////////////////////////////////////////////////////////

Uniform::Uniform() :
    _value( "", Value::UNDEFINED )
{
    // do not use this constructor in application code!
    // it exists only because StateAttribute _requires_ a trivial default
    // constructor, but that is concept is meaningless for Uniform.
}


Uniform::Uniform( const char* name, Value::Type type ) :
    _value( name, type )
{}


Uniform::Uniform( const char* name, float f ) :
    _value( name, Value::FLOAT )
{
    _value.set( f );
}

Uniform::Uniform( const char* name, const osg::Vec2& v2 ) :
    _value( name, Value::FLOAT_VEC2 )
{
    _value.set( v2 );
}

Uniform::Uniform( const char* name, const osg::Vec3& v3 ) :
    _value( name, Value::FLOAT_VEC3 )
{
    _value.set( v3 );
}

Uniform::Uniform( const char* name, const osg::Vec4& v4 ) :
    _value( name, Value::FLOAT_VEC4 )
{
    _value.set( v4 );
}

//TODO Uniform::Uniform( const char* name, const osg::Matrix2& m2 )

//TODO Uniform::Uniform( const char* name, const osg::Matrix3& m3 )

Uniform::Uniform( const char* name, const osg::Matrix& m4 ) :
    _value( name, Value::FLOAT_MAT4 )
{
    _value.set( m4 );
}

Uniform::Uniform( const char* name, int i ) :
    _value( name, Value::INT )
{
    _value.set( i );
}

//TODO Uniform::Uniform( const char* name, int i0, int i1 )

//TODO Uniform::Uniform( const char* name, int i0, int i1, int i2 )

//TODO Uniform::Uniform( const char* name, int i0, int i1, int i2, int i3 )

//TODO Uniform::Uniform( const char* name, bool b )

//TODO Uniform::Uniform( const char* name, bool b0, bool b1 )

//TODO Uniform::Uniform( const char* name, bool b0, bool b1, bool b2 )

//TODO Uniform::Uniform( const char* name, bool b0, bool b1, bool b2, bool b3 )


Uniform::Uniform( const Uniform& gu, const CopyOp& copyop ) :
    Object(gu,copyop),
    _value( gu._value )
{
}


bool Uniform::set( float f )
{
    if( ! isCompatibleType( Value::FLOAT ) ) return false;
    _value.set( f );
    return true;
}

bool Uniform::set( const osg::Vec2& v2 )
{
    if( ! isCompatibleType( Value::FLOAT_VEC2 ) ) return false;
    _value.set( v2 );
    return true;
}

bool Uniform::set( const osg::Vec3& v3 )
{
    if( ! isCompatibleType( Value::FLOAT_VEC3 ) ) return false;
    _value.set( v3 );
    return true;
}

bool Uniform::set( const osg::Vec4& v4 )
{
    if( ! isCompatibleType( Value::FLOAT_VEC4 ) ) return false;
    _value.set( v4 );
    return true;
}

//TODO bool Uniform::set( const osg::Matrix2& m2 )

//TODO bool Uniform::set( const osg::Matrix3& m3 )

bool Uniform::set( const osg::Matrix& m4 )
{
    if( ! isCompatibleType( Value::FLOAT_MAT4 ) ) return false;
    _value.set( m4 );
    return true;
}

bool Uniform::set( int i )
{
    if( ! isCompatibleType( Value::INT ) ) return false;
    _value.set( i );
    return true;
}

//TODO bool Uniform::set( int i0, int i1 )

//TODO bool Uniform::set( int i0, int i1, int i2 )

//TODO bool Uniform::set( int i0, int i1, int i2, int i3 )

//TODO bool Uniform::set( bool b )

//TODO bool Uniform::set( bool b0, bool b1 ); 

//TODO bool Uniform::set( bool b0, bool b1, bool b2 )

//TODO bool Uniform::set( bool b0, bool b1, bool b2, bool b3 )


bool Uniform::isCompatibleType( Value::Type t ) const
{
    if( t == _value.getType() ) return true;

    osg::notify(osg::WARN) <<
	"Cannot assign " << _value.getTypename(t) <<
	" to Uniform \"" << _value.getName() <<
	"\" of type " << _value.getTypename( _value.getType() ) <<
	std::endl;
    return false;
}


/*EOF*/
