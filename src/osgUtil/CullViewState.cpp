
#include <osg/Transform>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/LightSource>
#include <osg/Impostor>
#include <osg/Notify>

#include <osgUtil/CullViewState>

#include <float.h>

using namespace osg;
using namespace osgUtil;

//#define USE_OLD_CULL

CullViewState::CullViewState()
{
    _matrix = NULL;
    _inverse = NULL;
    _ratio2 = 0.002f*0.002f;
    _bbCornerNear = 8; // note this is an error value, valid range is 0..7 
    _bbCornerFar = 8;  // these error values are used to show a unset corner.
}


CullViewState::~CullViewState()
{
}

