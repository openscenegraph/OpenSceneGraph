#include <osg/GL>
#include <osg/LineWidth>
#include <osg/Notify>

using namespace osg;


LineWidth::LineWidth()
{
    _width = 1.0f;
}


LineWidth::~LineWidth()
{
}

void LineWidth::setWidth( float width )
{
    _width = width;
}

void LineWidth::apply(State&) const
{
    glLineWidth(_width);
}

