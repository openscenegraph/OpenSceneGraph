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
#include <osgSim/MultiSwitch>

#include <algorithm>

using namespace osgSim;
using namespace osg;

MultiSwitch::MultiSwitch():
    _newChildDefaultValue(true),
    _activeSwitchSet(0)
{
}

MultiSwitch::MultiSwitch(const MultiSwitch& sw,const osg::CopyOp& copyop):
    osg::Group(sw,copyop),
    _newChildDefaultValue(sw._newChildDefaultValue),
    _activeSwitchSet(sw._activeSwitchSet),
    _values(sw._values)
{
}

void MultiSwitch::traverse(osg::NodeVisitor& nv)
{
    if (nv.getTraversalMode()==osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
    {

        if (_activeSwitchSet<_values.size())
        {
            for(unsigned int pos=0;pos<_children.size();++pos)
            {
                if (_values[_activeSwitchSet][pos]) _children[pos]->accept(nv);
            }
        }
    }
    else
    {
        Group::traverse(nv);
    }
}

bool MultiSwitch::addChild( osg::Node *child)
{
    unsigned int childPosition = _children.size();
    if (Group::addChild(child))
    {
        for(SwitchSetList::iterator itr=_values.begin();
            itr!=_values.end();
            ++itr)
        {
            ValueList& values = *itr;
            if (_children.size()>values.size())
            {
                values.resize(_children.size(),_newChildDefaultValue);
                values[childPosition]=_newChildDefaultValue;
            }
        }
        return true;
    }
    return false;
}

bool MultiSwitch::insertChild( unsigned int index, osg::Node *child)
{
    if (Group::insertChild(index,child))
    {
        for(SwitchSetList::iterator itr=_values.begin();
            itr!=_values.end();
            ++itr)
        {
            ValueList& values = *itr;
            if (index>=values.size())
            {
                values.push_back(_newChildDefaultValue);
            }
            else
            {
                values.insert(values.begin()+index, _newChildDefaultValue);
            }
        }


        return true;
    }
    return false;
}

bool MultiSwitch::removeChild( osg::Node *child )
{
    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return false;

    for(SwitchSetList::iterator itr=_values.begin();
        itr!=_values.end();
        ++itr)
    {
        ValueList& values = *itr;
        values.erase(values.begin()+pos);
    }

    return Group::removeChild(child);
}

void MultiSwitch::setValue(unsigned int switchSet, unsigned int pos,bool value)
{
    expandToEncompassSwitchSet(switchSet);

    ValueList& values = _values[switchSet];
    if (pos>=values.size()) values.resize(pos+1,_newChildDefaultValue);
    values[pos]=value;
}

void MultiSwitch::setChildValue(const osg::Node* child,unsigned int switchSet, bool value)
{
    expandToEncompassSwitchSet(switchSet);

    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return;

    ValueList& values = _values[switchSet];
    values[pos]=value;
}

bool MultiSwitch::getValue(unsigned int switchSet, unsigned int pos) const
{
    if (switchSet>=_values.size()) return false;

    const ValueList& values = _values[switchSet];
    if (pos>=values.size()) return false;

    return values[pos];
}

bool MultiSwitch::getChildValue(const osg::Node* child, unsigned int switchSet) const
{
    if (switchSet>=_values.size()) return false;

    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return false;

    const ValueList& values = _values[switchSet];
    return values[pos];
}

void MultiSwitch::expandToEncompassSwitchSet(unsigned int switchSet)
{
    if (switchSet>=_values.size())
    {
        // need to expand arrays.
        unsigned int originalSize = _values.size();
        _values.resize(switchSet+1);
        _valueNames.resize(switchSet+1);
        for(unsigned int i=originalSize;i<=switchSet;++i)
        {
            ValueList& values = _values[i];
            values.resize(_children.size(),_newChildDefaultValue);
        }
    }
}

bool MultiSwitch::setAllChildrenOff(unsigned int switchSet)
{
    _newChildDefaultValue = false;

    expandToEncompassSwitchSet(switchSet);

    ValueList& values = _values[switchSet];
    for(ValueList::iterator itr=values.begin();
        itr!=values.end();
        ++itr)
    {
        *itr = false;
    }
    return true;
}

bool MultiSwitch::setAllChildrenOn(unsigned int switchSet)
{
    _newChildDefaultValue = true;

    expandToEncompassSwitchSet(switchSet);

    ValueList& values = _values[switchSet];
    for(ValueList::iterator itr=values.begin();
        itr!=values.end();
        ++itr)
    {
        *itr = true;
    }
    return true;
}

bool MultiSwitch::setSingleChildOn(unsigned int switchSet, unsigned int pos)
{
    expandToEncompassSwitchSet(switchSet);

    ValueList& values = _values[switchSet];
    for(ValueList::iterator itr=values.begin();
        itr!=values.end();
        ++itr)
    {
        *itr = false;
    }
    setValue(switchSet, pos,true);
    return true;
}


void MultiSwitch::setSwitchSetList(const SwitchSetList& switchSetList)
{
    expandToEncompassSwitchSet(switchSetList.size());

    _values = switchSetList;
}

void MultiSwitch::setValueList(unsigned int switchSet, const ValueList& values)
{
    expandToEncompassSwitchSet(switchSet);

    _values[switchSet] = values;
}

void MultiSwitch::setValueName(unsigned int switchSet, const std::string& name)
{
    expandToEncompassSwitchSet(switchSet);

    _valueNames[switchSet] = name;
}
