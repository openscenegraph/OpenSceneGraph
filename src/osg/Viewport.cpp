#include <osg/Viewport>

using namespace osg;

Viewport::Viewport()
{
    _x = 0;
    _y = 0;
    _width = 800;
    _height = 600;
}


Viewport::~Viewport()
{
}

void Viewport::apply(State&) const
{
    glViewport(_x,_y,_width,_height);
}

