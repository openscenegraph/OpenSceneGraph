#include <osg/GL>
#include <osg/LineStipple>
#include <osg/Notify>

using namespace osg;


LineStipple::LineStipple()
{
  _factor = 1;
  _pattern = 0xffff;
}


LineStipple::~LineStipple()
{
}

void LineStipple::setFactor(GLint factor)
{
    _factor = factor;
}
  
void LineStipple::setPattern(GLushort pattern)
{
    _pattern = pattern;
}

void LineStipple::apply(State&) const
{
    glLineStipple(_factor, _pattern);
}

