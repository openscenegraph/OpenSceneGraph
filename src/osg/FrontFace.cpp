#include "osg/GL"
#include "osg/FrontFace"

using namespace osg;

FrontFace::FrontFace()
{
    _mode = COUNTER_CLOCKWISE;
}


FrontFace::~FrontFace()
{
}

void FrontFace::apply(State&) const
{
    glFrontFace((GLenum)_mode);
}
