#include <osg/Light>
#include <osg/Notify>

using namespace osg;

int Light::_currentLightNum = -1;

Light::Light( void )
{
    _lightnum = ++_currentLightNum;
    _on = 1;

    init();

    //     notify(DEBUG) << "_ambient "<<_ambient<<endl;
    //     notify(DEBUG) << "_diffuse "<<_diffuse<<endl;
    //     notify(DEBUG) << "_specular "<<_specular<<endl;
    //     notify(DEBUG) << "_position "<<_position<<endl;
    //     notify(DEBUG) << "_direction "<<_direction<<endl;
    //     notify(DEBUG) << "_spot_exponent "<<_spot_exponent<<endl;
    //     notify(DEBUG) << "_spot_cutoff "<<_spot_cutoff<<endl;
    //     notify(DEBUG) << "_constant_attenuation "<<_constant_attenuation<<endl;
    //     notify(DEBUG) << "_linear_attenuation "<<_linear_attenuation<<endl;
    //     notify(DEBUG) << "_quadratic_attenuation "<<_quadratic_attenuation<<endl;
}


Light::~Light( void )
{
}


void Light::init( void )
{
    _ambient.set(0.05f,0.05f,0.05f,1.0f);
    _diffuse.set(0.8f,0.8f,0.8f,1.0f);
    _specular.set(0.05f,0.05f,0.05f,1.0f);
    _position.set(0.0f,0.0f,1.0f,0.0f);
    _direction.set(0.0f,0.0f,-1.0f);
    _spot_exponent = 0.0f;
    _spot_cutoff = 180.0f;
    _constant_attenuation = 1.0f;
    _linear_attenuation = 0.0f;
    _quadratic_attenuation = 0.0f;
}


void Light::captureLightState()
{
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_AMBIENT, _ambient.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_DIFFUSE, _diffuse.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPECULAR, _specular.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_POSITION, _position.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_DIRECTION, _direction.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_EXPONENT, &_spot_exponent );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_CUTOFF,   &_spot_cutoff );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_CONSTANT_ATTENUATION,   &_constant_attenuation );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_LINEAR_ATTENUATION,   &_linear_attenuation );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_QUADRATIC_ATTENUATION,   &_quadratic_attenuation );
}

void Light::apply(State&) const
{
    if( _on )
    {
        // note state should probably be handling the glEnable...
        glEnable ( (GLenum)((int)GL_LIGHT0 + _lightnum) );
        glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_AMBIENT,               _ambient.ptr() );
        glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_DIFFUSE,               _diffuse.ptr() );
        glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPECULAR,              _specular.ptr() );
        glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_POSITION,              _position.ptr() );
        glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_DIRECTION,        _direction.ptr() );
        glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_EXPONENT,         _spot_exponent );
        glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_CUTOFF,           _spot_cutoff );
        glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_CONSTANT_ATTENUATION,  _constant_attenuation );
        glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_LINEAR_ATTENUATION,    _linear_attenuation );
        glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_QUADRATIC_ATTENUATION, _quadratic_attenuation );
    }
    else
        glDisable( (GLenum)((int)GL_LIGHT0 + _lightnum) );
}
