#include <osg/GL>
#include <osg/Matrix>
#include <osgGA/MatrixManipulator>

using namespace osg;
using namespace osgGA;

MatrixManipulator::MatrixManipulator()
{
}


MatrixManipulator::~MatrixManipulator()
{
}



bool MatrixManipulator::handle(const GUIEventAdapter&,GUIActionAdapter&)
{
    return false;
}
