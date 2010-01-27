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

using namespace osgAnimation;

void VertexInfluenceSet::addVertexInfluence(const VertexInfluence& v) { _bone2Vertexes.push_back(v); }
const VertexInfluenceSet::VertexIndexToBoneWeightMap& VertexInfluenceSet::getVertexToBoneList() const { return _vertex2Bones;}
// this class manage VertexInfluence database by mesh
// reference bones per vertex ...
void VertexInfluenceSet::buildVertex2BoneList()
{
    _vertex2Bones.clear();
    for (BoneToVertexList::const_iterator it = _bone2Vertexes.begin(); it != _bone2Vertexes.end(); ++it) 
    {
        const VertexInfluence& vi = (*it);
        int size = vi.size();
        for (int i = 0; i < size; i++) 
        {
            VertexIndexWeight viw = vi[i];
            int index = viw.first;
            float weight = viw.second;
            if (vi.getName().empty())
                osg::notify(osg::WARN) << "VertexInfluenceSet::buildVertex2BoneList warning vertex " << index << " is not assigned to a bone" << std::endl;
            _vertex2Bones[index].push_back(BoneWeight(vi.getName(), weight));
        }
    }

    // normalize weight per vertex
    for (VertexIndexToBoneWeightMap::iterator it = _vertex2Bones.begin(); it != _vertex2Bones.end(); ++it)
    {
        BoneWeightList& bones = it->second;
        int size = bones.size();
        float sum = 0;
        for (int i = 0; i < size; i++)
            sum += bones[i].getWeight();
        if (sum < 1e-4) 
        {
            osg::notify(osg::WARN) << "VertexInfluenceSet::buildVertex2BoneList warning the vertex " << it->first << " seems to have 0 weight, skip normalize for this vertex" << std::endl;
        }
        else 
        {
            float mult = 1.0/sum;
            for (int i = 0; i < size; i++)
                bones[i].setWeight(bones[i].getWeight() * mult);
        }
    }
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

void VertexInfluenceSet::clear()
{
    _bone2Vertexes.clear(); 
    _uniqVertexSetToBoneSet.clear();
}

void VertexInfluenceSet::buildUniqVertexSetToBoneSetList()
{
    _uniqVertexSetToBoneSet.clear();

    typedef std::map<BoneWeightList,UniqVertexSetToBoneSet, SortByBoneWeightList> UnifyBoneGroup;
    UnifyBoneGroup unifyBuffer;

    for (VertexIndexToBoneWeightMap::iterator it = _vertex2Bones.begin(); it != _vertex2Bones.end(); ++it) 
    {
        BoneWeightList bones = it->second;
        int vertexIndex = it->first;

        // sort the vector to have a consistent key
        std::sort(bones.begin(), bones.end(), SortByNameAndWeight());
   
        // we use the vector<BoneWeight> as key to differentiate group
        UnifyBoneGroup::iterator result = unifyBuffer.find(bones);
        if (result == unifyBuffer.end())
            unifyBuffer[bones].setBones(bones);
        unifyBuffer[bones].getVertexes().push_back(vertexIndex);
    }

    _uniqVertexSetToBoneSet.reserve(unifyBuffer.size());
    for (UnifyBoneGroup::iterator it = unifyBuffer.begin(); it != unifyBuffer.end(); ++it) 
    {
        _uniqVertexSetToBoneSet.push_back(it->second);
    }
}

