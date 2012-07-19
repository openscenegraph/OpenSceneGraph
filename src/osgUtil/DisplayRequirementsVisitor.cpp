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
#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <string.h>
#include <list>
#include <set>

#include <osgUtil/DisplayRequirementsVisitor>

using namespace osg;
using namespace osgUtil;

DisplayRequirementsVisitor::DisplayRequirementsVisitor()
{
    setTraversalMode(NodeVisitor::TRAVERSE_ALL_CHILDREN);
}

void DisplayRequirementsVisitor::applyStateSet(StateSet& stateset)
{
    if (!_ds) _ds = new osg::DisplaySettings;

   unsigned int min = 0; // assume stencil not needed by this stateset.

   if (stateset.getMode(GL_STENCIL_TEST) & StateAttribute::ON)
   {
        min = 1; // number stencil bits we need at least.
   }

   if (stateset.getAttribute(StateAttribute::STENCIL))
   {
        min = 1; // number stencil bits we need at least.
   }

   if (min>_ds->getMinimumNumStencilBits())
   {
        // only update if new minimum exceeds previous minimum.
        _ds->setMinimumNumStencilBits(min);
   }
}

void DisplayRequirementsVisitor::apply(Node& node)
{
    osg::StateSet* stateset = node.getStateSet();
    if (stateset) applyStateSet(*stateset);

    if (strcmp(node.className(),"Impostor")==0)
    {
        if (!_ds) _ds = new osg::DisplaySettings;

        unsigned int min = 1; // number alpha bits we need at least.
        if (min>_ds->getMinimumNumAlphaBits())
        {
            // only update if new minimum exceeds previous minimum.
            _ds->setMinimumNumAlphaBits(min);
        }
    }

    traverse(node);
}

void DisplayRequirementsVisitor::apply(Geode& geode)
{
    osg::StateSet* geode_stateset = geode.getStateSet();
    if (geode_stateset) applyStateSet(*geode_stateset);

    for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
        osg::StateSet* stateset = geode.getDrawable(i)->getStateSet();
        if (stateset) applyStateSet(*stateset);
    }
}
