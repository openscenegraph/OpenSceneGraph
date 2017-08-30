/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
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
#include <osg/Notify>
#include <iostream>
#include <algorithm>
#include <set>

using namespace osgAnimation;

struct invweight_ordered
{
    inline bool operator() (const BoneWeight& bw1, const BoneWeight& bw2)
    {
        if (bw1.getWeight() > bw2.getWeight())return true;
        if (bw1.getWeight() < bw2.getWeight())return false;
        return(bw1.getBoneName()<bw2.getBoneName());
    }
};

void VertexInfluenceMap::normalize(unsigned int numvert) {

    typedef std::pair<float, std::vector<float*> > PerVertWeights;
    std::vector<PerVertWeights > localstore;
    localstore.resize(numvert);
    for(VertexInfluenceMap::iterator mapit=this->begin(); mapit!=this->end(); ++mapit) {
        IndexWeightList &curvecinf=mapit->second;
        for(IndexWeightList::iterator curinf=curvecinf.begin(); curinf!=curvecinf.end(); ++curinf) {
            IndexWeight& inf=*curinf;
            localstore[inf.first].first+=inf.second;
            localstore[inf.first].second.push_back(&inf.second);

        }
    }
    unsigned int vertid=0;
    for(std::vector<PerVertWeights >::iterator itvert=localstore.begin(); itvert!=localstore.end(); ++itvert, ++vertid) {
        PerVertWeights & weights=*itvert;
        if(weights.first< 1e-4)
        {
            OSG_WARN << "VertexInfluenceMap::normalize warning the vertex " <<vertid << " seems to have 0 weight, skip normalize for this vertex" << std::endl;
        }
        else
        {
            float mult = 1.0/weights.first;
            for (std::vector<float*>::iterator itf =weights.second.begin(); itf!=weights.second.end(); ++itf)
                **itf*=mult;
        }
    }

}
///remove weakest influences in order to fit targetted numbonepervertex
void VertexInfluenceMap::cullInfluenceCountPerVertex(unsigned int numbonepervertex,float minweight, bool renormalize) {

    typedef std::set<BoneWeight,invweight_ordered >  BoneWeightOrdered;
    std::map<int,BoneWeightOrdered > tempVec2Bones;
    for(VertexInfluenceMap::iterator mapit=this->begin(); mapit!=this->end(); ++mapit)
    {
        const std::string& bonename=mapit->first;
        IndexWeightList &curvecinf=mapit->second;
        for(IndexWeightList::iterator curinf=curvecinf.begin(); curinf!=curvecinf.end(); ++curinf) {
            IndexWeight& inf=*curinf;
            if( bonename.empty()) {
                OSG_WARN << "VertexInfluenceSet::buildVertex2BoneList warning vertex " << inf.first << " is not assigned to a bone" << std::endl;
            }
            else if(inf.second>minweight)tempVec2Bones[inf.first].insert(BoneWeight(bonename, inf.second));
        }
    }
    this->clear();
    for( std::map<int,BoneWeightOrdered >::iterator mapit=tempVec2Bones.begin(); mapit!=tempVec2Bones.end(); ++mapit) {
        BoneWeightOrdered& bwset=mapit->second;
        unsigned int newsize=numbonepervertex<bwset.size()?numbonepervertex:bwset.size();
        float sum=0;
        while(bwset.size()>newsize)bwset.erase(*bwset.rbegin());
        if(renormalize){
            for(BoneWeightOrdered::iterator bwit=bwset.begin(); bwit!=bwset.end(); ++bwit)
                sum+=bwit->getWeight();
            if(sum>1e-4){
                sum=1.0f/sum;
                for(BoneWeightOrdered::iterator bwit=bwset.begin(); bwit!=bwset.end(); ++bwit) {
                    VertexInfluence & inf= (*this)[bwit->getBoneName()];
                    inf.push_back(IndexWeight(mapit->first, bwit->getWeight()*sum));
                    inf.setName(bwit->getBoneName());
                }
            }
        }else{
            for(BoneWeightOrdered::iterator bwit=bwset.begin(); bwit!=bwset.end(); ++bwit) {
                VertexInfluence & inf= (*this)[bwit->getBoneName()];
                inf.push_back(IndexWeight(mapit->first,bwit->getWeight()));
                inf.setName(bwit->getBoneName());
            }

        }
    }
}

void VertexInfluenceMap::computePerVertexInfluenceList(std::vector<BoneWeightList>& vertex2Bones,unsigned int numvert)const
{
  vertex2Bones.resize(numvert);
  for (osgAnimation::VertexInfluenceMap::const_iterator it = begin();
            it != end();
            ++it)
    {
        const IndexWeightList& inflist = it->second;
        if (it->first.empty()) {
            OSG_WARN << "RigTransformSoftware::VertexInfluenceMap contains unamed bone IndexWeightList" << std::endl;
        }
        for(IndexWeightList::const_iterator infit=inflist.begin(); infit!=inflist.end(); ++infit)
        {
            const IndexWeight &iw = *infit;
            const unsigned int &index = iw.getIndex();
            float weight = iw.getWeight();

            vertex2Bones[index].push_back(BoneWeight(it->first, weight));;
        }
    }
}

// sort by name and weight
struct SortByNameAndWeight : public std::less<BoneWeight>
{
    bool operator()(const BoneWeight& b0,
                    const BoneWeight& b1) const
    {
        if (b0.getBoneName() < b1.getBoneName())
            return true;
        else if (b0.getBoneName() > b1.getBoneName())
            return false;
        return (b0.getWeight() < b1.getWeight());
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
            if (SortByNameAndWeight()(b0[i], b1[i]))
                return true;
            else if (SortByNameAndWeight()(b1[i], b0[i]))
                return false;
        }
        return false;
    }
};
void VertexInfluenceMap::computeMinimalVertexGroupList(std::vector<VertexGroup>&uniqVertexGroupList,unsigned int numvert)const
{
    uniqVertexGroupList.clear();
    std::vector<BoneWeightList> vertex2Bones;
    computePerVertexInfluenceList(vertex2Bones,numvert);
    typedef std::map<BoneWeightList,VertexGroup, SortByBoneWeightList> UnifyBoneGroup;
    UnifyBoneGroup unifyBuffer;

    unsigned int vertexID=0;
    for (std::vector<BoneWeightList>::iterator it = vertex2Bones.begin(); it != vertex2Bones.end(); ++it,++vertexID)
    {
        BoneWeightList &boneweightlist = *it;//->second;
        //int vertexIndex = it->first;

        // sort the vector to have a consistent key
        std::sort(boneweightlist.begin(), boneweightlist.end(), SortByNameAndWeight());

        // we use the vector<BoneWeight> as key to differentiate group
        UnifyBoneGroup::iterator result = unifyBuffer.find(boneweightlist);
        if (result == unifyBuffer.end())
            unifyBuffer[boneweightlist].setBoneWeights(boneweightlist);
        unifyBuffer[boneweightlist].vertIDs().push_back(vertexID);
    }
    if(vertex2Bones.size()==unifyBuffer.size()) {
        OSG_WARN << "VertexInfluenceSet::buildmap is useless no duplicate VertexGroup" << std::endl;
    }
    uniqVertexGroupList.reserve(unifyBuffer.size());
    for (UnifyBoneGroup::iterator it = unifyBuffer.begin(); it != unifyBuffer.end(); ++it)
    {
        uniqVertexGroupList.push_back(it->second);
    }
}
