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

typedef std::vector<RigTransformSoftware::BonePtrWeight> BonePtrWeightList;

void RigTransformSoftware::buildMinimumUpdateSet( const BoneMap&boneMap, const RigGeometry&rig ){

    ///1 Create Index2Vec<BoneWeight>
    const VertexInfluenceMap &vertexInfluenceMap=*rig.getInfluenceMap();
    std::vector<BonePtrWeightList> perVertexInfluences;
    perVertexInfluences.resize(rig.getSourceGeometry()->getVertexArray()->getNumElements());

    for (osgAnimation::VertexInfluenceMap::const_iterator perBoneinfit = vertexInfluenceMap.begin();
            perBoneinfit != vertexInfluenceMap.end();
            ++perBoneinfit)
    {
        const IndexWeightList& inflist = perBoneinfit->second;
        const std::string& bonename = perBoneinfit->first;

        if (bonename.empty()) {
            OSG_WARN << "RigTransformSoftware::VertexInfluenceMap contains unamed bone IndexWeightList" << std::endl;
        }
        BoneMap::const_iterator bmit = boneMap.find(bonename);
        if (bmit == boneMap.end() )
        {
            if (_invalidInfluence.find(bonename) != _invalidInfluence.end()) {
                _invalidInfluence[bonename] = true;
                OSG_WARN << "RigTransformSoftware Bone " << bonename << " not found, skip the influence group " << std::endl;
            }
            continue;
        }
        Bone* bone = bmit->second.get();
        for(IndexWeightList::const_iterator infit=inflist.begin(); infit!=inflist.end(); ++infit)
        {
            const VertexIndexWeight &iw = *infit;
            const unsigned int &index = iw.first;
            float weight = iw.second;
            perVertexInfluences[index].push_back(BonePtrWeight(bone, weight));
        }
    }

    // normalize _vertex2Bones weight per vertex
    unsigned vertexID=0;
    for (std::vector<BonePtrWeightList>::iterator it = perVertexInfluences.begin(); it != perVertexInfluences.end(); ++it, ++vertexID)
    {
        BonePtrWeightList& bones = *it;
        float sum = 0;
        for(BonePtrWeightList::iterator bwit=bones.begin();bwit!=bones.end();++bwit)
            sum += bwit->getWeight();
        if (sum < 1e-4)
        {
            OSG_WARN << "VertexInfluenceSet::buildVertex2BoneList warning the vertex " << vertexID << " seems to have 0 weight, skip normalize for this vertex" << std::endl;
        }
        else
        {
            float mult = 1.0/sum;
            for(BonePtrWeightList::iterator bwit=bones.begin();bwit!=bones.end();++bwit)
                bwit->setWeight(bwit->getWeight() * mult);
        }
    }

    ///2 Create inverse mapping Vec<BoneWeight>2Vec<Index> from previous built Index2Vec<BoneWeight>
    ///in order to minimize weighted matrices computation on update
    _uniqVertexGroupList.clear();

    typedef std::map<BonePtrWeightList, VertexGroup> UnifyBoneGroup;
    UnifyBoneGroup unifyBuffer;
    vertexID=0;
    for (std::vector<BonePtrWeightList>::iterator perVertinfit = perVertexInfluences.begin(); perVertinfit!=perVertexInfluences.end(); ++perVertinfit,++vertexID)
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
        _uniqVertexGroupList.push_back(it->second);
    OSG_INFO << "uniq groups " << _uniqVertexGroupList.size() << " for " << rig.getName() << std::endl;
}

bool RigTransformSoftware::prepareData(RigGeometry&rig) {
    ///find skeleton if not set
    if(!rig.getSkeleton() && !rig.getParents().empty())
    {
        RigGeometry::FindNearestParentSkeleton finder;
        if(rig.getParents().size() > 1)
            osg::notify(osg::WARN) << "A RigGeometry should not have multi parent ( " << rig.getName() << " )" << std::endl;
        rig.getParents()[0]->accept(finder);

        if(!finder._root.valid())
        {
            osg::notify(osg::WARN) << "A RigGeometry did not find a parent skeleton for RigGeometry ( " << rig.getName() << " )" << std::endl;
            return false;
        }
        rig.setSkeleton(finder._root.get());
    }
    if(!rig.getSkeleton())
        return false;
    ///get bonemap from skeleton
    BoneMapVisitor mapVisitor;
    rig.getSkeleton()->accept(mapVisitor);
    BoneMap boneMap = mapVisitor.getBoneMap();

    /// build minimal set of VertexGroup
    buildMinimumUpdateSet(boneMap,rig);

    ///set geom as it source
    if (rig.getSourceGeometry())
        rig.copyFrom(*rig.getSourceGeometry());


    osg::Vec3Array* normalSrc = dynamic_cast<osg::Vec3Array*>(rig.getSourceGeometry()->getNormalArray());
    osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(rig.getSourceGeometry()->getVertexArray());

    if(!(positionSrc) || positionSrc->empty() )
        return false;
    if(normalSrc&& normalSrc->size()!=positionSrc->size())
        return false;

    /// setup Vertex and Normal arrays with copy of sources
    rig.setVertexArray(new osg::Vec3Array);
    osg::Vec3Array* positionDst =new osg::Vec3Array;
    rig.setVertexArray(positionDst);
    *positionDst=*positionSrc;
    positionDst->setDataVariance(osg::Object::DYNAMIC);

    if(normalSrc) {
        osg::Vec3Array* normalDst =new osg::Vec3Array;
        *normalDst=*normalSrc;
        rig.setNormalArray(normalDst, osg::Array::BIND_PER_VERTEX);
        normalDst->setDataVariance(osg::Object::DYNAMIC);
    }

    _needInit = false;
    return true;
}

void RigTransformSoftware::operator()(RigGeometry& geom)
{
    if (_needInit)
        if (!prepareData(geom))
            return;

    if (!geom.getSourceGeometry()) {
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
