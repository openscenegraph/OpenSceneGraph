#include <osg/NodeVisitor>
#include <osg/Transform>
#include <stdlib.h>

using namespace osg;

NodeVisitor::NodeVisitor(TraversalMode tm)
{
    _traversalNumber = -1;

    _traversalVisitor = NULL;
    _traversalMode = tm;
    _traversalMask = 0xffffffff;
    _nodeMaskOverride = 0x0;
}


NodeVisitor::~NodeVisitor()
{
    // if (_traversalVisitor) detach from _traversalVisitor;
}


void NodeVisitor::setTraversalMode(const TraversalMode mode)
{
    if (_traversalMode==mode) return;
    if (mode==TRAVERSE_VISITOR)
    {
        if (_traversalVisitor==NULL) _traversalMode = TRAVERSE_NONE;
        else _traversalMode = TRAVERSE_VISITOR;
    }
    else
    {
        if (_traversalVisitor.valid()) _traversalVisitor=NULL;
        _traversalMode = mode;
    }
}


void NodeVisitor::setTraversalVisitor(NodeVisitor* nv)
{
    if (_traversalVisitor==nv) return;
    _traversalVisitor = nv;
    if (_traversalVisitor.valid()) _traversalMode = TRAVERSE_VISITOR;
    else _traversalMode = TRAVERSE_NONE;
}


class TransformVisitor : public NodeVisitor
{
    public:
    
        enum CoordMode
        {
            WORLD_TO_LOCAL,
            LOCAL_TO_WORLD
        };
        

        MatrixMode      _matrixMode;
        CoordMode       _coordMode;
        Matrix&         _matrix;
        NodeVisitor*    _nodeVisitor;

        TransformVisitor(Matrix& matrix,MatrixMode matrixMode,CoordMode coordMode,NodeVisitor* nv):
            NodeVisitor(),
            _matrixMode(matrixMode),
            _coordMode(coordMode),
            _matrix(matrix),
            _nodeVisitor(nv)
            {}

        virtual void apply(Transform& transform)
        {
            bool applyTransform = 
                (_matrixMode==transform.getMatrixMode()) ||
                (_matrixMode==MODELVIEW && (transform.getMatrixMode()==MODEL || transform.getMatrixMode()==VIEW));
            
            if (applyTransform)
            {
                if (_coordMode==LOCAL_TO_WORLD)
                {
                    osg::Matrix localToWorldMat;
                    transform.getLocalToWorldMatrix(localToWorldMat,_nodeVisitor);
                    _matrix.preMult(localToWorldMat);
                }
                else // worldToLocal
                {
                    osg::Matrix worldToLocalMat;
                    transform.getWorldToLocalMatrix(worldToLocalMat,_nodeVisitor);
                    _matrix.postMult(worldToLocalMat);
                }
            }
        }
    
};


const bool NodeVisitor::getLocalToWorldMatrix(Matrix& matrix, MatrixMode mode, Node* node)
{
    matrix.makeIdentity();
    TransformVisitor tv(matrix,mode,TransformVisitor::LOCAL_TO_WORLD,this);
    for(NodePath::iterator itr=_nodePath.begin();
        itr!=_nodePath.end();
        ++itr)
    {
        if (*itr==node) return true; // don't account for matrix attached to specofied node
        (*itr)->accept(tv);
    }
    return true;
}

const bool NodeVisitor::getWorldToLocalMatrix(Matrix& matrix, MatrixMode mode, Node* node)
{
    matrix.makeIdentity();
    TransformVisitor tv(matrix,mode,TransformVisitor::WORLD_TO_LOCAL,this);
    for(NodePath::iterator itr=_nodePath.begin();
        itr!=_nodePath.end();
        ++itr)
    {
        if (*itr==node) return true; // don't account for matrix attached to specofied node
        (*itr)->accept(tv);
    }
    return true;
}
