#include "osg/GL"
#include "osg/CullFace"

using namespace osg;

CullFace::CullFace()
{
    _mode = BACK;
}


CullFace::~CullFace()
{
}

void CullFace::apply(State&) const
{
    glCullFace((GLenum)_mode);
}
