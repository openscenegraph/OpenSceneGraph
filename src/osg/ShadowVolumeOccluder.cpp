#include <osg/ShadowOccluderVolume>

using namespace osg;


void ShadowVolumeOccluder::computeOccluder(const NodePath& nodePath,const ConvexPlanerOccluder& occluder,const Matrix& MVP)
{
    std::cout<<"    Computing Occluder"<<std::endl;
}

bool ShadowVolumeOccluder::contains(const std::vector<Vec3>& vertices)
{
    if (_occluderVolume.containsAllOf(vertices))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(vertices)) return false;
        }
        return true;
    }
    return false;
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
