/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

// sort by name and weight
struct SortByNameAndWeight : public std::less<VertexInfluenceSet::BoneWeight>
{
    bool operator()(const VertexInfluenceSet::BoneWeight& b0,
                    const VertexInfluenceSet::BoneWeight& b1) const
    {
        if (b0.getBoneName() < b1.getBoneName())
            return true;
        else if (b0.getBoneName() > b1.getBoneName())
            return false;
        if (b0.getWeight() < b1.getWeight())
            return true;
        return false;
    }
};

struct SortByBoneWeightList : public std::less<VertexInfluenceSet::BoneWeightList>
{
    bool operator()(const VertexInfluenceSet::BoneWeightList& b0,
                    const VertexInfluenceSet::BoneWeightList& b1) const
    {
        if (b0.size() < b1.size())
            return true;
        else if (b0.size() > b1.size())
            return false;

        int size = b0.size();
        for (int i = 0; i < size; i++)
        {
            bool result = SortByNameAndWeight()(b0[i], b1[i]);
            if (result)
                return true;
            else if (SortByNameAndWeight()(b1[i], b0[i]))
                return false;
        }
        return false;
    }
};

bool RigTransformSoftware::init(RigGeometry& geom)
{
    if (!geom.getSkeleton())
        return false;

    BoneMapVisitor mapVisitor;
    geom.getSkeleton()->accept(mapVisitor);
    BoneMap bm = mapVisitor.getBoneMap();
    initVertexSetFromBones(bm, geom.getVertexInfluenceSet().getUniqVertexGroupList());

    if (geom.getSourceGeometry())
        geom.copyFrom(*geom.getSourceGeometry());


    osg::Vec3Array* normalSrc = dynamic_cast<osg::Vec3Array*>(geom.getSourceGeometry()->getNormalArray());
    osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(geom.getSourceGeometry()->getVertexArray());

    if(!(positionSrc) || positionSrc->empty() )
        return false;
    if(normalSrc&& normalSrc->size()!=positionSrc->size())
        return false;


    geom.setVertexArray(new osg::Vec3Array);
    osg::Vec3Array* positionDst =new osg::Vec3Array;
    geom.setVertexArray(positionDst);
    *positionDst=*positionSrc;
    positionDst->setDataVariance(osg::Object::DYNAMIC);


    if(normalSrc){
        osg::Vec3Array* normalDst =new osg::Vec3Array;
        *normalDst=*normalSrc;
        geom.setNormalArray(normalDst, osg::Array::BIND_PER_VERTEX);
        normalDst->setDataVariance(osg::Object::DYNAMIC);
    }

    _needInit = false;
    return true;
}

void RigTransformSoftware::operator()(RigGeometry& geom)
{
    if (_needInit)
        if (!init(geom))
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

///convert BoneWeight to BonePtrWeight using bonemap
void RigTransformSoftware::initVertexSetFromBones(const BoneMap& map, const VertexInfluenceSet::UniqVertexGroupList& vertexgroups)
{
    _uniqInfluenceSet2VertIDList.clear();

    int size = vertexgroups.size();
    _uniqInfluenceSet2VertIDList.resize(size);
    //for (VertexInfluenceSet::UniqVertexGroupList::const_iterator vgit=vertexgroups.begin();         vgit!=vertexgroups.end();vgit++)
    for(int i = 0; i < size; i++)
    {
        const VertexInfluenceSet::VertexGroup& vg = vertexgroups[i];
        int nbBones = vg.getBones().size();
        BonePtrWeightList& boneList = _uniqInfluenceSet2VertIDList[i].getBoneWeights();

        double sumOfWeight = 0;
        for (int b = 0; b < nbBones; b++)
        {
            const std::string& bname = vg.getBones()[b].getBoneName();
            float weight = vg.getBones()[b].getWeight();
            BoneMap::const_iterator it = map.find(bname);
            if (it == map.end() )
            {
                if (_invalidInfluence.find(bname) != _invalidInfluence.end()) {
                    _invalidInfluence[bname] = true;
                    OSG_WARN << "RigTransformSoftware Bone " << bname << " not found, skip the influence group " <<bname  << std::endl;
                }
                continue;
            }
            Bone* bone = it->second.get();
            boneList.push_back(BonePtrWeight(bone, weight));
            sumOfWeight += weight;
        }
        // if a bone referenced by a vertexinfluence is missed it can make the sum less than 1.0
        // so we check it and renormalize the all weight bone
        /*const double threshold = 1e-4;
        if (!_boneSetVertexSet[i].getBones().empty() &&
            (sumOfWeight < 1.0 - threshold ||  sumOfWeight > 1.0 + threshold))
        {
            for (int b = 0; b < (int)boneList.size(); b++)
                boneList[b].setWeight(boneList[b].getWeight() / sumOfWeight);
        }*/
        _uniqInfluenceSet2VertIDList[i].getVertexes() = vg.getVertexes();
    }
}
