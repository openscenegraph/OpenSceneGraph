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

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformSoftware>
#include <sstream>
#include <osg/GL2Extensions>

using namespace osgAnimation;

RigGeometry::RigGeometry()
{
    _supportsDisplayList = false;
    setUseVertexBufferObjects(true);
    setUpdateCallback(new UpdateVertex);
    setDataVariance(osg::Object::DYNAMIC);
    _needToComputeMatrix = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::Matrix::identity();

    // disable the computation of boundingbox for the rig mesh
    setComputeBoundingBoxCallback(new ComputeBoundingBoxCallback);
}

RigGeometry::RigGeometry(const osg::Geometry& b) : osg::Geometry(b, osg::CopyOp::SHALLOW_COPY)
{
    _supportsDisplayList = false;
    setUseVertexBufferObjects(true);
    setUpdateCallback(new UpdateVertex);
    setDataVariance(osg::Object::DYNAMIC);
    _needToComputeMatrix = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::Matrix::identity();

    // disable the computation of boundingbox for the rig mesh
    setComputeBoundingBoxCallback(new ComputeBoundingBoxCallback);
}

RigGeometry::RigGeometry(const RigGeometry& b, const osg::CopyOp& copyop) :
    osg::Geometry(b,copyop),
    _vertexInfluenceSet(b._vertexInfluenceSet),
    _vertexInfluenceMap(b._vertexInfluenceMap),
    _needToComputeMatrix(b._needToComputeMatrix)
{
    // we dont copy the RigImplementation yet. because the RigImplementation need to be initialized in a valid graph, with a skeleton ...
    // dont know yet what to do with a clone of a RigGeometry
}


const osg::Matrix& RigGeometry::getMatrixFromSkeletonToGeometry() const { return _matrixFromSkeletonToGeometry; }
const osg::Matrix& RigGeometry::getInvMatrixFromSkeletonToGeometry() const { return _invMatrixFromSkeletonToGeometry;}


void RigGeometry::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::Geometry::drawImplementation(renderInfo);
}

void RigGeometry::buildVertexInfluenceSet()
{
    if (!_vertexInfluenceMap.valid())
    {
        osg::notify(osg::WARN) << "buildVertexInfluenceSet can't be called without VertexInfluence already set to the RigGeometry ( " << getName() << " ) " << std::endl;
        return;
    }
    _vertexInfluenceSet.clear();
    for (osgAnimation::VertexInfluenceMap::iterator it = _vertexInfluenceMap->begin(); 
         it != _vertexInfluenceMap->end(); 
         it++)
        _vertexInfluenceSet.addVertexInfluence(it->second);

    _vertexInfluenceSet.buildVertex2BoneList();
    _vertexInfluenceSet.buildUniqVertexSetToBoneSetList();
    osg::notify(osg::NOTICE) << "uniq groups " << _vertexInfluenceSet.getUniqVertexSetToBoneSetList().size() << " for " << getName() << std::endl;
}

void RigGeometry::computeMatrixFromRootSkeleton()
{
    if (!_root.valid())
    {
        osg::notify(osg::WARN) << "Warning " << className() <<"::computeMatrixFromRootSkeleton if you have this message it means you miss to call buildTransformer(Skeleton* root), or your RigGeometry (" << getName() <<") is not attached to a Skeleton subgraph" << std::endl;
        return;
    }
    osg::MatrixList mtxList = getParent(0)->getWorldMatrices(_root.get());
    _matrixFromSkeletonToGeometry = mtxList[0];
    _invMatrixFromSkeletonToGeometry = osg::Matrix::inverse(_matrixFromSkeletonToGeometry);
    _needToComputeMatrix = false;
}

void RigGeometry::update()
{
    if (!getRigTransformImplementation())
    {
        _rigTransformImplementation = new RigTransformSoftware;
    }

    if (getRigTransformImplementation()->needInit())
        if (!getRigTransformImplementation()->init(*this))
            return;
    getRigTransformImplementation()->update(*this);
}

const VertexInfluenceSet& RigGeometry::getVertexInfluenceSet() const { return _vertexInfluenceSet;}

const Skeleton* RigGeometry::getSkeleton() const { return _root.get(); }
Skeleton* RigGeometry::getSkeleton() { return _root.get(); }
void RigGeometry::setSkeleton(Skeleton* root) { _root = root;}
RigTransform* RigGeometry::getRigTransformImplementation() { return _rigTransformImplementation.get(); }
void RigGeometry::setRigTransformImplementation(RigTransform* rig) { _rigTransformImplementation = rig; }
