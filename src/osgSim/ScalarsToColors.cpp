#include <osgSim/ScalarsToColors>

using namespace osgSim;

ScalarsToColors::ScalarsToColors(float scalarMin, float scalarMax): _min(scalarMin), _max(scalarMax)
{
}

osg::Vec4 ScalarsToColors::getColor(float scalar) const
{
    if(scalar<_min) return osg::Vec4(0.0f,0.0f,0.0f,0.0f);
    if(scalar>_max) return osg::Vec4(0.0f,0.0f,0.0f,0.0f);

    float c = (_min+scalar)/(_max-_min);
    return osg::Vec4(c,c,c,1.0);
}

float ScalarsToColors::getMin() const
{
    return _min;
}

float ScalarsToColors::getMax() const
{
    return _max;
}
