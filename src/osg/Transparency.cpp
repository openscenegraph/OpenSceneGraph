#include "osg/Transparency"

using namespace osg;

Transparency::Transparency()
{
    _source_factor      = SRC_ALPHA;
    _destination_factor = ONE_MINUS_SRC_ALPHA;
}


Transparency::~Transparency()
{
}

void Transparency::apply(State&) const
{
    glBlendFunc( (GLenum)_source_factor, (GLenum)_destination_factor );
}
