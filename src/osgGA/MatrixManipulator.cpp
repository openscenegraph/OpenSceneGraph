#include <osg/GL>
#include <osg/Matrix>
#include <osgGA/MatrixManipulator>

using namespace osg;
using namespace osgGA;

MatrixManipulator::MatrixManipulator()
{
    _minimumDistance = 0.001;
    
    _intersectTraversalMask = 0xffffffff;

    _autoComputeHomePosition = true;
        
    _homeEye.set(0.0,-1.0,0.0);
    _homeCenter.set(0.0,0.0,0.0);
    _homeUp.set(0.0,0.0,1.0);
}


MatrixManipulator::~MatrixManipulator()
{
}



bool MatrixManipulator::handle(const GUIEventAdapter&,GUIActionAdapter&)
{
    return false;
}
