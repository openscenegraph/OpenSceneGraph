#include <osg/ColorMask>

using namespace osg;

ColorMask::ColorMask()
{
    // set up same defaults as glColorMask.
    _red = true;
    _green = true;
    _blue = true;
    _alpha = true;
}


ColorMask::~ColorMask()
{
}

void ColorMask::apply(State&) const
{
    glColorMask((GLboolean)_red,(GLboolean)_green,(GLboolean)_blue,(GLboolean)_alpha);
}

