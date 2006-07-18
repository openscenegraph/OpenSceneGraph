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
#include <osg/CameraNode>

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
        bool            _ignoreCameraNodes;

        TransformVisitor(Matrix& matrix,CoordMode coordMode, bool ignoreCameraNodes):
            NodeVisitor(),
            _coordMode(coordMode),
            _matrix(matrix),
            _ignoreCameraNodes(ignoreCameraNodes)
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
            if (_ignoreCameraNodes)
            {
                // we need to found out the last absolute CameraNode in NodePath and
                // set the i index to after it so the final accumulation set ignores it.
                i = nodePath.size();
                NodePath::const_reverse_iterator ritr;
                for(ritr = nodePath.rbegin();
                    ritr != nodePath.rend();
                    ++ritr, --i)
                {
                    const osg::CameraNode* camera = dynamic_cast<const osg::CameraNode*>(*ritr);
                    if (camera && 
                        (camera->getReferenceFrame()==osg::Transform::ABSOLUTE_RF || camera->getParents().empty()))
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
    
};

Matrix osg::computeLocalToWorld(const NodePath& nodePath, bool ignoreCameraNodes)
{
    Matrix matrix;
    TransformVisitor tv(matrix,TransformVisitor::LOCAL_TO_WORLD,ignoreCameraNodes);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeWorldToLocal(const NodePath& nodePath, bool ignoreCameraNodes)
{
    osg::Matrix matrix;
    TransformVisitor tv(matrix,TransformVisitor::WORLD_TO_LOCAL,ignoreCameraNodes);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeLocalToEye(const Matrix& modelview,const NodePath& nodePath, bool ignoreCameraNodes)
{
    Matrix matrix(modelview);
    TransformVisitor tv(matrix,TransformVisitor::LOCAL_TO_WORLD,ignoreCameraNodes);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeEyeToLocal(const Matrix& modelview,const NodePath& nodePath, bool ignoreCameraNodes)
{
    Matrix matrix;
    matrix.invert(modelview);
    TransformVisitor tv(matrix,TransformVisitor::WORLD_TO_LOCAL,ignoreCameraNodes);
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

    Vec3 xdash = bsphere._center;
    xdash.x() += bsphere._radius;
    xdash = xdash*l2w;

    Vec3 ydash = bsphere._center;
    ydash.y() += bsphere._radius;
    ydash = ydash*l2w;

    Vec3 zdash = bsphere._center;
    zdash.z() += bsphere._radius;
    zdash = zdash*l2w;


    bsphere._center = bsphere._center*l2w;

    xdash -= bsphere._center;
    float len_xdash = xdash.length();

    ydash -= bsphere._center;
    float len_ydash = ydash.length();

    zdash -= bsphere._center;
    float len_zdash = zdash.length();

    bsphere._radius = len_xdash;
    if (bsphere._radius<len_ydash) bsphere._radius = len_ydash;
    if (bsphere._radius<len_zdash) bsphere._radius = len_zdash;

    return bsphere;

}
