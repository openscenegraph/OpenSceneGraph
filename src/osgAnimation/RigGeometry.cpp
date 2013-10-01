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
#include <osg/GL2Extensions>

using namespace osgAnimation;

// The idea is to compute a bounding box with a factor x of the first step we compute the bounding box
class RigComputeBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
public:
    RigComputeBoundingBoxCallback(double factor = 2.0) : _computed(false), _factor(factor) {}
    void reset() { _computed = false; }
    virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const
    {
        const osgAnimation::RigGeometry& rig = dynamic_cast<const osgAnimation::RigGeometry&>(drawable);

        // if a valid inital bounding box is set we use it without asking more
        if (rig.getInitialBound().valid())
            return rig.getInitialBound();

        if (_computed)
            return _boundingBox;

        // if the computing of bb is invalid (like no geometry inside)
        // then dont tag the bounding box as computed
        osg::BoundingBox bb = rig.computeBound();
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
protected:
    mutable bool _computed;
    double _factor;
    mutable osg::BoundingBox _boundingBox;
};

RigGeometry::RigGeometry()
{
    _supportsDisplayList = false;
    setUseVertexBufferObjects(true);
    setUpdateCallback(new UpdateVertex);
    setDataVariance(osg::Object::DYNAMIC);
    _needToComputeMatrix = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::Matrix::identity();

    // disable the computation of boundingbox for the rig mesh
    setComputeBoundingBoxCallback(new RigComputeBoundingBoxCallback);
}


RigGeometry::RigGeometry(const RigGeometry& b, const osg::CopyOp& copyop) :
    osg::Geometry(b,copyop),
    _geometry(b._geometry),
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
        OSG_WARN << "buildVertexInfluenceSet can't be called without VertexInfluence already set to the RigGeometry ( " << getName() << " ) " << std::endl;
        return;
    }
    _vertexInfluenceSet.clear();
    for (osgAnimation::VertexInfluenceMap::iterator it = _vertexInfluenceMap->begin();
         it != _vertexInfluenceMap->end();
         ++it)
        _vertexInfluenceSet.addVertexInfluence(it->second);

    _vertexInfluenceSet.buildVertex2BoneList();
    _vertexInfluenceSet.buildUniqVertexSetToBoneSetList();
    OSG_DEBUG << "uniq groups " << _vertexInfluenceSet.getUniqVertexSetToBoneSetList().size() << " for " << getName() << std::endl;
}

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
    if (!getRigTransformImplementation())
    {
        _rigTransformImplementation = new RigTransformSoftware;
    }

    RigTransform& implementation = *getRigTransformImplementation();
    (implementation)(*this);
}

void RigGeometry::copyFrom(osg::Geometry& from)
{
    bool copyToSelf = (this==&from);

    osg::Geometry& target = *this;

    if (!copyToSelf) target.setStateSet(from.getStateSet());

    // copy over primitive sets.
    if (!copyToSelf) target.getPrimitiveSetList() = from.getPrimitiveSetList();

    if (from.getVertexArray())
    {
        if (!copyToSelf) target.setVertexArray(from.getVertexArray());
    }

    if (from.getNormalArray())
    {
        if (!copyToSelf) target.setNormalArray(from.getNormalArray());
    }

    if (from.getColorArray())
    {
        if (!copyToSelf) target.setColorArray(from.getColorArray());
    }

    if (from.getSecondaryColorArray())
    {
        if (!copyToSelf) target.setSecondaryColorArray(from.getSecondaryColorArray());
    }

    if (from.getFogCoordArray())
    {
        if (!copyToSelf) target.setFogCoordArray(from.getFogCoordArray());
    }

    for(unsigned int ti=0;ti<from.getNumTexCoordArrays();++ti)
    {
        if (from.getTexCoordArray(ti))
        {
            if (!copyToSelf) target.setTexCoordArray(ti,from.getTexCoordArray(ti));
        }
    }

    osg::Geometry::ArrayList& arrayList = from.getVertexAttribArrayList();
    for(unsigned int vi=0;vi< arrayList.size();++vi)
    {
        osg::Array* array = arrayList[vi].get();
        if (array)
        {
            if (!copyToSelf) target.setVertexAttribArray(vi,array);
        }
    }
}

const VertexInfluenceSet& RigGeometry::getVertexInfluenceSet() const { return _vertexInfluenceSet;}

const Skeleton* RigGeometry::getSkeleton() const { return _root.get(); }
Skeleton* RigGeometry::getSkeleton() { return _root.get(); }
void RigGeometry::setSkeleton(Skeleton* root) { _root = root;}
RigTransform* RigGeometry::getRigTransformImplementation() { return _rigTransformImplementation.get(); }
void RigGeometry::setRigTransformImplementation(RigTransform* rig) { _rigTransformImplementation = rig; }

osg::Geometry* RigGeometry::getSourceGeometry() { return _geometry.get(); }
const osg::Geometry* RigGeometry::getSourceGeometry() const { return _geometry.get(); }
void RigGeometry::setSourceGeometry(osg::Geometry* geometry) { _geometry = geometry; }
