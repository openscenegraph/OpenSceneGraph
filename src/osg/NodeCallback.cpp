#include <osg/Node>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>

using namespace osg;

void NodeCallback::traverse(Node* node,NodeVisitor* nv)
{
    if (_nestedCallback.valid()) (*_nestedCallback)(node,nv);
    else nv->traverse(*node);
}        
