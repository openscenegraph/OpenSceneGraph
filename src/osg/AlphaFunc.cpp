#include <osg/AlphaFunc>

using namespace osg;

AlphaFunc::AlphaFunc()
{
    _comparisonFunc = ALWAYS;
    _referenceValue = 1.0f;
}


AlphaFunc::~AlphaFunc()
{
}

void AlphaFunc::apply(State&) const
{
    glAlphaFunc((GLenum)_comparisonFunc,_referenceValue);
}

