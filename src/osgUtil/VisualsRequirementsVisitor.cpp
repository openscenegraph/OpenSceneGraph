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
    _requiresDoubleBuffer = true;
    _requiresRBG = true;
    _requiresDepthBuffer = true;
    _minimumNumberAlphaBits = 0;
    _minimumNumberStencilBits = 0;

}

void VisualsRequirementsVisitor::applyStateSet(StateSet& stateset)
{
   if (stateset.getMode(GL_STENCIL_TEST) & StateAttribute::ON)
   {
        _minimumNumberStencilBits = 1;
   }

   if (stateset.getAttribute(StateAttribute::STENCIL))
   {
        _minimumNumberStencilBits = 1;
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
    _minimumNumberAlphaBits = 1;
    apply((Node&)impostor);
}
