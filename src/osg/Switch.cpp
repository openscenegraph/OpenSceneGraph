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
#include <osg/Switch>
#include <osg/BoundingBox>
#include <osg/Transform>

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
    if (nv.getTraversalMode()==NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
    {
        for(unsigned int pos=0;pos<_children.size();++pos)
        {
            if (_values[pos]) _children[pos]->accept(nv);
        }
    }
    else
    {
        Group::traverse(nv);
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
	}
	_values[childPosition]=value;
	return true;
    }
    return false;
}

bool Switch::insertChild( unsigned int index, Node *child )
{
    return insertChild(index,child,_newChildDefaultValue);
}

bool Switch::insertChild( unsigned int index, Node *child, bool value )
{
    if (Group::insertChild(index,child))
    {
        if (index>=_values.size())
	{
	    _values.push_back(value);
	}
        else
        {
	    _values.insert(_values.begin()+index, value);
        }
        
	return true;
    }
    return false;
}

bool Switch::removeChild( Node *child )
{
    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return false;
    
    _values.erase(_values.begin()+pos);
    
    return Group::removeChild(child);    
}

void Switch::setValue(unsigned int pos,bool value)
{
    if (pos>=_values.size()) _values.resize(pos+1,_newChildDefaultValue);
    _values[pos]=value;
}

void Switch::setChildValue(const Node* child,bool value)
{
    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return;
    
    _values[pos]=value;
}

bool Switch::getValue(unsigned int pos) const
{
    if (pos>=_values.size()) return false;
    return _values[pos];
}

bool Switch::getChildValue(const Node* child) const
{
    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return false;
    
    return _values[pos];
}

bool Switch::setAllChildrenOff()
{
    _newChildDefaultValue = false;
    for(ValueList::iterator itr=_values.begin();
        itr!=_values.end();
        ++itr)
    {
        *itr = false;
    }
    return true;
}

bool Switch::setAllChildrenOn()
{
    _newChildDefaultValue = true;
    for(ValueList::iterator itr=_values.begin();
        itr!=_values.end();
        ++itr)
    {
        *itr = true;
    }
    return true;
}

bool Switch::setSingleChildOn(unsigned int pos)
{
    for(ValueList::iterator itr=_values.begin();
        itr!=_values.end();
        ++itr)
    {
        *itr = false;
    }
    setValue(pos,true);
    return true;
}

bool Switch::computeBound() const
{
    _bsphere.init();
    if (_children.empty()) 
    {
        _bsphere_computed = true;
        return false;
    }

    // note, special handling of the case when a child is an Transform,
    // such that only Transforms which are relative to their parents coordinates frame (i.e this group)
    // are handled, Transform relative to and absolute reference frame are ignored.

    BoundingBox bb;
    bb.init();
    NodeList::const_iterator itr;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = (*itr)->asTransform();
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_RF)
        {
            if( getChildValue((*itr).get()) == true )
                bb.expandBy((*itr)->getBound());
        }
    }

    if (!bb.valid()) 
    {
        _bsphere_computed = true;
        return false;
    }

    _bsphere._center = bb.center();
    _bsphere._radius = 0.0f;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = (*itr)->asTransform();
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_RF)
        {
            if( getChildValue((*itr).get()) == true )
                _bsphere.expandRadiusBy((*itr)->getBound());
        }
    }

    _bsphere_computed = true;
    return true;
}

#ifdef USE_DEPRECATED_API
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

#endif
