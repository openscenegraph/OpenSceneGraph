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
#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformSoftware>
#include <sstream>

using namespace osgAnimation;

// The idea is to compute a bounding box with a factor x of the first step we compute the bounding box
osg::BoundingBox RigComputeBoundingBoxCallback::computeBound(const osg::Drawable& drawable) const
{
    const osgAnimation::RigGeometry& rig = dynamic_cast<const osgAnimation::RigGeometry&>(drawable);

    // if a valid initial bounding box is set we use it without asking more
    if (rig.getInitialBound().valid())
        return rig.getInitialBound();

    if (_computed)
        return _boundingBox;

    // if the computing of bb is invalid (like no geometry inside)
    // then don't tag the bounding box as computed
    osg::BoundingBox bb = rig.computeBoundingBox();
    if (!bb.valid())
        return bb;


    _boundingBox.expandBy(bb);
    osg::Vec3 center = _boundingBox.center();
    osg::Vec3 vec = (_boundingBox._max-center)*_factor;
    _boundingBox.expandBy(center + vec);
    _boundingBox.expandBy(center - vec);
    _computed = true;
//        OSG_NOTICE << "build the bounding box for RigGeometry " << rig.getName() << " " << _boundingBox._min << " " << _boundingBox._max << std::endl;
    return _boundingBox;
}


RigGeometry::RigGeometry()
{
    _supportsDisplayList = false;
    setUseVertexBufferObjects(true);
    setUpdateCallback(new UpdateRigGeometry);
    setDataVariance(osg::Object::DYNAMIC);
    _needToComputeMatrix = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::Matrix::identity();
    // disable the computation of boundingbox for the rig mesh
    setComputeBoundingBoxCallback(new RigComputeBoundingBoxCallback());
    _rigTransformImplementation = new osgAnimation::RigTransformSoftware;

}


RigGeometry::RigGeometry(const RigGeometry& b, const osg::CopyOp& copyop) :
    osg::Geometry(b,copyop),
    _geometry(b._geometry),
    _rigTransformImplementation(osg::clone(b._rigTransformImplementation.get(), copyop)),
    _vertexInfluenceMap(b._vertexInfluenceMap),
    _needToComputeMatrix(b._needToComputeMatrix)
{
    _needToComputeMatrix = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::Matrix::identity();
    // disable the computation of boundingbox for the rig mesh

    setComputeBoundingBoxCallback(new RigComputeBoundingBoxCallback());
    // we don't copy the RigImplementation yet. because the RigImplementation need to be initialized in a valid graph, with a skeleton ...
    // don't know yet what to do with a clone of a RigGeometry

}


const osg::Matrix& RigGeometry::getMatrixFromSkeletonToGeometry() const { return _matrixFromSkeletonToGeometry; }
const osg::Matrix& RigGeometry::getInvMatrixFromSkeletonToGeometry() const { return _invMatrixFromSkeletonToGeometry;}


void RigGeometry::computeMatrixFromRootSkeleton()
{
    if (!_root.valid())
    {
        OSG_WARN << "Warning " << className() <<"::computeMatrixFromRootSkeleton if you have this message it means you miss to call buildTransformer(Skeleton* root), or your RigGeometry (" << getName() <<") is not attached to a Skeleton subgraph" << std::endl;
        return;
    }
    osg::MatrixList mtxList = getParent(0)->getWorldMatrices(_root.get());
    osg::Matrix notRoot = _root->getMatrix();
    _matrixFromSkeletonToGeometry = mtxList[0] * osg::Matrix::inverse(notRoot);
    _invMatrixFromSkeletonToGeometry = osg::Matrix::inverse(_matrixFromSkeletonToGeometry);
    _needToComputeMatrix = false;
}

void RigGeometry::update()
{
    RigTransform& implementation = *_rigTransformImplementation;
    (implementation)(*this);
}

void RigGeometry::copyFrom(osg::Geometry& from)
{
    if (this==&from) return;

    osg::Geometry& target = *this;

    target.setStateSet(from.getStateSet());

    // copy over primitive sets.
    target.getPrimitiveSetList() = from.getPrimitiveSetList();

    if (from.getVertexArray())
    {
        target.setVertexArray(from.getVertexArray());
    }

    if (from.getNormalArray())
    {
        target.setNormalArray(from.getNormalArray());
    }

    if (from.getColorArray())
    {
        target.setColorArray(from.getColorArray());
    }

    if (from.getSecondaryColorArray())
    {
        target.setSecondaryColorArray(from.getSecondaryColorArray());
    }

    if (from.getFogCoordArray())
    {
        target.setFogCoordArray(from.getFogCoordArray());
    }

    for(unsigned int ti=0;ti<from.getNumTexCoordArrays();++ti)
    {
        if (from.getTexCoordArray(ti))
        {
            target.setTexCoordArray(ti,from.getTexCoordArray(ti));
        }
    }

    osg::Geometry::ArrayList& arrayList = from.getVertexAttribArrayList();
    for(unsigned int vi=0;vi< arrayList.size();++vi)
    {
        osg::Array* array = arrayList[vi].get();
        if (array)
        {
            target.setVertexAttribArray(vi,array);
        }
    }
}




