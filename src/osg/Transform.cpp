/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

        TransformVisitor(Matrix& matrix,CoordMode coordMode):
            NodeVisitor(),
            _coordMode(coordMode),
            _matrix(matrix)
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
            NodePath& non_const_nodePath = const_cast<NodePath&>(nodePath);
            for(NodePath::iterator itr=non_const_nodePath.begin();
                itr!=non_const_nodePath.end();
                ++itr)
            {
                (*itr)->accept(*this);
            }
        }
    
};

Matrix osg::computeLocalToWorld(const NodePath& nodePath)
{
    Matrix matrix;
    TransformVisitor tv(matrix,TransformVisitor::LOCAL_TO_WORLD);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeWorldToLocal(const NodePath& nodePath)
{
    osg::Matrix matrix;
    TransformVisitor tv(matrix,TransformVisitor::WORLD_TO_LOCAL);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeLocalToEye(const Matrix& modelview,const NodePath& nodePath)
{
    Matrix matrix(modelview);
    TransformVisitor tv(matrix,TransformVisitor::LOCAL_TO_WORLD);
    tv.accumulate(nodePath);
    return matrix;
}

Matrix osg::computeEyeToLocal(const Matrix& modelview,const NodePath& nodePath)
{
    Matrix matrix;
    matrix.invert(modelview);
    TransformVisitor tv(matrix,TransformVisitor::WORLD_TO_LOCAL);
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
    if (_referenceFrame==RELATIVE_RF) setCullingActive(false);
    else setCullingActive(true);
}

bool Transform::computeBound() const
{
    if (!Group::computeBound()) return false;
    
    // note, NULL pointer for NodeVisitor, so compute's need
    // to handle this case gracefully, normally this should not be a problem.
    Matrix l2w;

    computeLocalToWorldMatrix(l2w,NULL);

    Vec3 xdash = _bsphere._center;
    xdash.x() += _bsphere._radius;
    xdash = xdash*l2w;

    Vec3 ydash = _bsphere._center;
    ydash.y() += _bsphere._radius;
    ydash = ydash*l2w;

    Vec3 zdash = _bsphere._center;
    zdash.z() += _bsphere._radius;
    zdash = zdash*l2w;

    _bsphere._center = _bsphere._center*l2w;

    xdash -= _bsphere._center;
    float len_xdash = xdash.length();

    ydash -= _bsphere._center;
    float len_ydash = ydash.length();

    zdash -= _bsphere._center;
    float len_zdash = zdash.length();

    _bsphere._radius = len_xdash;
    if (_bsphere._radius<len_ydash) _bsphere._radius = len_ydash;
    if (_bsphere._radius<len_zdash) _bsphere._radius = len_zdash;

    return true;

}
