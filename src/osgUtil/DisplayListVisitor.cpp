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
#include <osgUtil/DisplayListVisitor>
#include <osg/Drawable>

using namespace osg;
using namespace osgUtil;

DisplayListVisitor::DisplayListVisitor(Mode mode)
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    
    _mode = mode;
    
    _state = NULL;
}


void DisplayListVisitor::apply(osg::Node& node)
{
    if ((_mode&COMPILE_STATE_ATTRIBUTES) && node.getStateSet() && _state.valid())
    {
        node.getStateSet()->compile(*_state);
    }

    traverse(node);
}

void DisplayListVisitor::apply(osg::Geode& node)
{
    if (_mode&COMPILE_STATE_ATTRIBUTES && _state.valid())
    {
        if (node.getStateSet())
        {
            node.getStateSet()->compile(*_state);
        }

        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            Drawable* drawable = node.getDrawable(i);
            if (drawable->getStateSet())
            {
                drawable->getStateSet()->compile(*_state);
            }
        }
    }

    if (_mode&SWITCH_OFF_DISPLAY_LISTS)
    {
        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            node.getDrawable(i)->setUseDisplayList(false);
        }
    }
    if (_mode&SWITCH_ON_DISPLAY_LISTS)
    {
        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            node.getDrawable(i)->setUseDisplayList(true);
        }
    }

    if (_mode&COMPILE_DISPLAY_LISTS && _state.valid())
    {
        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            node.getDrawable(i)->compile(*_state);
        }
    }
}
