#include <osg/ShadowOccluderVolume>

using namespace osg;


void ShadowVolumeOccluder::computeOccluder(const NodePath& nodePath,const ConvexPlanerOccluder& occluder,const Matrix& MVP)
{
}

bool ShadowVolumeOccluder::contains(const BoundingSphere& bound)
{
    if (_occluderVolume.containsAllOf(bound))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(bound)) return false;
        }
        return true;
    }
    return false;
}

bool ShadowVolumeOccluder::contains(const BoundingBox& bound)
{
    if (_occluderVolume.containsAllOf(bound))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(bound)) return false;
        }
        return true;
    }
    return false;
}
