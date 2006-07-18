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
#include <osgUtil/GLObjectsVisitor>
#include <osg/Drawable>
#include <osg/Notify>

using namespace osg;
using namespace osgUtil;

GLObjectsVisitor::GLObjectsVisitor(Mode mode)
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    _mode = mode;

    _state = NULL;
}


void GLObjectsVisitor::apply(osg::Node& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    traverse(node);
}

void GLObjectsVisitor::apply(osg::Geode& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        Drawable* drawable = node.getDrawable(i);
        if (drawable)
        {
            apply(*drawable);
            if (drawable->getStateSet())
            {
                apply(*(drawable->getStateSet()));
            }
        }
    }
}

void GLObjectsVisitor::apply(osg::Drawable& drawable)
{
    if (_drawablesAppliedSet.count(&drawable)!=0) return;
    
    _drawablesAppliedSet.insert(&drawable);

    if (_mode&SWITCH_OFF_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(false);
    }

    if (_mode&SWITCH_ON_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(true);
    }

    if (_mode&COMPILE_DISPLAY_LISTS && _state.valid())
    {
        drawable.compileGLObjects(*_state);
    }

    if (_mode&RELEASE_DISPLAY_LISTS)
    {
        drawable.releaseGLObjects(_state.get());
    }

    if (_mode&SWITCH_ON_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(true);
    }

    if (_mode&SWITCH_OFF_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(false);
    }
}

void GLObjectsVisitor::apply(osg::StateSet& stateset)
{
    if (_stateSetAppliedSet.count(&stateset)!=0) return;
    
    _stateSetAppliedSet.insert(&stateset);

    if (_mode & COMPILE_STATE_ATTRIBUTES && _state.valid())
    {
        stateset.compileGLObjects(*_state);
    }

    if (_mode & RELEASE_STATE_ATTRIBUTES)
    {
        stateset.releaseGLObjects(_state.get());
    }
    
    if (_mode & CHECK_BLACK_LISTED_MODES)
    {
        stateset.checkValidityOfAssociatedModes(*_state.get());
    }
}
