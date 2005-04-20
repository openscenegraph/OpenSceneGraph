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

/* file:    src/osg/Uniform.cpp
 * author:    Mike Weiblen 2005-04-07
*/

#include <osg/Notify>
#include <osg/Uniform>
#include <osg/Program>

using namespace osg;

///////////////////////////////////////////////////////////////////////////
// osg::Uniform
///////////////////////////////////////////////////////////////////////////

Uniform::Uniform() :
    _name(""), _type(UNDEFINED)
{
}


Uniform::Uniform( const char* name, Type type ) :
    _name(name), _type(type)
{
    switch( _type )
    {
    case FLOAT:        set( 0.0f ); break;
    case FLOAT_VEC2:    set( osg::Vec2() ); break;
    case FLOAT_VEC3:    set( osg::Vec3() ); break;
    case FLOAT_VEC4:    set( osg::Vec4() ); break;
    case INT:        set( 0 ); break;
    case INT_VEC2:        set( 0, 0 ); break;
    case INT_VEC3:        set( 0, 0, 0 ); break;
    case INT_VEC4:        set( 0, 0, 0, 0 ); break;
    case BOOL:        set( false ); break;
    case BOOL_VEC2:        set( false, false ); break;
    case BOOL_VEC3:        set( false, false, false ); break;
    case BOOL_VEC4:        set( false, false, false, false ); break;
    // TODO case FLOAT_MAT2:    
    // TODO case FLOAT_MAT3:    
    case FLOAT_MAT4:    set( osg::Matrix() ); break;
    case SAMPLER_1D:    set( 0 ); break;
    case SAMPLER_2D:    set( 0 ); break;
    case SAMPLER_3D:    set( 0 ); break;
    case SAMPLER_CUBE:    set( 0 ); break;
    case SAMPLER_1D_SHADOW:    set( 0 ); break;
    case SAMPLER_2D_SHADOW:    set( 0 ); break;
    default:
        osg::notify(osg::WARN) << "UNDEFINED Uniform type" << std::endl;
        break;
    }
}

Uniform::Uniform( const Uniform& rhs, const CopyOp& copyop ) :
    Object(rhs,copyop), _name(rhs._name), _type(rhs._type)
{
    copyData( rhs );
}

bool Uniform::setType( Type t )
{
    if( _type != UNDEFINED )
    {
    osg::notify(osg::WARN) << "cannot change Uniform type" << std::endl;
    return false;
    }
    _type = t;
    return true;
}

bool Uniform::setName( const std::string& name )
{
    if( _name != "" )
    {
    osg::notify(osg::WARN) << "cannot change Uniform name" << std::endl;
    return false;
    }
    _name = name;
    return true;
}


///////////////////////////////////////////////////////////////////////////

int Uniform::compare(const Uniform& rhs) const
{
    if( this == &rhs ) return 0;

    if( _type < rhs._type ) return -1;
    if( rhs._type < _type ) return 1;

    if( _name < rhs._name ) return -1;
    if( rhs._name < _name ) return 1;

    return compareData( rhs );
}

int Uniform::compareData(const Uniform& rhs) const
{
    // caller must ensure that _type==rhs._type

    switch( repType(getType()) )
    {
    case FLOAT:
        if( _data.f1 < rhs._data.f1 ) return -1;
        if( _data.f1 > rhs._data.f1 ) return  1;
        return 0;

    case FLOAT_VEC2:
        if( _data.f2[0] < rhs._data.f2[0] ) return -1;
        if( _data.f2[0] > rhs._data.f2[0] ) return  1;
        if( _data.f2[1] < rhs._data.f2[1] ) return -1;
        if( _data.f2[1] > rhs._data.f2[1] ) return  1;
        return 0;

    case FLOAT_VEC3:
        if( _data.f3[0] < rhs._data.f3[0] ) return -1;
        if( _data.f3[0] > rhs._data.f3[0] ) return  1;
        if( _data.f3[1] < rhs._data.f3[1] ) return -1;
        if( _data.f3[1] > rhs._data.f3[1] ) return  1;
        if( _data.f3[2] < rhs._data.f3[2] ) return -1;
        if( _data.f3[2] > rhs._data.f3[2] ) return  1;
        return 0;

    case FLOAT_VEC4:
    case FLOAT_MAT2:
        if( _data.f4[0] < rhs._data.f4[0] ) return -1;
        if( _data.f4[0] > rhs._data.f4[0] ) return  1;
        if( _data.f4[1] < rhs._data.f4[1] ) return -1;
        if( _data.f4[1] > rhs._data.f4[1] ) return  1;
        if( _data.f4[2] < rhs._data.f4[2] ) return -1;
        if( _data.f4[2] > rhs._data.f4[2] ) return  1;
        if( _data.f4[3] < rhs._data.f4[3] ) return -1;
        if( _data.f4[3] > rhs._data.f4[3] ) return  1;
        return 0;

    case FLOAT_MAT3:
        return memcmp(_data.f9, rhs._data.f9, sizeof(_data.f9));

    case FLOAT_MAT4:
            return memcmp(_data.f16, rhs._data.f16, sizeof(_data.f16));

    case INT:
        if( _data.i1 < rhs._data.i1 ) return -1;
        if( _data.i1 > rhs._data.i1 ) return  1;
        return 0;

    case INT_VEC2:
        if( _data.i2[0] < rhs._data.i2[0] ) return -1;
        if( _data.i2[0] > rhs._data.i2[0] ) return  1;
        if( _data.i2[1] < rhs._data.i2[1] ) return -1;
        if( _data.i2[1] > rhs._data.i2[1] ) return  1;
        return 0;

    case INT_VEC3:
        if( _data.i3[0] < rhs._data.i3[0] ) return -1;
        if( _data.i3[0] > rhs._data.i3[0] ) return  1;
        if( _data.i3[1] < rhs._data.i3[1] ) return -1;
        if( _data.i3[1] > rhs._data.i3[1] ) return  1;
        if( _data.i3[2] < rhs._data.i3[2] ) return -1;
        if( _data.i3[2] > rhs._data.i3[2] ) return  1;
        return 0;

    case INT_VEC4:
        if( _data.i4[0] < rhs._data.i4[0] ) return -1;
        if( _data.i4[0] > rhs._data.i4[0] ) return  1;
        if( _data.i4[1] < rhs._data.i4[1] ) return -1;
        if( _data.i4[1] > rhs._data.i4[1] ) return  1;
        if( _data.i4[2] < rhs._data.i4[2] ) return -1;
        if( _data.i4[2] > rhs._data.i4[2] ) return  1;
        if( _data.i4[3] < rhs._data.i4[3] ) return -1;
        if( _data.i4[3] > rhs._data.i4[3] ) return  1;
        return 0;

    default:
        osg::notify(osg::WARN) << "cannot compare UNDEFINED Uniform type" << std::endl;
        return 0;
    }
}

void Uniform::copyData(const Uniform& rhs)
{
    // caller must ensure that _type==rhs._type

    int i;
    switch( repType(getType()) )
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
        for(i=0;i<9;++i) _data.f9[i]=rhs._data.f9[i];
        break;

    case FLOAT_MAT4:
        for(i=0;i<16;++i) _data.f16[i]=rhs._data.f16[i];
        break;

    case INT:
        _data.i1 = rhs._data.i1;
        break;

    case INT_VEC2:
        _data.i2[0] = rhs._data.i2[0];
        _data.i2[1] = rhs._data.i2[1];
        break;

    case INT_VEC3:
        _data.i3[0] = rhs._data.i3[0];
        _data.i3[1] = rhs._data.i3[1];
        _data.i3[2] = rhs._data.i3[2];
        break;

    case INT_VEC4:
        _data.i4[0] = rhs._data.i4[0];
        _data.i4[1] = rhs._data.i4[1];
        _data.i4[2] = rhs._data.i4[2];
        _data.i4[3] = rhs._data.i4[3];
        break;

    default:
        osg::notify(osg::WARN) << "cannot copy UNDEFINED Uniform type" << std::endl;
        break;
    }
}

bool Uniform::isCompatibleType( Type t ) const
{
    if( (t==UNDEFINED) || (getType()==UNDEFINED) ) return false;
    if( t == getType() ) return true;
    if( repType(t) == repType(getType()) ) return true;

    osg::notify(osg::WARN)
    << "Cannot assign between Uniform types " << getTypename(t)
    << " and " << getTypename(getType()) << std::endl;
    return false;
}

///////////////////////////////////////////////////////////////////////////
// static methods

const char* Uniform::getTypename( Type t )
{
    switch( t )
    {
    case FLOAT:        return "float";
    case FLOAT_VEC2:    return "vec2";
    case FLOAT_VEC3:    return "vec3";
    case FLOAT_VEC4:    return "vec4";
    case INT:        return "int";
    case INT_VEC2:        return "ivec2";
    case INT_VEC3:        return "ivec3";
    case INT_VEC4:        return "ivec4";
    case BOOL:        return "bool";
    case BOOL_VEC2:        return "bvec2";
    case BOOL_VEC3:        return "bvec3";
    case BOOL_VEC4:        return "bvec4";
    case FLOAT_MAT2:    return "mat2";
    case FLOAT_MAT3:    return "mat3";
    case FLOAT_MAT4:    return "mat4";
    case SAMPLER_1D:    return "sampler1D";
    case SAMPLER_2D:    return "sampler2D";
    case SAMPLER_3D:    return "sampler3D";
    case SAMPLER_CUBE:    return "samplerCube";
    case SAMPLER_1D_SHADOW:    return "sampler1DShadow";
    case SAMPLER_2D_SHADOW:    return "sampler2DShadow";
    default:        return "UNDEFINED";
    }
}

Uniform::Type Uniform::getTypeId( const std::string& tname )
{
    if( tname == "float" )    return FLOAT;
    if( tname == "vec2" )    return FLOAT_VEC2;
    if( tname == "vec3" )    return FLOAT_VEC3;
    if( tname == "vec4" )    return FLOAT_VEC4;
    if( tname == "int" )    return INT;
    if( tname == "ivec2" )    return INT_VEC2;
    if( tname == "ivec3" )    return INT_VEC3;
    if( tname == "ivec4" )    return INT_VEC4;
    if( tname == "bool" )    return BOOL;
    if( tname == "bvec2" )    return BOOL_VEC2;
    if( tname == "bvec3" )    return BOOL_VEC3;
    if( tname == "bvec4" )    return BOOL_VEC4;
    if( tname == "mat2" )    return FLOAT_MAT2;
    if( tname == "mat3" )    return FLOAT_MAT3;
    if( tname == "mat4" )    return FLOAT_MAT4;
    if( tname == "sampler1D" )    return SAMPLER_1D;
    if( tname == "sampler2D" )    return SAMPLER_2D;
    if( tname == "sampler3D" )    return SAMPLER_3D;
    if( tname == "samplerCube" )    return SAMPLER_CUBE;
    if( tname == "sampler1DShadow" )    return SAMPLER_1D_SHADOW;
    if( tname == "sampler2DShadow" )    return SAMPLER_2D_SHADOW;
    return UNDEFINED;
}

Uniform::Type Uniform::repType( Type t )
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


///////////////////////////////////////////////////////////////////////////
// value constructors

Uniform::Uniform( const char* name, float f ) :
    _name(name), _type(FLOAT)
{
    set( f );
}

Uniform::Uniform( const char* name, const osg::Vec2& v2 ) :
    _name(name), _type(FLOAT_VEC2)
{
    set( v2 );
}

Uniform::Uniform( const char* name, const osg::Vec3& v3 ) :
    _name(name), _type(FLOAT_VEC3)
{
    set( v3 );
}

Uniform::Uniform( const char* name, const osg::Vec4& v4 ) :
    _name(name), _type(FLOAT_VEC4)
{
    set( v4 );
}

//Uniform::Uniform( const char* name, const osg::Matrix2& m2 )

//Uniform::Uniform( const char* name, const osg::Matrix3& m3 )

Uniform::Uniform( const char* name, const osg::Matrix& m4 ) :
    _name(name), _type(FLOAT_MAT4)
{
    set( m4 );
}

Uniform::Uniform( const char* name, int i ) :
    _name(name), _type(INT)
{
    set( i );
}

Uniform::Uniform( const char* name, int i0, int i1 ) :
    _name(name), _type(INT_VEC2)
{
    set( i0, i1 );
}

Uniform::Uniform( const char* name, int i0, int i1, int i2 ) :
    _name(name), _type(INT_VEC3)
{
    set( i0, i1, i2 );
}

Uniform::Uniform( const char* name, int i0, int i1, int i2, int i3 ) :
    _name(name), _type(INT_VEC4)
{
    set( i0, i1, i2, i3 );
}

Uniform::Uniform( const char* name, bool b ) :
    _name(name), _type(BOOL)
{
    set( b );
}

Uniform::Uniform( const char* name, bool b0, bool b1 ) :
    _name(name), _type(BOOL_VEC2)
{
    set( b0, b1 );
}

Uniform::Uniform( const char* name, bool b0, bool b1, bool b2 ) :
    _name(name), _type(BOOL_VEC3)
{
    set( b0, b1, b2 );
}

Uniform::Uniform( const char* name, bool b0, bool b1, bool b2, bool b3 ) :
    _name(name), _type(BOOL_VEC4)
{
    set( b0, b1, b2, b3 );
}

///////////////////////////////////////////////////////////////////////////
// value assignment

bool Uniform::set( float f )
{
    if( ! isCompatibleType(FLOAT) ) return false;
    _data.f1 = f;
    return true;
}

bool Uniform::set( const osg::Vec2& v2 )
{
    if( ! isCompatibleType(FLOAT_VEC2) ) return false;
    _data.f2[0] = v2.x();
    _data.f2[1] = v2.y();
    return true;
}

bool Uniform::set( const osg::Vec3& v3 )
{
    if( ! isCompatibleType(FLOAT_VEC3) ) return false;
    _data.f3[0] = v3.x();
    _data.f3[1] = v3.y();
    _data.f3[2] = v3.z();
    return true;
}

bool Uniform::set( const osg::Vec4& v4 )
{
    if( ! isCompatibleType(FLOAT_VEC4) ) return false;
    _data.f4[0] = v4.x();
    _data.f4[1] = v4.y();
    _data.f4[2] = v4.z();
    _data.f4[3] = v4.w();
    return true;
}

//TODO bool Uniform::set( const osg::Matrix2& m2 )

//TODO bool Uniform::set( const osg::Matrix3& m3 )

bool Uniform::set( const osg::Matrix& m4 )
{
    if( ! isCompatibleType(FLOAT_MAT4) ) return false;
    int n = 0;
    for(int row=0; row<4; ++row)
    {
    for(int col=0; col<4; ++col)
    {
        _data.f16[n++] = m4(row,col);
    }
    }
    return true;
}

bool Uniform::set( int i )
{
    if( ! isCompatibleType(INT) ) return false;
    _data.i1 = i;
    return true;
}

bool Uniform::set( int i0, int i1 )
{
    if( ! isCompatibleType(INT_VEC2) ) return false;
    _data.i2[0] = i0;
    _data.i2[1] = i1;
    return true;
}

bool Uniform::set( int i0, int i1, int i2 )
{
    if( ! isCompatibleType(INT_VEC3) ) return false;
    _data.i3[0] = i0;
    _data.i3[1] = i1;
    _data.i3[2] = i2;
    return true;
}

bool Uniform::set( int i0, int i1, int i2, int i3 )
{
    if( ! isCompatibleType(INT_VEC4) ) return false;
    _data.i4[0] = i0;
    _data.i4[1] = i1;
    _data.i4[2] = i2;
    _data.i4[3] = i3;
    return true;
}

bool Uniform::set( bool b )
{
    if( ! isCompatibleType(BOOL) ) return false;
    _data.i1 = b;
    return true;
}

bool Uniform::set( bool b0, bool b1 )
{
    if( ! isCompatibleType(BOOL_VEC2) ) return false;
    _data.i2[0] = b0;
    _data.i2[1] = b1;
    return true;
}

bool Uniform::set( bool b0, bool b1, bool b2 )
{
    if( ! isCompatibleType(BOOL_VEC3) ) return false;
    _data.i3[0] = b0;
    _data.i3[1] = b1;
    _data.i3[2] = b2;
    return true;
}

bool Uniform::set( bool b0, bool b1, bool b2, bool b3 )
{
    if( ! isCompatibleType(BOOL_VEC4) ) return false;
    _data.i4[0] = b0;
    _data.i4[1] = b1;
    _data.i4[2] = b2;
    _data.i4[3] = b3;
    return true;
}

///////////////////////////////////////////////////////////////////////////
// value query

bool Uniform::get( float& f ) const
{
    if( ! isCompatibleType(FLOAT) ) return false;
    f = _data.f1;
    return true;
}

bool Uniform::get( osg::Vec2& v2 ) const
{
    if( ! isCompatibleType(FLOAT_VEC2) ) return false;
    v2.x() = _data.f2[0];
    v2.y() = _data.f2[1];
    return true;
}

bool Uniform::get( osg::Vec3& v3 ) const
{
    if( ! isCompatibleType(FLOAT_VEC3) ) return false;
    v3.x() = _data.f3[0];
    v3.y() = _data.f3[1];
    v3.z() = _data.f3[2];
    return true;
}

bool Uniform::get( osg::Vec4& v4 ) const
{
    if( ! isCompatibleType(FLOAT_VEC4) ) return false;
    v4.x() = _data.f4[0];
    v4.y() = _data.f4[1];
    v4.z() = _data.f4[2];
    v4.w() = _data.f4[3];
    return true;
}

//TODO bool Uniform::get( osg::Matrix2& m2 ) const

//TODO bool Uniform::get( osg::Matrix3& m3 ) const

bool Uniform::get( osg::Matrix& m4 ) const
{
    if( ! isCompatibleType(FLOAT_MAT4) ) return false;
    int n = 0;
    for(int row=0; row<4; ++row)
    {
    for(int col=0; col<4; ++col)
    {
        m4(row,col) = _data.f16[n++];
    }
    }
    return true;
}

bool Uniform::get( int& i ) const
{
    if( ! isCompatibleType(INT) ) return false;
    i = _data.i1;
    return true;
}

bool Uniform::get( int& i0, int& i1 ) const
{
    if( ! isCompatibleType(INT_VEC2) ) return false;
    i0 = _data.i2[0];
    i1 = _data.i2[1];
    return true;
}

bool Uniform::get( int& i0, int& i1, int& i2 ) const
{
    if( ! isCompatibleType(INT_VEC3) ) return false;
    i0 = _data.i3[0];
    i1 = _data.i3[1];
    i2 = _data.i3[2];
    return true;
}

bool Uniform::get( int& i0, int& i1, int& i2, int& i3 ) const
{
    if( ! isCompatibleType(INT_VEC4) ) return false;
    i0 = _data.i4[0];
    i1 = _data.i4[1];
    i2 = _data.i4[2];
    i3 = _data.i4[3];
    return true;
}

bool Uniform::get( bool& b ) const
{
    if( ! isCompatibleType(BOOL) ) return false;
    b = (_data.i1 != 0);
    return true;
}

bool Uniform::get( bool& b0, bool& b1 ) const
{
    if( ! isCompatibleType(BOOL_VEC2) ) return false;
    b0 = (_data.i2[0] != 0);
    b1 = (_data.i2[1] != 0);
    return true;
}

bool Uniform::get( bool& b0, bool& b1, bool& b2 ) const
{
    if( ! isCompatibleType(BOOL_VEC3) ) return false;
    b0 = (_data.i3[0] != 0);
    b1 = (_data.i3[1] != 0);
    b2 = (_data.i3[2] != 0);
    return true;
}

bool Uniform::get( bool& b0, bool& b1, bool& b2, bool& b3 ) const
{
    if( ! isCompatibleType(BOOL_VEC4) ) return false;
    b0 = (_data.i4[0] != 0);
    b1 = (_data.i4[1] != 0);
    b2 = (_data.i4[2] != 0);
    b3 = (_data.i4[3] != 0);
    return true;
}

void Uniform::apply(const GL2Extensions* ext, GLint location) const
{
    switch( _type )
    {
    case FLOAT:
        ext->glUniform1f( location, _data.f1 );
        break;

    case FLOAT_VEC2:
        ext->glUniform2fv( location, 1, _data.f2 );
        break;

    case FLOAT_VEC3:
        ext->glUniform3fv( location, 1, _data.f3 );
        break;

    case FLOAT_VEC4:
        ext->glUniform4fv( location, 1, _data.f4 );
        break;

    case FLOAT_MAT2:
        ext->glUniformMatrix2fv( location, 1, GL_FALSE, _data.f4 );
        break;

    case FLOAT_MAT3:
        ext->glUniformMatrix3fv( location, 1, GL_FALSE, _data.f9 );
        break;

    case FLOAT_MAT4:
        ext->glUniformMatrix4fv( location, 1, GL_FALSE, _data.f16 );
        break;

    case INT:
        ext->glUniform1i( location, _data.i1 );
        break;

    case INT_VEC2:
        ext->glUniform2iv( location, 1, _data.i2 );
        break;

    case INT_VEC3:
        ext->glUniform3iv( location, 1, _data.i3 );
        break;

    case INT_VEC4:
        ext->glUniform4iv( location, 1, _data.i4 );
        break;

    default:
        osg::notify(osg::FATAL) << "how got here?" << std::endl;
        break;
    }
}

/*EOF*/
