#include <osg/GL>
#include <osg/Matrix>
#include <osgUtil/CameraManipulator>

using namespace osg;
using namespace osgUtil;

CameraManipulator::CameraManipulator(): _camera(NULL)
{
    _homeToUseNode = true;
    _eyeHome.set(0.0,0.0,0.0);
    _centerHome.set(0.0,0.0,-1.0);
    _upHome.set(0.0,1.0,0.0);
}


CameraManipulator::~CameraManipulator()
{
}


void CameraManipulator::setCamera(Camera *camera)
{
    _camera=camera;
}


const Camera *CameraManipulator::getCamera() const
{
    return _camera.get();
}


bool CameraManipulator::handle(const GUIEventAdapter&,GUIActionAdapter&)
{
    return false;
}
