#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <list>
#include <set>

#include <osg/GeoSet>

#include <osgUtil/VisualsRequirementsVisitor>

using namespace osg;
using namespace osgUtil;

VisualsRequirementsVisitor::VisualsRequirementsVisitor()
{
    setTraversalMode(NodeVisitor::TRAVERSE_ALL_CHILDREN);
}

void VisualsRequirementsVisitor::applyStateSet(StateSet& stateset)
{
    if (!_vs) _vs = new osg::VisualsSettings;

   unsigned int min = 0; // assume stencil not needed by this stateset.
   
   if (stateset.getMode(GL_STENCIL_TEST) & StateAttribute::ON)
   {
        min = 1; // number stencil bits we need at least.
   }

   if (stateset.getAttribute(StateAttribute::STENCIL))
   {
        min = 1; // number stencil bits we need at least.
   }
   
   if (min>_vs->getMinimumNumStencilBits())
   {
        // only update if new minimum exceeds previous minimum.
        _vs->setMinimumNumStencilBits(min);
   }    
}

void VisualsRequirementsVisitor::apply(Node& node)
{
    osg::StateSet* stateset = node.getStateSet();
    if (stateset) applyStateSet(*stateset);

    traverse(node);
}

void VisualsRequirementsVisitor::apply(Geode& geode)
{
    osg::StateSet* geode_stateset = geode.getStateSet();
    if (geode_stateset) applyStateSet(*geode_stateset);
    
    for(int i = 0; i < geode.getNumDrawables(); i++ )
    {
        osg::StateSet* stateset = geode.getDrawable(i)->getStateSet();
        if (stateset) applyStateSet(*stateset);
    }
}

void VisualsRequirementsVisitor::apply(Impostor& impostor)
{
    if (!_vs) _vs = new osg::VisualsSettings;

    unsigned int min = 1; // number alpha bits we need at least.
    if (min>_vs->getMinimumNumAlphaBits())
    {
        // only update if new minimum exceeds previous minimum.
        _vs->setMinimumNumAlphaBits(min);
    }
    
    apply((Node&)impostor);
}
