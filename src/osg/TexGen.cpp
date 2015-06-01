/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <osg/TexGen>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osg;

TexGen::TexGen()
{
    _mode = OBJECT_LINEAR;
    _plane_s.set(1.0f, 0.0f, 0.0f, 0.0f);
    _plane_t.set(0.0f, 1.0f, 0.0f, 0.0f);
    _plane_r.set(0.0f, 0.0f, 1.0f, 0.0f);
    _plane_q.set(0.0f, 0.0f, 0.0f, 1.0f);
}


TexGen::~TexGen()
{
}


void TexGen::setPlane(Coord which, const Plane& plane)
{
    switch( which )
    {
        case S : _plane_s = plane; break;
        case T : _plane_t = plane; break;
        case R : _plane_r = plane; break;
        case Q : _plane_q = plane; break;
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::setPlane("<<(unsigned int)which<<","<<plane<<")"<<std::endl; break;
    }
}

const Plane& TexGen::getPlane(Coord which) const
{
    switch( which )
    {
        case S : return _plane_s;
        case T : return _plane_t;
        case R : return _plane_r;
        case Q : return _plane_q;
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r;
    }
}

Plane& TexGen::getPlane(Coord which)
{
    switch( which )
    {
        case S : return _plane_s;
        case T : return _plane_t;
        case R : return _plane_r;
        case Q : return _plane_q;
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r;
    }
}

void TexGen::setPlanesFromMatrix(const Matrixd& matrix)
{
    _plane_s.set(matrix(0,0),matrix(1,0),matrix(2,0),matrix(3,0));
    _plane_t.set(matrix(0,1),matrix(1,1),matrix(2,1),matrix(3,1));
    _plane_r.set(matrix(0,2),matrix(1,2),matrix(2,2),matrix(3,2));
    _plane_q.set(matrix(0,3),matrix(1,3),matrix(2,3),matrix(3,3));
}

void TexGen::apply(State&) const
{
#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
    if (_mode == OBJECT_LINEAR || _mode == EYE_LINEAR)
    {
        GLenum glmode = _mode == OBJECT_LINEAR ? GL_OBJECT_PLANE : GL_EYE_PLANE;

        if (sizeof(_plane_s[0])==sizeof(GLfloat))
        {
            glTexGenfv(GL_S, glmode, (const GLfloat*)_plane_s.ptr());
            glTexGenfv(GL_T, glmode, (const GLfloat*)_plane_t.ptr());
            glTexGenfv(GL_R, glmode, (const GLfloat*)_plane_r.ptr());
            glTexGenfv(GL_Q, glmode, (const GLfloat*)_plane_q.ptr());
        }
        else
        {
            glTexGendv(GL_S, glmode, (const GLdouble*)_plane_s.ptr());
            glTexGendv(GL_T, glmode, (const GLdouble*)_plane_t.ptr());
            glTexGendv(GL_R, glmode, (const GLdouble*)_plane_r.ptr());
            glTexGendv(GL_Q, glmode, (const GLdouble*)_plane_q.ptr());
        }

        glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, _mode );

    }
    else if (_mode == NORMAL_MAP)
    {
        glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, _mode );
//      glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, _mode );
    }
    else if (_mode == REFLECTION_MAP)
    {
        glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, _mode );
//      glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, _mode );
    }
    else                         // SPHERE_MAP
    {
        // Also don't set the mode of GL_R & GL_Q as these will generate
        // GL_INVALID_ENUM (See OpenGL Reference Guide, glTexGEn.)

        glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
    }
#else
    OSG_NOTICE<<"Warning: TexGen::apply(State&) - not supported."<<std::endl;
#endif
}
