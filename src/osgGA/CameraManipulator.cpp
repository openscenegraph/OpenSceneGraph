#include <osg/GL>
#include <osg/Matrix>
#include <osgGA/CameraManipulator>

using namespace osg;
using namespace osgGA;

CameraManipulator::CameraManipulator(): _camera(NULL)
{
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

Camera *CameraManipulator::getCamera()
{
    return _camera.get();
}

bool CameraManipulator::handle(const GUIEventAdapter&,GUIActionAdapter&)
{
    return false;
}
