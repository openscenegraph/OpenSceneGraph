#include "osgUtil/DisplayListVisitor"
#include "osg/GeoSet"

using namespace osg;
using namespace osgUtil;

DisplayListVisitor::DisplayListVisitor(DisplayListMode mode)
{
    setTraverseMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    _displayListMode = mode;
}

void DisplayListVisitor::apply(osg::Geode& node)
{
    switch(_displayListMode)
    {
    case(SWITCH_OFF_DISPLAY_LISTS):
        {
            for(int i=0;i<node.getNumGeosets();++i)
            {
                node.getGeoSet(i)->setUseDisplayList(false);
            }
        }
        break;
    case(SWITCH_ON_DISPLAY_LISTS):
        {
            for(int i=0;i<node.getNumGeosets();++i)
            {
                node.getGeoSet(i)->setUseDisplayList(true);
            }
        }
        break;
    case(SWITCH_ON_AND_COMPILE_DISPLAY_LISTS):
        node.compileGeoSets();
        break;
    }
}
