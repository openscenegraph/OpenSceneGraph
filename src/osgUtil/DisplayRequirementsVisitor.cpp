#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <list>
#include <set>

#include <osg/GeoSet>

#include <osgUtil/DisplayRequirementsVisitor>

using namespace osg;
using namespace osgUtil;

DisplayRequirementsVisitor::DisplayRequirementsVisitor()
{
    setTraversalMode(NodeVisitor::TRAVERSE_ALL_CHILDREN);
}

void DisplayRequirementsVisitor::applyStateSet(StateSet& stateset)
{
    if (!_ds) _ds = osgNew osg::DisplaySettings;

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

    traverse(node);
}

void DisplayRequirementsVisitor::apply(Geode& geode)
{
    osg::StateSet* geode_stateset = geode.getStateSet();
    if (geode_stateset) applyStateSet(*geode_stateset);
    
    for(int i = 0; i < geode.getNumDrawables(); i++ )
    {
        osg::StateSet* stateset = geode.getDrawable(i)->getStateSet();
        if (stateset) applyStateSet(*stateset);
    }
}

void DisplayRequirementsVisitor::apply(Impostor& impostor)
{
    if (!_ds) _ds = osgNew osg::DisplaySettings;

    unsigned int min = 1; // number alpha bits we need at least.
    if (min>_ds->getMinimumNumAlphaBits())
    {
        // only update if new minimum exceeds previous minimum.
        _ds->setMinimumNumAlphaBits(min);
    }
    
    apply((Node&)impostor);
}
