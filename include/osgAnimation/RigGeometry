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

#ifndef OSGANIMATION_RIGGEOMETRY_H
#define OSGANIMATION_RIGGEOMETRY_H

#include <osgAnimation/Export>
#include <osgAnimation/Skeleton>
#include <osgAnimation/RigTransform>
#include <osgAnimation/VertexInfluence>
#include <osg/Geometry>

namespace osgAnimation
{
    // The idea is to compute a bounding box with a factor x of the first step we compute the bounding box
    class OSGANIMATION_EXPORT RigComputeBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
    {
    public:
        RigComputeBoundingBoxCallback(double factor = 2.0): _computed(false), _factor(factor) {}

        RigComputeBoundingBoxCallback(const RigComputeBoundingBoxCallback& rhs, const osg::CopyOp& copyop) :
            osg::Drawable::ComputeBoundingBoxCallback(rhs, copyop),
            _computed(false),
            _factor(rhs._factor) {}

        META_Object(osgAnimation, RigComputeBoundingBoxCallback);

        void reset() { _computed = false; }

        virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const;
    protected:
        mutable bool _computed;
        double _factor;
        mutable osg::BoundingBox _boundingBox;
    };


    class OSGANIMATION_EXPORT RigGeometry : public osg::Geometry
    {
    public:

        RigGeometry();

        RigGeometry(const RigGeometry& b, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

        META_Object(osgAnimation, RigGeometry);

        inline void setInfluenceMap(VertexInfluenceMap* vertexInfluenceMap) { _vertexInfluenceMap = vertexInfluenceMap; }
        inline const VertexInfluenceMap* getInfluenceMap() const { return _vertexInfluenceMap.get(); }
        inline VertexInfluenceMap* getInfluenceMap() { return _vertexInfluenceMap.get(); }

        inline const Skeleton* getSkeleton() const { return _root.get(); }
        inline Skeleton* getSkeleton() { return _root.get(); }
        // will be used by the update callback to init correctly the rig mesh
        inline void setSkeleton(Skeleton* root) { _root = root; }

        void setNeedToComputeMatrix(bool state) { _needToComputeMatrix = state; }
        bool getNeedToComputeMatrix() const { return _needToComputeMatrix; }

        void computeMatrixFromRootSkeleton();

        // set implementation of rig method
        inline RigTransform* getRigTransformImplementation() { return _rigTransformImplementation.get(); }
        inline void setRigTransformImplementation(RigTransform* rig) { _rigTransformImplementation = rig; }
        inline const RigTransform* getRigTransformImplementation() const { return _rigTransformImplementation.get(); }

        void update();

        void buildVertexInfluenceSet() { _rigTransformImplementation->prepareData(*this); }

        const osg::Matrix& getMatrixFromSkeletonToGeometry() const;

        const osg::Matrix& getInvMatrixFromSkeletonToGeometry() const;

        inline osg::Geometry* getSourceGeometry() { return _geometry.get(); }
        inline const osg::Geometry* getSourceGeometry() const { return _geometry.get(); }
        inline void setSourceGeometry(osg::Geometry* geometry) { _geometry = geometry; }

        void copyFrom(osg::Geometry& from);

        struct FindNearestParentSkeleton : public osg::NodeVisitor
        {
            osg::ref_ptr<Skeleton> _root;
            FindNearestParentSkeleton() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS) {}
            void apply(osg::Transform& node)
            {
                if (_root.valid())
                    return;
                _root = dynamic_cast<osgAnimation::Skeleton*>(&node);
                traverse(node);
            }
        };

    protected:

        osg::ref_ptr<osg::Geometry> _geometry;
        osg::ref_ptr<RigTransform> _rigTransformImplementation;
        osg::ref_ptr<VertexInfluenceMap> _vertexInfluenceMap;

        osg::Matrix _matrixFromSkeletonToGeometry;
        osg::Matrix _invMatrixFromSkeletonToGeometry;
        osg::observer_ptr<Skeleton> _root;
        bool _needToComputeMatrix;

    };


    struct UpdateRigGeometry : public osg::Drawable::UpdateCallback
    {
        UpdateRigGeometry() {}

        UpdateRigGeometry(const UpdateRigGeometry& org, const osg::CopyOp& copyop):
            osg::Object(org, copyop),
            osg::Callback(org, copyop),
            osg::DrawableUpdateCallback(org, copyop) {}

        META_Object(osgAnimation, UpdateRigGeometry);

        virtual void update(osg::NodeVisitor* nv, osg::Drawable* drw)
        {
            RigGeometry* geom = dynamic_cast<RigGeometry*>(drw);
            if(!geom)
                return;
            if(!geom->getSkeleton() && !geom->getParents().empty())
            {
                RigGeometry::FindNearestParentSkeleton finder;
                if(geom->getParents().size() > 1)
                    osg::notify(osg::WARN) << "A RigGeometry should not have multi parent ( " << geom->getName() << " )" << std::endl;
                geom->getParents()[0]->accept(finder);

                if(!finder._root.valid())
                {
                    osg::notify(osg::WARN) << "A RigGeometry did not find a parent skeleton for RigGeometry ( " << geom->getName() << " )" << std::endl;
                    return;
                }
                geom->getRigTransformImplementation()->prepareData(*geom);
                geom->setSkeleton(finder._root.get());
            }

            if(!geom->getSkeleton())
                return;

            if(geom->getNeedToComputeMatrix())
                geom->computeMatrixFromRootSkeleton();

            if(geom->getSourceGeometry())
            {
                osg::Drawable::UpdateCallback * up = dynamic_cast<osg::Drawable::UpdateCallback*>(geom->getSourceGeometry()->getUpdateCallback());
                if(up)
                    up->update(nv, geom->getSourceGeometry());
            }

            geom->update();
        }
    };
}

#endif
