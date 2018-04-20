/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
 *  Copyright (C) 2017 Julien Valentin <mp3butcher@hotmail.com>
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */


#include <osgAnimation/VertexInfluence>
#include <osgAnimation/RigTransformSoftware>
#include <osgAnimation/BoneMapVisitor>
#include <osgAnimation/RigGeometry>

#include <algorithm>

using namespace osgAnimation;

RigTransformSoftware::RigTransformSoftware()
{
    _needInit = true;
}

RigTransformSoftware::RigTransformSoftware(const RigTransformSoftware& rts,const osg::CopyOp& copyop):
    RigTransform(rts, copyop),
    _needInit(rts._needInit),
    _invalidInfluence(rts._invalidInfluence)
{

}

void RigTransformSoftware::buildMinimumUpdateSet( const RigGeometry&rig )
{
    ///1 Create Index2Vec<BoneWeight>
    unsigned int nbVertices=rig.getSourceGeometry()->getVertexArray()->getNumElements();
    const VertexInfluenceMap &vertexInfluenceMap = *rig.getInfluenceMap();
    std::vector<BonePtrWeightList> perVertexInfluences;
    perVertexInfluences.reserve(nbVertices);
    perVertexInfluences.resize(nbVertices);

    unsigned int vimapBoneID = 0;
    for (osgAnimation::VertexInfluenceMap::const_iterator perBoneinfit = vertexInfluenceMap.begin();
            perBoneinfit != vertexInfluenceMap.end();
            ++perBoneinfit,++vimapBoneID)
    {
        const IndexWeightList& inflist = perBoneinfit->second;
        const std::string& bonename = perBoneinfit->first;

        if (bonename.empty())
        {
            OSG_WARN << "RigTransformSoftware::VertexInfluenceMap contains unnamed bone IndexWeightList" << std::endl;
        }
        for(IndexWeightList::const_iterator infit = inflist.begin(); infit!=inflist.end(); ++infit)
        {
            const VertexIndexWeight &iw = *infit;
            const unsigned int &index = iw.first;
            float weight = iw.second;
            perVertexInfluences[index].push_back(BonePtrWeight(vimapBoneID, weight));
        }
    }

    ///2 Create inverse mapping Vec<BoneWeight>2Vec<Index> from previous built Index2Vec<BoneWeight>
    ///in order to minimize weighted matrices computation on update
    _uniqVertexGroupList.clear();

    typedef std::map<BonePtrWeightList, VertexGroup> UnifyBoneGroup;
    UnifyBoneGroup unifyBuffer;
    unsigned int vertexID = 0;
    for (std::vector<BonePtrWeightList>::iterator perVertinfit = perVertexInfluences.begin();
            perVertinfit!=perVertexInfluences.end();
            ++perVertinfit,++vertexID)
    {
        BonePtrWeightList &boneinfs = *perVertinfit;
        // sort the vector to have a consistent key
        std::sort(boneinfs.begin(), boneinfs.end() );
        // we use the vector<BoneWeight> as key to differentiate group
        UnifyBoneGroup::iterator result = unifyBuffer.find(boneinfs);
        if (result != unifyBuffer.end())
            result->second.getVertices().push_back(vertexID);
        else
        {
            VertexGroup& vg = unifyBuffer[boneinfs];
            vg.getBoneWeights() = boneinfs;
            vg.getVertices().push_back(vertexID);
        }
    }
    
    _uniqVertexGroupList.reserve(unifyBuffer.size());
    for (UnifyBoneGroup::const_iterator it = unifyBuffer.begin(); it != unifyBuffer.end(); ++it)
    {
        _uniqVertexGroupList.push_back(it->second);
    }
    OSG_INFO << "uniq groups " << _uniqVertexGroupList.size() << " for " << rig.getName() << std::endl;
}


bool RigTransformSoftware::prepareData(RigGeometry&rig)
{
    ///set geom as it source
    if (rig.getSourceGeometry())
        rig.copyFrom(*rig.getSourceGeometry());

    osg::Vec3Array* normalSrc = dynamic_cast<osg::Vec3Array*>(rig.getSourceGeometry()->getNormalArray());
    osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(rig.getSourceGeometry()->getVertexArray());

    if(!(positionSrc) || positionSrc->empty() )
        return false;
    if(normalSrc && normalSrc->size() != positionSrc->size())
        return false;

    /// setup Vertex and Normal arrays with copy of sources
    rig.setVertexArray(new osg::Vec3Array);
    osg::Vec3Array* positionDst = new osg::Vec3Array;
    rig.setVertexArray(positionDst);
    *positionDst = *positionSrc;
    positionDst->setDataVariance(osg::Object::DYNAMIC);

    if(normalSrc)
    {
        osg::Vec3Array* normalDst = new osg::Vec3Array;
        *normalDst = *normalSrc;
        rig.setNormalArray(normalDst, osg::Array::BIND_PER_VERTEX);
        normalDst->setDataVariance(osg::Object::DYNAMIC);
    }

    /// build minimal set of VertexGroup
    buildMinimumUpdateSet(rig);

    return true;
}

bool RigTransformSoftware::init(RigGeometry&rig)
{
    ///test if dataprepared
    if(_uniqVertexGroupList.empty())
    {
        prepareData(rig);
        return false;
    }

    if(!rig.getSkeleton())
        return false;
    
    ///get bonemap from skeleton
    BoneMapVisitor mapVisitor;
    rig.getSkeleton()->accept(mapVisitor);
    BoneMap boneMap = mapVisitor.getBoneMap();
    VertexInfluenceMap & vertexInfluenceMap = *rig.getInfluenceMap();

    ///create local bonemap
    std::vector<Bone*> localid2bone;
    localid2bone.reserve(vertexInfluenceMap.size());
    for (osgAnimation::VertexInfluenceMap::const_iterator perBoneinfit = vertexInfluenceMap.begin();
            perBoneinfit != vertexInfluenceMap.end();
            ++perBoneinfit)
    {
        const std::string& bonename = perBoneinfit->first;

        if (bonename.empty())
        {
            OSG_WARN << "RigTransformSoftware::VertexInfluenceMap contains unnamed bone IndexWeightList" << std::endl;
        }
        BoneMap::const_iterator bmit = boneMap.find(bonename);
        if (bmit == boneMap.end() )
        {
            if (_invalidInfluence.find(bonename) == _invalidInfluence.end())
            {
                _invalidInfluence[bonename] = true;
                OSG_WARN << "RigTransformSoftware Bone " << bonename << " not found, skip the influence group " << std::endl;
            }

            localid2bone.push_back(0);
            continue;
        }
        
        Bone* bone = bmit->second.get();
        localid2bone.push_back(bone);
    }

    ///fill bone ptr in the _uniqVertexGroupList
    for(VertexGroupList::iterator itvg = _uniqVertexGroupList.begin(); itvg != _uniqVertexGroupList.end(); ++itvg)
    {
        VertexGroup& uniq = *itvg;
        for(BonePtrWeightList::iterator bwit = uniq.getBoneWeights().begin(); bwit != uniq.getBoneWeights().end(); )
        {
            Bone * b = localid2bone[bwit->getBoneID()];
            if(!b)
                bwit = uniq.getBoneWeights().erase(bwit);
            else
                bwit++->setBonePtr(b);
        }
    }

    for(VertexGroupList::iterator itvg = _uniqVertexGroupList.begin(); itvg != _uniqVertexGroupList.end(); ++itvg)
    {
        itvg->normalize();
    }

    _needInit = false;

    return true;
}

void RigTransformSoftware::VertexGroup::normalize()
{
    osg::Matrix::value_type sum=0;
    for(BonePtrWeightList::iterator bwit = _boneweights.begin(); bwit != _boneweights.end(); ++bwit )
    {
        sum += bwit->getWeight();
    }

    if (sum < 1e-4)
    {
        OSG_WARN << "RigTransformSoftware::VertexGroup: warning try to normalize a zero sum vertexgroup" << std::endl;
    }
    else
    {
        for(BonePtrWeightList::iterator bwit = _boneweights.begin(); bwit != _boneweights.end(); ++bwit )
        {
            bwit->setWeight(bwit->getWeight()/sum);
        }
    }
}

void RigTransformSoftware::operator()(RigGeometry& geom)
{
    if (_needInit && !init(geom)) return;

    if (!geom.getSourceGeometry())
    {
        OSG_WARN << this << " RigTransformSoftware no source geometry found on RigGeometry" << std::endl;
        return;
    }
    
    osg::Geometry& source = *geom.getSourceGeometry();
    osg::Geometry& destination = geom;

    osg::Vec3Array* positionSrc = static_cast<osg::Vec3Array*>(source.getVertexArray());
    osg::Vec3Array* positionDst = static_cast<osg::Vec3Array*>(destination.getVertexArray());
    osg::Vec3Array* normalSrc = dynamic_cast<osg::Vec3Array*>(source.getNormalArray());
    osg::Vec3Array* normalDst = static_cast<osg::Vec3Array*>(destination.getNormalArray());


    compute<osg::Vec3>(geom.getMatrixFromSkeletonToGeometry(),
                       geom.getInvMatrixFromSkeletonToGeometry(),
                       &positionSrc->front(),
                       &positionDst->front());    
    positionDst->dirty();

    if (normalSrc )
    {
        computeNormal<osg::Vec3>(geom.getMatrixFromSkeletonToGeometry(),
                                 geom.getInvMatrixFromSkeletonToGeometry(),
                                 &normalSrc->front(),
                                 &normalDst->front());
        normalDst->dirty();
    }

}
