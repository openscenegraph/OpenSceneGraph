#include <osg/BlendFunc>

using namespace osg;

BlendFunc::BlendFunc()
{
    _source_factor      = SRC_ALPHA;
    _destination_factor = ONE_MINUS_SRC_ALPHA;
}


BlendFunc::~BlendFunc()
{
}

void BlendFunc::apply(State&) const
{
    glBlendFunc( (GLenum)_source_factor, (GLenum)_destination_factor );
}
