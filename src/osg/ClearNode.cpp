#include <osg/ClearNode>

#include <algorithm>

using namespace osg;

/**
 * ClearNode constructor.
 */
ClearNode::ClearNode():
    _requiresClear(true),
    _clearColor(0.0f,0.0f,0.0f,1.0f)
{
    StateSet* stateset = new StateSet;
    stateset->setRenderBinDetails(-1,"RenderBin");
    setStateSet(stateset);
}

