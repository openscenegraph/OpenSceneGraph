#include <osg/CollectOccludersVisitor>

using namespace osg;

CollectOccludersVisitor::CollectOccludersVisitor()
{
}

CollectOccludersVisitor::~CollectOccludersVisitor()
{
}

void CollectOccludersVisitor::reset()
{
    CullStack::reset();
}

void CollectOccludersVisitor::apply(osg::Node&)
{
}

void CollectOccludersVisitor::apply(osg::Transform& node)
{
}

void CollectOccludersVisitor::apply(osg::Projection& node)
{
}

void CollectOccludersVisitor::apply(osg::Switch& node)
{
}

void CollectOccludersVisitor::apply(osg::LOD& node)
{
}

void CollectOccludersVisitor::apply(osg::OccluderNode& node)
{
}


