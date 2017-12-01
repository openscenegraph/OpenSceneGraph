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
#include <osg/State>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osg;

TexGen::TexGen()
{
    _mode = OBJECT_LINEAR;
    _plane_s = new osg::PlaneUniform(osg::Plane(1.0f, 0.0f, 0.0f, 0.0f));
    _plane_t = new osg::PlaneUniform(osg::Plane(0.0f, 1.0f, 0.0f, 0.0f));
    _plane_r = new osg::PlaneUniform(osg::Plane(0.0f, 0.0f, 1.0f, 0.0f));
    _plane_q = new osg::PlaneUniform(osg::Plane(0.0f, 0.0f, 0.0f, 1.0f));

    configureUniformNames();
}

/** Copy constructor using CopyOp to manage deep vs shallow copy. */
TexGen::TexGen(const TexGen& texgen,const CopyOp& copyop):
    TextureAttribute(texgen,copyop),
    _mode(texgen._mode)
{
    _plane_s = new osg::PlaneUniform(texgen._plane_s->getValue());
    _plane_t = new osg::PlaneUniform(texgen._plane_t->getValue());
    _plane_r = new osg::PlaneUniform(texgen._plane_r->getValue());
    _plane_q = new osg::PlaneUniform(texgen._plane_q->getValue());

    configureUniformNames();
}


TexGen::~TexGen()
{
}

void TexGen::setMode( Mode mode )
{
    if (_mode!=mode)
    {
        _mode = mode;
        configureUniformNames();
    }
}


void TexGen::configureUniformNames()
{
    //OSG_NOTICE<<__PRETTY_FUNCTION__<<"  _textureUnit="<<_textureUnit<<std::endl;

    MakeString str;
    std::string TEXTURE_GEN_FUNCTION = str<<"TEXTURE_GEN_FUNCTION"<<_textureUnit;

    _defineList.clear();

    switch(_mode)
    {
        case(OBJECT_LINEAR):
            _plane_s->setName("osg_ObjectPlaneS", _textureUnit);
            _plane_t->setName("osg_ObjectPlaneT", _textureUnit);
            _plane_r->setName("osg_ObjectPlaneR", _textureUnit);
            _plane_q->setName("osg_ObjectPlaneQ", _textureUnit);

            _defineList[TEXTURE_GEN_FUNCTION] = StateSet::DefinePair(str.clear()<<"(texcoord, unit, s, t, r, q) texgen_OBJECT_LINEAR(texcoord, unit, s, t, r, q)", osg::StateAttribute::ON);
            break;

        case(EYE_LINEAR):
            _plane_s->setName("osg_EyePlaneS", _textureUnit);
            _plane_t->setName("osg_EyePlaneT", _textureUnit);
            _plane_r->setName("osg_EyePlaneR", _textureUnit);
            _plane_q->setName("osg_EyePlaneQ", _textureUnit);
            _defineList[TEXTURE_GEN_FUNCTION] = StateSet::DefinePair(str.clear()<<"(texcoord, unit, s, t, r, q) texgen_EYE_LINEAR(texcoord, unit, s, t, r, q)", osg::StateAttribute::ON);
            break;

        case(NORMAL_MAP):
            _defineList[TEXTURE_GEN_FUNCTION] = StateSet::DefinePair(str.clear()<<"(texcoord, unit, s, t, r, q) texgen_NORMAL_MAP(texcoord, unit, s, t, r, q)", osg::StateAttribute::ON);
            break;

        case(REFLECTION_MAP):
            _defineList[TEXTURE_GEN_FUNCTION] = StateSet::DefinePair(str.clear()<<"(texcoord, unit, s, t, r, q) texgen_REFLECTION_MAP(texcoord, unit, s, t, r, q)", osg::StateAttribute::ON);
            break;

        case(SPHERE_MAP):
            _defineList[TEXTURE_GEN_FUNCTION] = StateSet::DefinePair(str.clear()<<"(texcoord, unit, s, t, r, q) texgen_SPHERE_MAP(texcoord, unit, s, t, r, q)", osg::StateAttribute::ON);
            break;

        default:
            break;
    }
}

void TexGen::setPlane(Coord which, const Plane& plane)
{
    switch( which )
    {
        case S : _plane_s->setValue(plane); break;
        case T : _plane_t->setValue(plane); break;
        case R : _plane_r->setValue(plane); break;
        case Q : _plane_q->setValue(plane); break;
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::setPlane("<<(unsigned int)which<<","<<plane<<")"<<std::endl; break;
    }
}

const Plane& TexGen::getPlane(Coord which) const
{
    switch( which )
    {
        case S : return _plane_s->getValue();
        case T : return _plane_t->getValue();
        case R : return _plane_r->getValue();
        case Q : return _plane_q->getValue();
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r->getValue();
    }
}

Plane& TexGen::getPlane(Coord which)
{
    switch( which )
    {
        case S : return _plane_s->getValue();
        case T : return _plane_t->getValue();
        case R : return _plane_r->getValue();
        case Q : return _plane_q->getValue();
        default : OSG_WARN<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r->getValue();
    }
}

void TexGen::setPlanesFromMatrix(const Matrixd& matrix)
{
    _plane_s->setValue(Plane(matrix(0,0),matrix(1,0),matrix(2,0),matrix(3,0)));
    _plane_t->setValue(Plane(matrix(0,1),matrix(1,1),matrix(2,1),matrix(3,1)));
    _plane_r->setValue(Plane(matrix(0,2),matrix(1,2),matrix(2,2),matrix(3,2)));
    _plane_q->setValue(Plane(matrix(0,3),matrix(1,3),matrix(2,3),matrix(3,3)));
}


void TexGen::apply(State& state) const
{
#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
    if (state.getUseStateAttributeFixedFunction())
    {
        if (_mode == OBJECT_LINEAR || _mode == EYE_LINEAR)
        {
            GLenum glmode = _mode == OBJECT_LINEAR ? GL_OBJECT_PLANE : GL_EYE_PLANE;

            if (sizeof((_plane_s->getValue())[0])==sizeof(GLfloat))
            {
                glTexGenfv(GL_S, glmode, (const GLfloat*)_plane_s->getValue().ptr());
                glTexGenfv(GL_T, glmode, (const GLfloat*)_plane_t->getValue().ptr());
                glTexGenfv(GL_R, glmode, (const GLfloat*)_plane_r->getValue().ptr());
                glTexGenfv(GL_Q, glmode, (const GLfloat*)_plane_q->getValue().ptr());
            }
            else
            {
                glTexGendv(GL_S, glmode, (const GLdouble*)_plane_s->getValue().ptr());
                glTexGendv(GL_T, glmode, (const GLdouble*)_plane_t->getValue().ptr());
                glTexGendv(GL_R, glmode, (const GLdouble*)_plane_r->getValue().ptr());
                glTexGendv(GL_Q, glmode, (const GLdouble*)_plane_q->getValue().ptr());
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
        else // SPHERE_MAP
        {
            // Also don't set the mode of GL_R & GL_Q as these will generate
            // GL_INVALID_ENUM (See OpenGL Reference Guide, glTexGEn.)

            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
        }
    }

    if (state.getUseStateAttributeShaders())
#endif
    {
        state.applyShaderCompositionDefines(_defineList);

        switch(_mode)
        {
            case(OBJECT_LINEAR):
                state.applyShaderCompositionUniform(_plane_s.get());
                state.applyShaderCompositionUniform(_plane_t.get());
                state.applyShaderCompositionUniform(_plane_r.get());
                state.applyShaderCompositionUniform(_plane_q.get());
                break;
            case(EYE_LINEAR):
                state.applyShaderCompositionUniform(_plane_s.get());
                state.applyShaderCompositionUniform(_plane_t.get());
                state.applyShaderCompositionUniform(_plane_r.get());
                state.applyShaderCompositionUniform(_plane_q.get());
                break;
            default:
                break;
        }
    }

}
