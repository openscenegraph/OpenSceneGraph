#include <osg/ShadowOccluderVolume>

using namespace osg;


ShadowOccluderVolume::ShadowOccluderVolume(const ShadowOccluderVolume& soc,Matrix& MVP)
{
    set(soc,MVP);
}

ShadowOccluderVolume::ShadowOccluderVolume(const ConvexPlanerOccluder& occluder,Matrix& MVP)
{
    set(occluder,MVP);
}

void ShadowOccluderVolume::set(const ShadowOccluderVolume& soc,Matrix& MVP)
{
    
}

void ShadowOccluderVolume::set(const ConvexPlanerOccluder& occluder,Matrix& MVP)
{
    
}

bool ShadowOccluderVolume::contains(const BoundingSphere& bound)
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

bool ShadowOccluderVolume::contains(const BoundingBox& bound)
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
