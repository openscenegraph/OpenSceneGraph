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

typedef std::vector<RigTransformSoftware::BonePtrWeight> BoneWeightList;
// sort by name and weight
struct SortByNameAndWeight : public std::less<RigTransformSoftware::BonePtrWeight>
{
    bool operator()(const RigTransformSoftware::BonePtrWeight& b0,
                    const RigTransformSoftware::BonePtrWeight& b1) const
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

struct SortByBoneWeightList : public std::less<BoneWeightList>
{
    bool operator()(const BoneWeightList& b0,
                    const BoneWeightList& b1) const
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

void RigTransformSoftware::buildMinimumUpdateSet(const BoneMap&boneMap,const RigGeometry&rig ){

    ///1 Create Index2Vec<BoneWeight>
    std::vector<BoneWeightList> _vertex2Bones;
    _vertex2Bones.resize(rig.getSourceGeometry()->getVertexArray()->getNumElements());

    typedef std::pair<float,unsigned int> FloatInt;
    std::vector< FloatInt > sums;///stat totalweight nbref
    sums.resize(rig.getSourceGeometry()->getVertexArray()->getNumElements()
    );
    const VertexInfluenceMap *_vertexInfluenceMap=rig.getInfluenceMap();
    for (osgAnimation::VertexInfluenceMap::const_iterator it = _vertexInfluenceMap->begin();
            it != _vertexInfluenceMap->end();
            ++it)
    {
        const BoneInfluenceList& inflist = it->second;
        if (inflist.getBoneName().empty()) {
            OSG_WARN << "RigTransformSoftware::VertexInfluenceMap contains unamed bone BoneInfluenceList" << std::endl;
        }
        BoneMap::const_iterator bmit = boneMap.find(inflist.getBoneName());
        if (bmit == boneMap.end() )
        {
            if (_invalidInfluence.find(inflist.getBoneName()) != _invalidInfluence.end()) {
                _invalidInfluence[inflist.getBoneName()] = true;
                OSG_WARN << "RigTransformSoftware Bone " << inflist.getBoneName() << " not found, skip the influence group " << std::endl;
            }
            continue;
        }
        Bone* bone = bmit->second.get();
        for(BoneInfluenceList::const_iterator infit=inflist.begin(); infit!=inflist.end(); ++infit)
        {
            const IndexWeight &iw = *infit;
            const unsigned int &index = iw.getIndex();
            float weight = iw.getWeight();

            _vertex2Bones[index].push_back(BonePtrWeight(inflist.getBoneName(), weight,bone));;
        }
    }

    // normalize _vertex2Bones weight per vertex
    unsigned vertexID=0;
    for (std::vector<BoneWeightList>::iterator it = _vertex2Bones.begin(); it != _vertex2Bones.end(); ++it, ++vertexID)
    {
        BoneWeightList& bones = *it;
        float sum = 0;
        for(BoneWeightList::iterator bwit=bones.begin();bwit!=bones.end();++bwit)
            sum += bwit->getWeight();
        if (sum < 1e-4)
        {
            OSG_WARN << "VertexInfluenceSet::buildVertex2BoneList warning the vertex " << vertexID << " seems to have 0 weight, skip normalize for this vertex" << std::endl;
        }
        else
        {
            float mult = 1.0/sum;
            for(BoneWeightList::iterator bwit=bones.begin();bwit!=bones.end();++bwit)
                bwit->setWeight(bwit->getWeight() * mult);
        }
    }

    ///2 Create inverse mapping Vec<BoneWeight>2Vec<Index> from previous built Index2Vec<BoneWeight>
    ///in order to minimize weighted matrices computation on update
    typedef std::map<BoneWeightList, VertexGroup, SortByBoneWeightList> UnifyBoneGroup;
    UnifyBoneGroup unifyBuffer;
    vertexID=0;
    for (std::vector<BoneWeightList>::iterator it = _vertex2Bones.begin(); it != _vertex2Bones.end(); ++it,++vertexID)
    {
        BoneWeightList& bones = *it;
        // sort the vector to have a consistent key
        std::sort(bones.begin(), bones.end(), SortByNameAndWeight());
        // we use the vector<BoneWeight> as key to differentiate group
        UnifyBoneGroup::iterator result = unifyBuffer.find(bones);
        if (result == unifyBuffer.end())
            unifyBuffer[bones].getBoneWeights()=bones;
        unifyBuffer[bones].getVertexes().push_back(vertexID);
    }
    if(_vertex2Bones.size()==unifyBuffer.size()) {
        OSG_WARN << "RigTransformSoftware::build mapping is useless no duplicate VertexGroup : too much " <<_vertex2Bones.size()<<"=="<<unifyBuffer.size()<< std::endl;

    }
    _uniqInfluenceSet2VertIDList.reserve(unifyBuffer.size());

    for (UnifyBoneGroup::iterator it = unifyBuffer.begin(); it != unifyBuffer.end(); ++it)
        _uniqInfluenceSet2VertIDList.push_back(it->second);

    OSG_DEBUG << "uniq groups " << _uniqInfluenceSet2VertIDList.size() << " for " << rig.getName() << std::endl;
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
            boneList.push_back(BonePtrWeight(bone->getName(), weight, bone));
            sumOfWeight += weight;
        }
        // if a bone referenced by a vertexinfluence is missed it can make the sum less than 1.0
        // so we check it and renormalize the all weight bone
        const double threshold = 1e-4;
        if (!vg.getBones().empty() &&
            (sumOfWeight < 1.0 - threshold ||  sumOfWeight > 1.0 + threshold))
        {
            for (int b = 0; b < (int)boneList.size(); b++)
                boneList[b].setWeight(boneList[b].getWeight() / sumOfWeight);
        }
        _uniqInfluenceSet2VertIDList[i].getVertexes() = vg.getVertexes();
    }
}
