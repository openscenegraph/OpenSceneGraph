#include <osg/Switch>

#include <algorithm>

using namespace osg;

Switch::Switch():
    _newChildDefaultValue(true)
{
}

Switch::Switch(const Switch& sw,const CopyOp& copyop):
    Group(sw,copyop),
    _newChildDefaultValue(sw._newChildDefaultValue),
    _values(sw._values)
{
}

void Switch::traverse(NodeVisitor& nv)
{
    switch(nv.getTraversalMode())
    {
        case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
        {
            std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
            break;
        }
        case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            for(unsigned int pos=0;pos<_children.size();++pos)
            {
                if (_values[pos]) _children[pos]->accept(nv);
            }
            break;
        }
        default:
            break;
    }
}

bool Switch::addChild( Node *child )
{
    return addChild(child,_newChildDefaultValue);
}

bool Switch::addChild( Node *child, bool value )
{
    unsigned int childPosition = _children.size();
    if (Group::addChild(child))
    {
        if (_children.size()>_values.size())
	{
	    _values.resize(_children.size(),_newChildDefaultValue);
	    _values[childPosition]=value;
	}
	return true;
    }
    return false;
}

bool Switch::removeChild( Node *child )
{
    // find the child's position.
    unsigned int pos=findChildNum(child);
    if (pos==_children.size()) return false;
    
    _values.erase(_values.begin()+pos);
    
    return Group::removeChild(child);    
}

void Switch::setValue(unsigned int pos,bool value)
{
    if (pos>=_values.size()) _values.resize(pos+1,_newChildDefaultValue);
    _values[pos]=value;
}

void Switch::setValue(const Node* child,bool value)
{
    // find the child's position.
    unsigned int pos=findChildNum(child);
    if (pos==_children.size()) return;
    
    _values[pos]=value;
}

bool Switch::getValue(unsigned int pos) const
{
    if (pos>=_values.size()) return false;
    return _values[pos];
}

bool Switch::getValue(const Node* child) const
{
    // find the child's position.
    unsigned int pos=findChildNum(child);
    if (pos==_children.size()) return false;
    
    return _values[pos];
}

void Switch::setValue(int value)
{
    switch(value)
    {
        case(MULTIPLE_CHILDREN_ON):
            // do nothing...
            break;
        case(ALL_CHILDREN_OFF):
        {
            _newChildDefaultValue = false;
            for(ValueList::iterator itr=_values.begin();
                itr!=_values.end();
                ++itr)
            {
                *itr = false;
            }
            break;
        }
        case(ALL_CHILDREN_ON):
        {
            _newChildDefaultValue = true;
            for(ValueList::iterator itr=_values.begin();
                itr!=_values.end();
                ++itr)
            {
                *itr = true;
            }
            break;
        }
        default:
        {
            for(ValueList::iterator itr=_values.begin();
                itr!=_values.end();
                ++itr)
            {
                *itr = false;
            }
            setValue(value,true);
            break;
        }
    }
}

int Switch::getValue() const
{
    if (_values.empty()) return ALL_CHILDREN_OFF;
    
    unsigned int noChildrenSwitchedOn=0;
    int firstChildSelected=ALL_CHILDREN_OFF;
    for(unsigned int i=0; i<_values.size();++i)
    {
        if (_values[i])
        {
            ++noChildrenSwitchedOn;
            if (firstChildSelected==ALL_CHILDREN_OFF) firstChildSelected=i;
        }
    }
    
    if (noChildrenSwitchedOn>1)
    {
        if (noChildrenSwitchedOn==_values.size()) return ALL_CHILDREN_ON;
        else return MULTIPLE_CHILDREN_ON;
    }
    return firstChildSelected;

}
