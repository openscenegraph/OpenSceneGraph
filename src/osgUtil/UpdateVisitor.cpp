#include <osgUtil/UpdateVisitor>

using namespace osg;
using namespace osgUtil;

UpdateVisitor::UpdateVisitor():NodeVisitor(UPDATE_VISITOR,TRAVERSE_ACTIVE_CHILDREN)
{
}


UpdateVisitor::~UpdateVisitor()
{
}


void UpdateVisitor::reset()
{
}
