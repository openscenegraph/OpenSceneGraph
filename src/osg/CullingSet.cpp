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

void CullingSet::disableOccluder(NodePath& nodePath)
{
    //std::cout<<"  trying to disable occluder"<<std::endl;
    for(OccluderList::iterator itr=_occluderList.begin();
        itr!=_occluderList.end();
        ++itr)
    {
        if (itr->getNodePath()==nodePath)
        {
            //std::cout<<"  ++ disabling occluder"<<std::endl;
            // we have trapped for the case an occlude potentially occluding itself,
            // to prevent this we disable the results mask so that no subsequnt 
            // when the next pushCurrentMask calls happens this occluder is switched off.
            itr->disableResultMasks();
        }
    }
}

