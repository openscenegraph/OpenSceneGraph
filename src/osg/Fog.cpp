#include "osg/GL"
#include "osg/Fog"

using namespace osg;

Fog::Fog( void )
{
    _mode = EXP;
    _density = 1.0f;
    _start   = 0.0f;
    _end     = 1.0f;
    _color.set( 0.0f, 0.0f, 0.0f, 0.0f);
}


Fog::~Fog( void )
{
}


Fog* Fog::instance()
{
    static ref_ptr<Fog> s_fog(new Fog);
    return s_fog.get();
}


void Fog::enable( void )
{
    glEnable( GL_FOG );
}


void Fog::disable( void )
{
    glDisable( GL_FOG );
}


void Fog::apply( void )
{
    glFogi( GL_FOG_MODE,     _mode );
    glFogf( GL_FOG_DENSITY,  _density );
    glFogf( GL_FOG_START,    _start );
    glFogf( GL_FOG_END,      _end );
    glFogfv( GL_FOG_COLOR,    (GLfloat*)_color.ptr() );
}
