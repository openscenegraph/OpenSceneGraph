// Ideas and code borrowed from GLUT pointburst demo
// written by Mark J. Kilgard 

#ifdef WIN32
#include <windows.h>
#endif

#include "osg/GL"
#include "osg/Point"
#include "osg/ExtensionSupported"
#include "osg/Input"
#include "osg/Output"


using namespace osg;

#if defined(GL_SGIS_point_parameters) && !defined(GL_EXT_point_parameters)
/* Use the EXT point parameters interface for the SGIS implementation. */
#  define GL_POINT_SIZE_MIN_EXT GL_POINT_SIZE_MIN_SGIS
#  define GL_POINT_SIZE_MAX_EXT GL_POINT_SIZE_MAX_SGIS
#  define GL_POINT_FADE_THRESHOLD_SIZE_EXT GL_POINT_FADE_THRESHOLD_SIZE_SGIS
#  define GL_DISTANCE_ATTENUATION_EXT GL_DISTANCE_ATTENUATION_SGIS
#  define glPointParameterfEXT glPointParameterfSGIS
#  define glPointParameterfvEXT glPointParameterfvSGIS
#  define GL_EXT_point_parameters 1
#endif

#if !defined(GL_EXT_point_parameters)
#  define GL_POINT_SIZE_MIN_EXT               0x8126
#  define GL_POINT_SIZE_MAX_EXT               0x8127
#  define GL_POINT_FADE_THRESHOLD_SIZE_EXT    0x8128
#  define GL_DISTANCE_ATTENUATION_EXT         0x8129
#  ifdef _WIN32
     // Curse Microsoft for the insanity of wglGetProcAddress.
     typedef void (APIENTRY * PFNGLPOINTPARAMETERFEXTPROC) (GLenum pname, GLfloat param);
     typedef void (APIENTRY * PFNGLPOINTPARAMETERFVEXTPROC) (GLenum pname, const GLfloat *params);
#    define GL_EXT_point_parameters 1
#  endif
#endif

#ifdef _WIN32
  PFNGLPOINTPARAMETERFEXTPROC glPointParameterfEXT;
  PFNGLPOINTPARAMETERFVEXTPROC glPointParameterfvEXT;
#endif

static int  s_hasPointParameters;


Point::Point( void )
{
    _size = 1.0f;                                       // TODO find proper default
    _fadeThresholdSize = 1.0f;                           // TODO find proper default
    _distanceAttenuation = Vec3(0.0f, 1.0f/5.f, 0.0f);  // TODO find proper default
}


Point::~Point( void )
{
}


Point* Point::instance()
{
    static ref_ptr<Point> s_point(new Point);
    return s_point.get();
}


void Point::init_GL_EXT()
{
    s_hasPointParameters = 
                ExtensionSupported("GL_SGIS_point_parameters") ||
                ExtensionSupported("GL_EXT_point_parameters");
#ifdef _WIN32
    if (s_hasPointParameters)
    {
        glPointParameterfEXT = (PFNGLPOINTPARAMETERFEXTPROC)
        wglGetProcAddress("glPointParameterfEXT");
        glPointParameterfvEXT = (PFNGLPOINTPARAMETERFVEXTPROC)
        wglGetProcAddress("glPointParameterfvEXT");
    }
#endif
}


void Point::enableSmooth( void )
{
    glEnable( GL_POINT_SMOOTH );
}


void Point::disableSmooth( void )
{
    glDisable( GL_POINT_SMOOTH );
}


void Point::setSize( float size )
{
    _size = size;
}

void Point::setFadeThresholdSize(float fadeThresholdSize)
{
    _fadeThresholdSize = fadeThresholdSize;
}

void Point::setDistanceAttenuation(const Vec3& distanceAttenuation)
{
    _distanceAttenuation = distanceAttenuation;
}

void Point::apply( void )
{
    glPointSize(_size);

#if GL_EXT_point_parameters
    static bool s_gl_ext_init=false;

    if (!s_gl_ext_init)
    {
        s_gl_ext_init = true;
        init_GL_EXT();
    }

    if (s_hasPointParameters)
    {
        glPointParameterfvEXT(GL_DISTANCE_ATTENUATION_EXT, (const GLfloat*)&_distanceAttenuation);
        glPointParameterfEXT(GL_POINT_FADE_THRESHOLD_SIZE_EXT, _fadeThresholdSize);
    }
#endif
}


bool Point::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    float data;
    if (fr[0].matchWord("size") && fr[1].getFloat(data))
    {

        _size = data;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("fade_threshold_size") && fr[1].getFloat(data))
    {

        _fadeThresholdSize = data;
        fr+=2;
        iteratorAdvanced = true;
    }

    Vec3 distAtten;
    if (fr[0].matchWord("distance_attenuation") &&
        fr[1].getFloat(distAtten[0]) && fr[2].getFloat(distAtten[1]) && fr[3].getFloat(distAtten[2]))
    {

        _distanceAttenuation = distAtten;
        fr+=4;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Point::writeLocalData(Output& fw)
{
    fw.indent() << "size " << _size << endl;
    fw.indent() << "fade_threshold_size  " << _fadeThresholdSize << endl;
    fw.indent() << "distance_attenuation  " << _distanceAttenuation << endl;
    return true;
}


