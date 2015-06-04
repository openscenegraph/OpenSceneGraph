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
#include <osg/Transform>
#include <osg/Camera>

#include <osg/Notify>

using namespace osg;

class TransformVisitor : public NodeVisitor
{
    public:

        enum CoordMode
        {
            WORLD_TO_LOCAL,
            LOCAL_TO_WORLD
        };


        CoordMode       _coordMode;
        Matrix&         _matrix;
        bool            _ignoreCameras;

        TransformVisitor(Matrix& matrix,CoordMode coordMode, bool ignoreCameras):
            NodeVisitor(),
            _coordMode(coordMode),
            _matrix(matrix),
            _ignoreCameras(ignoreCameras)
            {}

        virtual void apply(Transform& transform)
        {
            if (_coordMode==LOCAL_TO_WORLD)
            {
                transform.computeLocalToWorldMatrix(_matrix,this);
            }
            else // worldToLocal
            {
                transform.computeWorldToLocalMatrix(_matrix,this);
            }
        }

        void accumulate(const NodePath& nodePath)
        {
            if (nodePath.empty()) return;

            unsigned int i = 0;
            if (_ignoreCameras)
            {
                // we need to found out the last absolute Camera in NodePath and
                // set the i index to after it so the final accumulation set ignores it.
                i = nodePath.size();
                NodePath::const_reverse_iterator ritr;
                for(ritr = nodePath.rbegin();
                    ritr != nodePath.rend();
                    ++ritr, --i)
                {
                    const osg::Camera* camera = dynamic_cast<const osg::Camera*>(*ritr);
                    if (camera &&
                        (camera->getReferenceFrame()!=osg::Transform::RELATIVE_RF || camera->getParents().empty()))
                    {
                        break;
                    }
                }
            }

            // do the accumulation of the active part of nodepath.
            for(;
                i<nodePath.size();
                ++i)
            {
                const_cast<Node*>(nodePath[i])->accept(*this);
            }
        }

    protected:

        TransformVisitor& operator = (const TransformVisitor&) { return *this; }

};

Matrix osg::computeLocalToWorld(const NodePath& nodePath, bool ignoreCameras)
{
    Matrix matrix;
    TransformVisitor tv(matrix,TransformVisitor::LOCAL_TO_WORLD,ignoreCameras);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeWorldToLocal(const NodePath& nodePath, bool ignoreCameras)
{
    osg::Matrix matrix;
    TransformVisitor tv(matrix,TransformVisitor::WORLD_TO_LOCAL,ignoreCameras);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeLocalToEye(const Matrix& modelview,const NodePath& nodePath, bool ignoreCameras)
{
    Matrix matrix(modelview);
    TransformVisitor tv(matrix,TransformVisitor::LOCAL_TO_WORLD,ignoreCameras);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeEyeToLocal(const Matrix& modelview,const NodePath& nodePath, bool ignoreCameras)
{
    Matrix matrix;
    matrix.invert(modelview);
    TransformVisitor tv(matrix,TransformVisitor::WORLD_TO_LOCAL,ignoreCameras);
    tv.accumulate(nodePath);
    return matrix;
}





Transform::Transform()
{
    _referenceFrame = RELATIVE_RF;
}

Transform::Transform(const Transform& transform,const CopyOp& copyop):
    Group(transform,copyop),
    _referenceFrame(transform._referenceFrame)
{
}

Transform::~Transform()
{
}

void Transform::setReferenceFrame(ReferenceFrame rf)
{
    if (_referenceFrame == rf) return;

    _referenceFrame = rf;

    // switch off culling if transform is absolute.
    setCullingActive(_referenceFrame==RELATIVE_RF);
}

BoundingSphere Transform::computeBound() const
{
    BoundingSphere bsphere = Group::computeBound();
    if (!bsphere.valid()) return bsphere;

    // note, NULL pointer for NodeVisitor, so compute's need
    // to handle this case gracefully, normally this should not be a problem.
    Matrix l2w;

    computeLocalToWorldMatrix(l2w,NULL);

    osg::BoundingSphere::vec_type xdash = bsphere._center;
    xdash.x() += bsphere._radius;
    xdash = xdash*l2w;

    osg::BoundingSphere::vec_type ydash = bsphere._center;
    ydash.y() += bsphere._radius;
    ydash = ydash*l2w;

    osg::BoundingSphere::vec_type zdash = bsphere._center;
    zdash.z() += bsphere._radius;
    zdash = zdash*l2w;

    bsphere._center = bsphere._center*l2w;

    xdash -= bsphere._center;
    osg::BoundingSphere::value_type sqrlen_xdash = xdash.length2();

    ydash -= bsphere._center;
    osg::BoundingSphere::value_type sqrlen_ydash = ydash.length2();

    zdash -= bsphere._center;
    osg::BoundingSphere::value_type sqrlen_zdash = zdash.length2();

    bsphere._radius = sqrlen_xdash;
    if (bsphere._radius<sqrlen_ydash) bsphere._radius = sqrlen_ydash;
    if (bsphere._radius<sqrlen_zdash) bsphere._radius = sqrlen_zdash;
    bsphere._radius = (osg::BoundingSphere::value_type)sqrt(bsphere._radius);

    return bsphere;

}
