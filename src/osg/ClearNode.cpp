#include <osg/ClearNode>

#include <algorithm>

using namespace osg;

/**
 * ClearNode constructor.
 */
ClearNode::ClearNode()
{
    StateSet* stateset = osgNew StateSet;
    stateset->setRenderBinDetails(-1,"RenderBin");
    setStateSet(stateset);
    
    _requiresClear = true;
    _clearColor.set(0.0f,0.0f,0.0f,1.0f);  

}

