#include <osg/TexGen>
#include <osg/Notify>

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
        default : notify(WARN)<<"Error: invalid 'which' passed TexGen::setPlane("<<(unsigned int)which<<","<<plane<<")"<<std::endl; break;
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
        default : notify(WARN)<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r;
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
        default : notify(WARN)<<"Error: invalid 'which' passed TexGen::getPlane(which)"<<std::endl; return _plane_r;
    }
}

void TexGen::apply(State&) const
{

    if (_mode == OBJECT_LINEAR)
    {
        glTexGenfv(GL_S, GL_OBJECT_PLANE, _plane_s.ptr());
        glTexGenfv(GL_T, GL_OBJECT_PLANE, _plane_t.ptr());
        glTexGenfv(GL_R, GL_OBJECT_PLANE, _plane_r.ptr());
        glTexGenfv(GL_Q, GL_OBJECT_PLANE, _plane_q.ptr());

        glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, _mode );

        // note, R & Q will be disabled so R&Q settings won't
        // have an effect, see above comment in enable(). RO.

    }
    else if (_mode == EYE_LINEAR)
    {
        glTexGenfv(GL_S, GL_EYE_PLANE, _plane_s.ptr());
        glTexGenfv(GL_T, GL_EYE_PLANE, _plane_t.ptr());
        glTexGenfv(GL_R, GL_EYE_PLANE, _plane_r.ptr());
        glTexGenfv(GL_Q, GL_EYE_PLANE, _plane_q.ptr());

        glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, _mode );

        // note, R & Q will be disabled so R&Q settings won't
        // have an effect, see above comment in enable(). RO.

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
        // We ignore the planes if we are not in OBJECT_ or EYE_LINEAR mode.

        // Also don't set the mode of GL_R & GL_Q as these will generate
        // GL_INVALID_ENUM (See OpenGL Refrence Guide, glTexGEn.)

        glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
        glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
    }

}
