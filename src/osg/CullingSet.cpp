#include <osg/CullingSet>

using namespace osg;

CullingSet::CullingSet()
{
    _mask = ALL_CULLING;
}

CullingSet::~CullingSet()
{
}
