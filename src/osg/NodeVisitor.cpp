#include <osg/NodeVisitor>
#include <osg/Transform>
#include <stdlib.h>

using namespace osg;

NodeVisitor::NodeVisitor(TraversalMode tm)
{
    _visitorType = NODE_VISITOR;
    _traversalNumber = -1;

    _traversalMode = tm;
    _traversalMask = 0xffffffff;
    _nodeMaskOverride = 0x0;
}

NodeVisitor::NodeVisitor(VisitorType type,TraversalMode tm)
{
    _visitorType = type;
    _traversalNumber = -1;

    _traversalMode = tm;
    _traversalMask = 0xffffffff;
    _nodeMaskOverride = 0x0;
}


NodeVisitor::~NodeVisitor()
{
    // if (_traversalVisitor) detach from _traversalVisitor;
}

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
        NodeVisitor*    _nodeVisitor;

        TransformVisitor(Matrix& matrix,CoordMode coordMode,NodeVisitor* nv):
            NodeVisitor(),
            _coordMode(coordMode),
            _matrix(matrix),
            _nodeVisitor(nv)
            {}

        virtual void apply(Transform& transform)
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
    
};


bool NodeVisitor::getLocalToWorldMatrix(Matrix& matrix, Node* node)
{
    TransformVisitor tv(matrix,TransformVisitor::LOCAL_TO_WORLD,this);
    for(NodePath::iterator itr=_nodePath.begin();
        itr!=_nodePath.end();
        ++itr)
    {
        if (*itr==node) return true; // don't account for matrix attached to specified node
        (*itr)->accept(tv);
    }
    return true;
}

bool NodeVisitor::getWorldToLocalMatrix(Matrix& matrix, Node* node)
{
    TransformVisitor tv(matrix,TransformVisitor::WORLD_TO_LOCAL,this);
    for(NodePath::iterator itr=_nodePath.begin();
        itr!=_nodePath.end();
        ++itr)
    {
        if (*itr==node) return true; // don't account for matrix attached to specified node
        (*itr)->accept(tv);
    }
    return true;
}
