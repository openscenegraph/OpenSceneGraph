#include <osg/CullingSet>

using namespace osg;

CullingSet::CullingSet()
{
    _mask = ALL_CULLING;
    _pixelSizeVector.set(0,0,0,1);
    _smallFeatureCullingPixelSize=1.0f;
}

CullingSet::~CullingSet()
{
}
