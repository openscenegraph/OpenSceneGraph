#include "osg/NodeVisitor"
#include <stdlib.h>

using namespace osg;

NodeVisitor::NodeVisitor(TraversalMode tm)
{
    _traverseVisitor = NULL;
    _traverseMode = tm;
}

NodeVisitor::~NodeVisitor()
{
    // if (_traverseVisitor) detach from _traverseVisitor;
}

void NodeVisitor::setTraverseMode(TraversalMode mode)
{
    if (_traverseMode==mode) return;
    if (mode==TRAVERSE_VISITOR)
    {
        if (_traverseVisitor==NULL) _traverseMode = TRAVERSE_NONE;
        else _traverseMode = TRAVERSE_VISITOR;
    }
    else
    {
        if (_traverseVisitor) _traverseVisitor=NULL;
        _traverseMode = mode;
    }
}

void NodeVisitor::setTraverseVisitor(NodeVisitor* nv)
{
    if (_traverseVisitor==nv) return;
    // if (_traverseVisitor) detach from _traverseVisitor;
    _traverseVisitor = nv;
    if (_traverseVisitor) _traverseMode = TRAVERSE_VISITOR;
    else _traverseMode = TRAVERSE_NONE;
    // attach to _traverseVisitor;
}
