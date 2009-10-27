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


#include <osgAnimation/RigTransformSoftware>
#include <osgAnimation/RigGeometry>

using namespace osgAnimation;

bool RigTransformSoftware::init(RigGeometry& geom)
{
    if (!geom.getSkeleton())
        return false;
    Bone::BoneMap bm = geom.getSkeleton()->getBoneMap();
    initVertexSetFromBones(bm, geom.getVertexInfluenceSet().getUniqVertexSetToBoneSetList());
    _needInit = false;
    return true;
}

void RigTransformSoftware::update(RigGeometry& geom)
{
    osg::Vec3Array* pos = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
    if (pos && _positionSource.size() != pos->size()) 
    {
        _positionSource = std::vector<osg::Vec3>(pos->begin(),pos->end());
        geom.getVertexArray()->setDataVariance(osg::Object::DYNAMIC);
    }
    osg::Vec3Array* normal = dynamic_cast<osg::Vec3Array*>(geom.getNormalArray());
    if (normal && _normalSource.size() != normal->size()) 
    {
        _normalSource = std::vector<osg::Vec3>(normal->begin(),normal->end());
        geom.getNormalArray()->setDataVariance(osg::Object::DYNAMIC);
    }

    if (!_positionSource.empty()) 
    {
        compute<osg::Vec3>(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry(),  &_positionSource.front(), &pos->front());
        pos->dirty();
    }
    if (!_normalSource.empty()) 
    {
        computeNormal<osg::Vec3>(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry(), &_normalSource.front(), &normal->front());
        normal->dirty();
    }
}

void RigTransformSoftware::initVertexSetFromBones(const Bone::BoneMap& map, const VertexInfluenceSet::UniqVertexSetToBoneSetList& influence)
{
    _boneSetVertexSet.clear();

    int size = influence.size();
    _boneSetVertexSet.resize(size);
    for (int i = 0; i < size; i++) 
    {
        const VertexInfluenceSet::UniqVertexSetToBoneSet& inf = influence[i];
        int nbBones = inf.getBones().size();
        BoneWeightList& boneList = _boneSetVertexSet[i].getBones();

        double sumOfWeight = 0;
        for (int b = 0; b < nbBones; b++) 
        {
            const std::string& bname = inf.getBones()[b].getBoneName();
            float weight = inf.getBones()[b].getWeight();
            Bone::BoneMap::const_iterator it = map.find(bname);
            if (it == map.end()) 
            {
                osg::notify(osg::WARN) << "RigTransformSoftware Bone " << bname << " not found, skip the influence group " <<bname  << std::endl;
                continue;
            }
            Bone* bone = it->second.get();
            boneList.push_back(BoneWeight(bone, weight));
            sumOfWeight += weight;
        }
        // if a bone referenced by a vertexinfluence is missed it can make the sum less than 1.0
        // so we check it and renormalize the all weight bone
        const double threshold = 1e-4;
        if (!_boneSetVertexSet[i].getBones().empty() && 
            (sumOfWeight < 1.0 - threshold ||  sumOfWeight > 1.0 + threshold))
        {
            for (int b = 0; b < (int)boneList.size(); b++)
                boneList[b].setWeight(boneList[b].getWeight() / sumOfWeight);
        }
        _boneSetVertexSet[i].getVertexes() = inf.getVertexes();
    }
}
