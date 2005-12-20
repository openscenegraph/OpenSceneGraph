/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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
#include <osg/Notify>

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
    if (Group::addChild(child))
    {
        if (_children.size()>_values.size())
        {
            _values.resize(_children.size(),_newChildDefaultValue);
        }
        // note, we don't override any pre-existing _values[childPosition] setting
        // like in addChild(child,value) below.
        return true;
    }
    return false;
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
    return removeChild( getChildIndex(child) );
}

bool Switch::removeChild(unsigned int pos,unsigned int numChildrenToRemove)
{
    if (pos>=_values.size() || numChildrenToRemove==0) return false;

    unsigned int endOfRemoveRange = pos+numChildrenToRemove;
    if (endOfRemoveRange>_values.size())
    {
        notify(DEBUG_INFO)<<"Warning: Switch::removeChild(i,numChildrenToRemove) has been passed an excessive number"<<std::endl;
        notify(DEBUG_INFO)<<"         of chilren to remove, trimming just to end of value list."<<std::endl;
        endOfRemoveRange=_values.size();
    }
    _values.erase(_values.begin()+pos,_values.begin()+endOfRemoveRange);

    return Group::removeChild(pos, numChildrenToRemove);
}

void Switch::setValue(unsigned int pos,bool value)
{
    if (pos>=_values.size()) _values.resize(pos+1,_newChildDefaultValue);
    _values[pos]=value;
    dirtyBound();
}

void Switch::setChildValue(const Node* child,bool value)
{
    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return;
    
    _values[pos]=value;
    dirtyBound();
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
    dirtyBound();
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
    dirtyBound();
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

BoundingSphere Switch::computeBound() const
{
    BoundingSphere bsphere;
    if (_children.empty()) 
    {
        return bsphere;
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
        return bsphere;
    }

    bsphere._center = bb.center();
    bsphere._radius = 0.0f;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        const osg::Transform* transform = (*itr)->asTransform();
        if (!transform || transform->getReferenceFrame()==osg::Transform::RELATIVE_RF)
        {
            if( getChildValue((*itr).get()) == true )
                bsphere.expandRadiusBy((*itr)->getBound());
        }
    }
    return bsphere;
}

