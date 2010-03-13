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

using namespace osgAnimation;

RigTransformSoftware::RigTransformSoftware()
{
    _needInit = true;
}

bool RigTransformSoftware::init(RigGeometry& geom)
{
    if (!geom.getSkeleton())
        return false;

    BoneMapVisitor mapVisitor;
    geom.getSkeleton()->accept(mapVisitor);
    BoneMap bm = mapVisitor.getBoneMap();
    initVertexSetFromBones(bm, geom.getVertexInfluenceSet().getUniqVertexSetToBoneSetList());

    if (geom.getSourceGeometry())
        geom.copyFrom(*geom.getSourceGeometry());
    geom.setVertexArray(0);
    geom.setNormalArray(0);

    _needInit = false;
    return true;
}

void RigTransformSoftware::operator()(RigGeometry& geom)
{
    if (_needInit)
        if (!init(geom))
            return;

    if (!geom.getSourceGeometry()) {
        osg::notify(osg::WARN) << this << " RigTransformSoftware no source geometry found on RigGeometry" << std::endl;
        return;
    }
    osg::Geometry& source = *geom.getSourceGeometry();
    osg::Geometry& destination = geom;

    osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(source.getVertexArray());
    osg::Vec3Array* positionDst = dynamic_cast<osg::Vec3Array*>(destination.getVertexArray());
    if (positionSrc && (!positionDst || (positionDst->size() != positionSrc->size()) ) )
    {
        if (!positionDst)
        {
            positionDst = new osg::Vec3Array;
            positionDst->setDataVariance(osg::Object::DYNAMIC);
            destination.setVertexArray(positionDst);
        }
        *positionDst = *positionSrc;
    }
    
    osg::Vec3Array* normalSrc = dynamic_cast<osg::Vec3Array*>(source.getNormalArray());
    osg::Vec3Array* normalDst = dynamic_cast<osg::Vec3Array*>(destination.getNormalArray());
    if (normalSrc && (!normalDst || (normalDst->size() != normalSrc->size()) ) )
    {
        if (!normalDst)
        {
            normalDst = new osg::Vec3Array;
            normalDst->setDataVariance(osg::Object::DYNAMIC);
            destination.setNormalArray(normalDst);
            destination.setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        }
        *normalDst = *normalSrc;
    }

    if (positionDst && !positionDst->empty())
    {
        compute<osg::Vec3>(geom.getMatrixFromSkeletonToGeometry(), 
                           geom.getInvMatrixFromSkeletonToGeometry(),  
                           &positionSrc->front(),
                           &positionDst->front());
        positionDst->dirty();
    }

    if (normalDst && !normalDst->empty())
    {
        computeNormal<osg::Vec3>(geom.getMatrixFromSkeletonToGeometry(), 
                           geom.getInvMatrixFromSkeletonToGeometry(),  
                           &normalSrc->front(),
                           &normalDst->front());
        normalDst->dirty();
    }
}

void RigTransformSoftware::initVertexSetFromBones(const BoneMap& map, const VertexInfluenceSet::UniqVertexSetToBoneSetList& influence)
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
            BoneMap::const_iterator it = map.find(bname);
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
