#include <osg/Switch>

#include <algorithm>

using namespace osg;

/**
 * Switch constructor. The default setting of _value is
 * ALL_CHILDREN_OFF.
 */
Switch::Switch()
{
    _value = ALL_CHILDREN_OFF;
}


void Switch::traverse(NodeVisitor& nv)
{
    switch(nv.getTraversalMode())
    {
        case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
            break;
        case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
            switch(_value)
            {
                case(ALL_CHILDREN_ON):
                    std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
                    break;
                case(ALL_CHILDREN_OFF):
                    return;
                default:
                    if (_value>=0 && (unsigned int)_value<_children.size()) _children[_value]->accept(nv);
                    break;
            }
            break;
        default:
            break;
    }
}

