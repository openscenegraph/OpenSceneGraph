#include <osg/ShadowVolumeOccluder>

using namespace osg;


void ShadowVolumeOccluder::computeOccluder(const NodePath& nodePath,const ConvexPlanerOccluder& occluder,const Matrix& MV,const Matrix& P)
{
    std::cout<<"    Computing Occluder"<<std::endl;
    
    // for the occluder polygon and each of the holes do
    //     first transform occluder polygon into clipspace by multiple it by c[i] = v[i]*(MV*P)
    //     then push to coords to far plane by setting its coord to c[i].z = -1.
    //     then transform far plane polygon back into projection space, by p[i]*inv(P)
    //     compute orientation of front plane, if normal.z()<0 then facing away from eye pont, so reverse the polygons, or simply invert planes.
    //     compute volume (quality) betwen front polygon in projection space and back polygon in projection space.
    
    
    
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
