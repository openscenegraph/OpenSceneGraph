#include <osg/Fog>
#include <osg/GLExtensions>

using namespace osg;

#ifndef GL_FOG_COORDINATE_SOURCE
    #define GL_FOG_COORDINATE_SOURCE    0x8450
#endif

Fog::Fog()
{
    _mode = EXP;
    _density = 1.0f;
    _start   = 0.0f;
    _end     = 1.0f;
    _color.set( 0.0f, 0.0f, 0.0f, 0.0f);
    _fogCoordinateSource = FRAGMENT_DEPTH;
}


Fog::~Fog()
{
}

void Fog::apply(State&) const
{
    glFogi( GL_FOG_MODE,     _mode );
    glFogf( GL_FOG_DENSITY,  _density );
    glFogf( GL_FOG_START,    _start );
    glFogf( GL_FOG_END,      _end );
    glFogfv( GL_FOG_COLOR,    (GLfloat*)_color.ptr() );
    
    static bool fogCoordExtensionSuppoted = osg::isGLExtensionSupported("GL_EXT_fog_coord");
    if (fogCoordExtensionSuppoted)
    {
        glFogi(GL_FOG_COORDINATE_SOURCE,_fogCoordinateSource);
    }
}
