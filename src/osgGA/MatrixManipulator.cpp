#include <osg/GL>
#include <osg/Matrix>
#include <osgGA/MatrixManipulator>

using namespace osg;
using namespace osgGA;

MatrixManipulator::MatrixManipulator()
{
    _minimumDistance = 0.001;
}


MatrixManipulator::~MatrixManipulator()
{
}



bool MatrixManipulator::handle(const GUIEventAdapter&,GUIActionAdapter&)
{
    return false;
}
