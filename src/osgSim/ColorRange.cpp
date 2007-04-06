#include <osgSim/ColorRange>

using namespace osgSim;

ColorRange::ColorRange(float min, float max): ScalarsToColors(min,max)
{
   // Default to something sensible
    _colors.push_back(osg::Vec4(1.0,0.0,0.0,1.0));  // R
    _colors.push_back(osg::Vec4(1.0,1.0,0.0,1.0));  // Y
    _colors.push_back(osg::Vec4(0.0,1.0,0.0,1.0));  // G
    _colors.push_back(osg::Vec4(0.0,1.0,1.0,1.0));  // C
    _colors.push_back(osg::Vec4(0.0,0.0,1.0,1.0));  // B
}

ColorRange::ColorRange(float min, float max, const std::vector<osg::Vec4>& colors): ScalarsToColors(min,max)
{
    setColors(colors);
}

void ColorRange::setColors(const std::vector<osg::Vec4>& colors)
{
    _colors=colors;
}

osg::Vec4 ColorRange::getColor(float scalar) const
{
    if (_colors.empty()) return osg::Vec4(1.0f,1.0f,1.0f,1.0f);
    if (_colors.size()==1) return  _colors.front();

    if(scalar<getMin()) return _colors.front();
    if(scalar>getMax()) return _colors.back();

    float r = ((scalar - getMin())/(getMax() - getMin())) * (_colors.size()-1);
    int lower = static_cast<int>(floor(r));
    int upper = static_cast<int>(ceil(r));

    osg::Vec4 color = _colors[lower] + ((_colors[upper] - _colors[lower]) * (r-lower));
    return color;
}
