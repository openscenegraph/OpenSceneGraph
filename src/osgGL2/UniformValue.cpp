/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 * Copyright (C) 2003 3Dlabs Inc. Ltd.
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
 * applications, as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* file:	src/osgGL2/UniformValue
 * author:	Mike Weiblen 2003-12-27
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

#include <string>

#include <osg/Notify>
#include <osgGL2/UniformValue>

using namespace osgGL2;
using namespace osg;

int UniformValue::getLocation( Extensions *ext, const GLhandleARB progObj ) const
{
    GLint loc = ext->glGetUniformLocation( progObj, _name.c_str() );
    if( loc == -1 )
    {
	osg::notify(osg::INFO) << "Uniform \"" << _name << 
		"\" not found in ProgramObject" << std::endl;
    }
    return loc;
}

///////////////////////////////////////////////////////////////////////////

void UniformValue_int::apply( Extensions *ext, const GLhandleARB progObj ) const
{
    int loc = getLocation( ext, progObj );
    if( loc != -1 )
    {
	ext->glUniform1i( loc, _value );
    }
} 

void UniformValue_float::apply( Extensions *ext, const GLhandleARB progObj ) const
{
    int loc = getLocation( ext, progObj );
    if( loc != -1 )
    {
	ext->glUniform1f( loc, _value );
    }
}

void UniformValue_Vec2::apply( Extensions *ext, const GLhandleARB progObj ) const
{
    int loc = getLocation( ext, progObj );
    if( loc != -1 )
    {
	ext->glUniform2fv( loc, 1, _value.ptr() );
    }
}

void UniformValue_Vec3::apply( Extensions *ext, const GLhandleARB progObj ) const
{
    int loc = getLocation( ext, progObj );
    if( loc != -1 )
    {
	ext->glUniform3fv( loc, 1, _value.ptr() );
    }
}

void UniformValue_Vec4::apply( Extensions *ext, const GLhandleARB progObj ) const
{
    int loc = getLocation( ext, progObj );
    if( loc != -1 )
    {
	ext->glUniform4fv( loc, 1, _value.ptr() );
    }
}

/*EOF*/
