#include <osg/GL>
#include <osg/ShadeModel>

using namespace osg;

ShadeModel::ShadeModel()
{
    _mode = SMOOTH;
}


ShadeModel::~ShadeModel()
{
}

void ShadeModel::apply(State&) const
{
    glShadeModel((GLenum)_mode);
}
