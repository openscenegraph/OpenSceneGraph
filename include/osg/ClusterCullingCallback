/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#ifndef OSG_CLUSTERCULLINGCALLBACK
#define OSG_CLUSTERCULLINGCALLBACK 1

#include <osg/Drawable>
#include <osg/Callback>

namespace osg {

/** @class ClusterCullingCallback 
    @brief Implements cluster culling to cull back facing subgraphs and drawables. Derived from Drawable::CullCallback and osg::NodeCallback.
    
    This culling callback is intended to be attached to a node using the setCullCallback method. If the
    node is a drawable cull(osg::NodeVisitor*, osg::Drawable*, osg::State*) otherwise
    operator()(Node*, NodeVisitor*) will be called during the cull traversal.
    
    To decide whether the node (in case of a drawable) or its children (in case of any other node type) are
    to be culled depends on four parameters:
     - a control point,
     - a normal specified at the control point,
     - a deviation value representing the cosinus of an enclosed angle and
     - a radius describing a sphere around the control point.
    
    The node is culled if the following two conditions are fulfilled:
     - the distance between the current eye/view point to the control point is larger or equal to radius,
     - the cosinus of the enclosed angle between the normal and the vector from the control point to the eye/view point
       is smaller(!) than the specified deviation value (normally this value is negative meaning that the enclosed angle
       between the control point and the eye/view point is larger than the angle indirectly specified by the
       deviation value).
    
    @remark As the deviation is representing the cosine of an enclosed angle its value should be within the
            the interval [-1; 1]. A value of one will cull all nodes while a value of -1 will never cull a node.
            The deviation will normally have negative values because then the enclosed angle between the normal and the
            eye/view point is larger than 90 degrees (and therefore the eye sees the "back" from the control point).
*/
class OSG_EXPORT ClusterCullingCallback : public DrawableCullCallback, public NodeCallback
{
    public:

        ClusterCullingCallback();
        ClusterCullingCallback(const ClusterCullingCallback& ccc,const CopyOp& copyop);
        ClusterCullingCallback(const osg::Vec3& controlPoint, const osg::Vec3& normal, float deviation, float radius=-1.0f);
        ClusterCullingCallback(const osg::Drawable* drawable);

        META_Object(osg,ClusterCullingCallback);

        virtual NodeCallback* asNodeCallback() { return osg::NodeCallback::asNodeCallback(); }
        virtual const NodeCallback* asNodeCallback() const { return osg::NodeCallback::asNodeCallback(); }

        virtual DrawableCullCallback* asDrawableCullCallback() { return osg::DrawableCullCallback::asDrawableCullCallback(); }
        virtual const DrawableCullCallback* asDrawableCullCallback() const { return osg::DrawableCullCallback::asDrawableCullCallback(); }

        // use the NodeCallbacks implementation of run.
        virtual bool run(osg::Object* object, osg::Object* data) { return NodeCallback::run(object, data); }

        /** Computes the control point, normal, and deviation from the
          * given drawable contents. */
        void computeFrom(const osg::Drawable* drawable);

        /** Transform the ClusterCullingCallback's positional members to a new coordinate frame.*/
        void transform(const osg::Matrixd& matrix);

        void set(const osg::Vec3& controlPoint, const osg::Vec3& normal, float deviation, float radius);

        void setControlPoint(const osg::Vec3& controlPoint) { _controlPoint = controlPoint; }
        const osg::Vec3& getControlPoint() const { return _controlPoint; }

        void setNormal(const osg::Vec3& normal) { _normal = normal; }
        const osg::Vec3& getNormal() const { return _normal; }

        void setRadius(float radius) { _radius = radius; }
        float getRadius() const { return _radius; }

        void setDeviation(float deviation) { _deviation = deviation; }
        float getDeviation() const { return _deviation; }

        virtual bool cull(osg::NodeVisitor*, osg::Drawable*, osg::State*) const;

        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void operator()(Node* node, NodeVisitor* nv);

    protected:

        virtual ~ClusterCullingCallback() {}

        osg::Vec3    _controlPoint;
        osg::Vec3    _normal;
        float        _radius;
        float        _deviation;
};


}

#endif
