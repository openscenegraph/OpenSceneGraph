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
    glBlendFunc( _source_factor, _destination_factor );
}
