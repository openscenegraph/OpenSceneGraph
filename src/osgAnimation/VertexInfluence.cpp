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
        BoneInfluenceList &curvecinf=mapit->second;
        for(BoneInfluenceList::iterator curinf=curvecinf.begin(); curinf!=curvecinf.end(); ++curinf) {
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
    for(VertexInfluenceMap::iterator mapit=this->begin(); mapit!=this->end(); ++mapit) {
        BoneInfluenceList &curvecinf=mapit->second;
        for(BoneInfluenceList::iterator curinf=curvecinf.begin(); curinf!=curvecinf.end(); ++curinf) {
            IndexWeight& inf=*curinf;
            if( curvecinf.getBoneName().empty()) {
                OSG_WARN << "VertexInfluenceSet::buildVertex2BoneList warning vertex " << inf.first << " is not assigned to a bone" << std::endl;
            }
            else if(inf.second>minweight)tempVec2Bones[inf.first].insert(BoneWeight(curvecinf.getBoneName(), inf.second));
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
                    BoneInfluenceList & inf= (*this)[bwit->getBoneName()];
                    inf.setBoneName(bwit->getBoneName());
                    inf.push_back(IndexWeight(mapit->first, bwit->getWeight()*sum));
                }
            }
        }else{
            for(BoneWeightOrdered::iterator bwit=bwset.begin(); bwit!=bwset.end(); ++bwit) {
                BoneInfluenceList & inf= (*this)[bwit->getBoneName()];
                inf.setBoneName(bwit->getBoneName());
                inf.push_back(IndexWeight(mapit->first,bwit->getWeight()));
            }

        }
    }
}
