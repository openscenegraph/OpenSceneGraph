#include <osgUtil/AppVisitor>

using namespace osg;
using namespace osgUtil;

AppVisitor::AppVisitor():NodeVisitor(TRAVERSE_ACTIVE_CHILDREN)
{
}


AppVisitor::~AppVisitor()
{
}


void AppVisitor::reset()
{
}
