/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
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
#include <osgAnimation/RigGeometry>
#include <osgAnimation/BoneMapVisitor>
#include <osg/Notify>
#include <iostream>
#include <algorithm>
#include <set>

using namespace osgAnimation;

struct invweight_ordered
{
    inline bool operator() (const BoneWeight& bw1, const BoneWeight& bw2) const
    {
        if (bw1.second > bw2.second)return true;
        if (bw1.second < bw2.second)return false;
        return(bw1.first < bw2.first);
    }
};

void VertexInfluenceMap::normalize(unsigned int numvert)
{

    typedef std::pair<float, std::vector<float*> > PerVertWeights;
    std::vector<PerVertWeights > localstore;
    localstore.resize(numvert);
    for(VertexInfluenceMap::iterator mapit = this->begin(); mapit != this->end(); ++mapit)
    {
        IndexWeightList &curvecinf = mapit->second;
        for(IndexWeightList::iterator curinf = curvecinf.begin(); curinf != curvecinf.end(); ++curinf)
        {
            VertexIndexWeight& inf = *curinf;
            localstore[inf.first].first += inf.second;
            localstore[inf.first].second.push_back(&inf.second);
        }
    }
    
    unsigned int vertid = 0;
    for(std::vector<PerVertWeights >::iterator itvert = localstore.begin();
        itvert != localstore.end();
        ++itvert, ++vertid)
    {
        PerVertWeights & weights = *itvert;
        if(weights.first< 1e-4)
        {
            OSG_WARN << "VertexInfluenceMap::normalize warning the vertex " <<vertid << " seems to have 0 weight, skip normalize for this vertex" << std::endl;
        }
        else
        {
            float mult = 1.0/weights.first;
            for (std::vector<float*>::iterator itf = weights.second.begin(); itf != weights.second.end(); ++itf)
            {
                **itf *= mult;
            }
        }
    }

}
///remove weakest influences in order to fit targeted numbonepervertex
void VertexInfluenceMap::cullInfluenceCountPerVertex(unsigned int numbonepervertex,float minweight, bool renormalize)
{

    typedef std::set<BoneWeight, invweight_ordered >  BoneWeightOrdered;
    std::map<int, BoneWeightOrdered > tempVec2Bones;
    for(VertexInfluenceMap::iterator mapit = this->begin(); mapit != this->end(); ++mapit)
    {
        const std::string& bonename = mapit->first;
        IndexWeightList &curvecinf = mapit->second;
        for(IndexWeightList::iterator curinf = curvecinf.begin(); curinf != curvecinf.end(); ++curinf)
        {
            VertexIndexWeight& inf = *curinf;
            if( bonename.empty())
            {
                OSG_WARN << "VertexInfluenceSet::cullInfluenceCountPerVertex warning vertex " << inf.first << " is not assigned to a bone" << std::endl;
            }
            else if(inf.second>minweight)
            {
                tempVec2Bones[inf.first].insert(BoneWeight(bonename, inf.second));
            }
        }
    }
    this->clear();
    for( std::map<int,BoneWeightOrdered >::iterator mapit = tempVec2Bones.begin(); mapit != tempVec2Bones.end(); ++mapit)
    {
        BoneWeightOrdered& bwset = mapit->second;
        unsigned int newsize = numbonepervertex<bwset.size()?numbonepervertex:bwset.size();
        float sum = 0.0f;
        while(bwset.size()>newsize)bwset.erase(*bwset.rbegin());
        if(renormalize)
        {
            for(BoneWeightOrdered::iterator bwit = bwset.begin(); bwit != bwset.end(); ++bwit)
            {
                sum += bwit->second;
            }
            
            if(sum > 1e-4)
            {
                sum = 1.0f/sum;
                for(BoneWeightOrdered::iterator bwit = bwset.begin(); bwit != bwset.end(); ++bwit)
                {
                    VertexInfluence & inf = (*this)[bwit->first];
                    inf.push_back(VertexIndexWeight(mapit->first, bwit->second*sum));
                    inf.setName(bwit->first);
                }
            }
        }
        else
        {
            for(BoneWeightOrdered::iterator bwit = bwset.begin(); bwit != bwset.end(); ++bwit)
            {
                VertexInfluence & inf = (*this)[bwit->first];
                inf.push_back(VertexIndexWeight(mapit->first,bwit->second));
                inf.setName(bwit->first);
            }
        }
    }
}

void VertexInfluenceMap::computePerVertexInfluenceList(std::vector<BoneWeightList>& vertex2Bones,unsigned int numvert)const
{
    vertex2Bones.resize(numvert);
    for (osgAnimation::VertexInfluenceMap::const_iterator it = begin(); it != end(); ++it)
    {
        const IndexWeightList& inflist = it->second;
        if (it->first.empty())
        {
            OSG_WARN << "VertexInfluenceMap::computePerVertexInfluenceList contains unnamed bone IndexWeightList" << std::endl;
        }
        
        for(IndexWeightList::const_iterator infit = inflist.begin(); infit != inflist.end(); ++infit)
        {
            const VertexIndexWeight &iw = *infit;
            const unsigned int &index = iw.first;
            const float &weight = iw.second;
            vertex2Bones[index].push_back(BoneWeight(it->first, weight));;
        }
    }
}

// sort by name and weight
struct SortByNameAndWeight : public std::less<BoneWeight>
{
    bool operator()(const BoneWeight& b0, const BoneWeight& b1) const
    {
        if (b0.first < b1.first)
            return true;
        else if (b0.first > b1.first)
            return false;
            
        return (b0.second < b1.second);
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
void VertexInfluenceMap::computeMinimalVertexGroupList(std::vector<VertexGroup>& uniqVertexGroupList, unsigned int numvert) const
{
    uniqVertexGroupList.clear();
    std::vector<BoneWeightList> vertex2Bones;
    computePerVertexInfluenceList(vertex2Bones,numvert);
    typedef std::map<BoneWeightList,VertexGroup, SortByBoneWeightList> UnifyBoneGroup;
    UnifyBoneGroup unifyBuffer;

    unsigned int vertexID = 0;
    for (std::vector<BoneWeightList>::iterator it = vertex2Bones.begin(); it != vertex2Bones.end(); ++it,++vertexID)
    {
        BoneWeightList &boneweightlist = *it;
        // sort the vector to have a consistent key
        std::sort(boneweightlist.begin(), boneweightlist.end(), SortByNameAndWeight());
        
        // we use the vector<BoneWeight> as key to differentiate group
        UnifyBoneGroup::iterator result = unifyBuffer.find(boneweightlist);
        if (result == unifyBuffer.end())
        {
            unifyBuffer[boneweightlist].setBoneWeights(boneweightlist);
        }
        
        unifyBuffer[boneweightlist].vertIDs().push_back(vertexID);
    }
    
    if(vertex2Bones.size() == unifyBuffer.size())
    {
        OSG_WARN << "VertexInfluenceMap::computeMinimalVertexGroupList is useless no duplicate VertexGroup" << std::endl;
    }
    
    uniqVertexGroupList.reserve(unifyBuffer.size());
    for (UnifyBoneGroup::iterator it = unifyBuffer.begin(); it != unifyBuffer.end(); ++it)
    {
        uniqVertexGroupList.push_back(it->second);
    }
}

//Experimental Bone removal stuff
typedef std::vector<osgAnimation::RigGeometry*> RigList;
class CollectRigVisitor : public osg::NodeVisitor
{
public:
    META_NodeVisitor(osgAnimation, CollectRigVisitor)
    CollectRigVisitor();

    void apply(osg::Geometry& node);
    inline const RigList& getRigList() const{return _map;}

protected:
    RigList _map;
};

CollectRigVisitor::CollectRigVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

void CollectRigVisitor::apply(osg::Geometry& node)
{
    RigGeometry* rig = dynamic_cast<RigGeometry*>(&node);
    if ( rig )
        _map.push_back(rig);
}

bool recursiveisUsefull( Bone* bone, std::set<std::string> foundnames)
{
    for(unsigned int i=0; i<bone->getNumChildren(); ++i)
    {
        Bone* child = dynamic_cast< Bone* >(bone->getChild(i));
        if(child)
        {
            if( foundnames.find(child->getName()) != foundnames.end() )
                return true;
            if( recursiveisUsefull(child,foundnames) ) 
                return true;
        }
    }
    return false;
}

void VertexInfluenceMap::removeUnexpressedBones(Skeleton &skel) const
{
    BoneMapVisitor mapVisitor;
    skel.accept(mapVisitor);

    CollectRigVisitor rigvis;
    skel.accept(rigvis);

    RigList  rigs = rigvis.getRigList();
    BoneMap boneMap = mapVisitor.getBoneMap();

    unsigned int removed=0;
    Bone* child, *par;

    std::set<std::string> usebones;
    for(RigList::iterator rigit = rigs.begin(); rigit != rigs.end(); ++rigit)
    {
        for(VertexInfluenceMap::iterator mapit = (*rigit)->getInfluenceMap()->begin();
            mapit != (*rigit)->getInfluenceMap()->end();
            ++mapit)
        {
            usebones.insert((*mapit).first);
        }
    }
  
    for(BoneMap::iterator bmit = boneMap.begin(); bmit != boneMap.end();)
    {
        if(usebones.find(bmit->second->getName()) == usebones.end())
        {
            if( !(par = bmit->second->getBoneParent()) )
            {
                ++bmit;
                continue;
            }

            Bone * bone2rm = bmit->second.get();

            if( recursiveisUsefull(bone2rm,usebones))
            {
                ++bmit;
                continue;
            }

            ///Bone can be removed
            ++ removed;
            OSG_INFO<<"removing useless bone: "<< bone2rm->getName() <<std::endl;
            osg::NodeList nodes;

            for(unsigned int numchild = 0; numchild < bone2rm->getNumChildren(); numchild++)
            {
                if( (child = dynamic_cast<Bone*>(bone2rm->getChild(numchild))) )
                {
                    if(par!=child &&child!=bone2rm)
                    {
                        par->addChild(child);
                        nodes.push_back(child);
                    }
                }
            }
            
            for(unsigned int i=0; i<nodes.size(); ++i)
            {
                bone2rm->removeChild(nodes[i]);
            }
            par->removeChild(bone2rm);

            ///rebuild bonemap after bone removal
            BoneMapVisitor mapVis ; 
            skel.accept(mapVis);
            boneMap = mapVis.getBoneMap();
            bmit = boneMap.begin(); 
                 
        }
        else 
        {
            ++bmit;
        }
    }
    OSG_WARN<<"Number of bone removed "<<removed<<std::endl;
}
