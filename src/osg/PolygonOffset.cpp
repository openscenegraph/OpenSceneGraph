#include <osg/GL>
#include <osg/PolygonOffset>

using namespace osg;

PolygonOffset::PolygonOffset()
{
    _factor = 0.0f;              // are these sensible defaut values?
    _units = 0.0f;
}


PolygonOffset::~PolygonOffset()
{
}

void PolygonOffset::apply(State&) const
{
    glPolygonOffset(_factor,_units);
}
