#include "osgUtil/DisplayListVisitor"
#include "osg/Drawable"

using namespace osg;
using namespace osgUtil;

DisplayListVisitor::DisplayListVisitor(DisplayListMode mode)
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    _displayListMode = mode;
    
    _localState = new osg::State;
    _externalState = NULL;
    
    _activeState = _localState;
}


void DisplayListVisitor::apply(osg::Geode& node)
{
    switch(_displayListMode)
    {
        case(SWITCH_OFF_DISPLAY_LISTS):
        {
            for(int i=0;i<node.getNumDrawables();++i)
            {
                node.getDrawable(i)->setUseDisplayList(false);
            }
        }
        break;
        case(SWITCH_ON_DISPLAY_LISTS):
        {
            for(int i=0;i<node.getNumDrawables();++i)
            {
                node.getDrawable(i)->setUseDisplayList(true);
            }
        }
        break;
        case(COMPILE_ON_DISPLAY_LISTS):
        {
            for(int i=0;i<node.getNumDrawables();++i)
            {
                if (node.getDrawable(i)->getUseDisplayList())
                {
                    node.getDrawable(i)->compile(*_activeState);
                }
            }
        }
        break;
        case(SWITCH_ON_AND_COMPILE_DISPLAY_LISTS):
        {
            node.compileDrawables(*_activeState);
        }
        break;
    }
}
