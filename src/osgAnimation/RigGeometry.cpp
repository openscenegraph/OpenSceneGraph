/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 * 
 * Cedric Pinson <mornifle@plopbyte.net>
 *
 */
#include <osgAnimation/RigGeometry>

using namespace osgAnimation;

RigGeometry::RigGeometry()
{
    setUseDisplayList(false);
    setUpdateCallback(new UpdateVertex);
    setDataVariance(osg::Object::DYNAMIC);
    _needToComputeMatrix = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::Matrix::identity();
}

RigGeometry::RigGeometry(const osg::Geometry& b) : osg::Geometry(b, osg::CopyOp::SHALLOW_COPY)
{
    setUseDisplayList(false);
    setUpdateCallback(new UpdateVertex);
    setDataVariance(osg::Object::DYNAMIC);
    _needToComputeMatrix = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::Matrix::identity();
}

RigGeometry::RigGeometry(const RigGeometry& b, const osg::CopyOp& copyop) : 
    osg::Geometry(b,copyop),
    _positionSource(b._positionSource),
    _normalSource(b._normalSource),
    _vertexInfluenceSet(b._vertexInfluenceSet),
    _vertexInfluenceMap(b._vertexInfluenceMap),
    _transformVertexes(b._transformVertexes),
    _needToComputeMatrix(b._needToComputeMatrix) 
{
}

void RigGeometry::buildTransformer(Skeleton* root)
{
    Bone::BoneMap bm = root->getBoneMap();
    _transformVertexes.init(bm, _vertexInfluenceSet.getUniqVertexSetToBoneSetList());
    _root = root;
}

void RigGeometry::buildVertexSet() 
{
    if (!_vertexInfluenceMap.valid()) 
    {
        osg::notify(osg::WARN) << "buildVertexSet can't be called without VertexInfluence already set to the RigGeometry ( " << getName() << " ) " << std::endl;
        return;
    }
    _vertexInfluenceSet.clear();
    for (osgAnimation::VertexInfluenceMap::iterator it = _vertexInfluenceMap->begin(); 
         it != _vertexInfluenceMap->end(); 
         it++)
        _vertexInfluenceSet.addVertexInfluence(it->second);

    _vertexInfluenceSet.buildVertex2BoneList();
    _vertexInfluenceSet.buildUniqVertexSetToBoneSetList();
    std::cout << "uniq groups " << _vertexInfluenceSet.getUniqVertexSetToBoneSetList().size() << " for " << getName() << std::endl;
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

void RigGeometry::transformSoftwareMethod()
{
    setUseDisplayList(false);
    setUseVertexBufferObjects(true);

    //    std::cout << getName() << " _matrixFromSkeletonToGeometry" << _matrixFromSkeletonToGeometry << std::endl;
    osg::Vec3Array* pos = dynamic_cast<osg::Vec3Array*>(getVertexArray());
    if (pos && _positionSource.size() != pos->size()) 
    {
        _positionSource = std::vector<osg::Vec3>(pos->begin(),pos->end());
        getVertexArray()->setDataVariance(osg::Object::DYNAMIC);
    }
    osg::Vec3Array* normal = dynamic_cast<osg::Vec3Array*>(getNormalArray());
    if (normal && _normalSource.size() != normal->size()) 
    {
        _normalSource = std::vector<osg::Vec3>(normal->begin(),normal->end());
        getNormalArray()->setDataVariance(osg::Object::DYNAMIC);
    }

    if (!_positionSource.empty()) 
    {
        _transformVertexes.compute<osg::Vec3>(_matrixFromSkeletonToGeometry, _invMatrixFromSkeletonToGeometry,  &_positionSource.front(), &pos->front());
        pos->dirty();
    }
    if (!_normalSource.empty()) 
    {
        _transformVertexes.computeNormal<osg::Vec3>(_matrixFromSkeletonToGeometry, _invMatrixFromSkeletonToGeometry, &_normalSource.front(), &normal->front());
        normal->dirty();
    }
    if (getUseDisplayList())
        dirtyDisplayList();
    dirtyBound();
}

const osgAnimation::Skeleton* RigGeometry::getSkeleton() const { return _root.get(); }
osgAnimation::Skeleton* RigGeometry::getSkeleton() { return _root.get(); }
