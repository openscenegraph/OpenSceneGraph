#include <osg/DispatchCompute>

using namespace osg;

DispatchCompute::DispatchCompute(const DispatchCompute&o,const osg::CopyOp& copyop):
    Drawable(o,copyop),
    _numGroupsX(o._numGroupsX),
    _numGroupsY(o._numGroupsY),
    _numGroupsZ(o._numGroupsZ)
{
}

void DispatchCompute::drawImplementation(RenderInfo& renderInfo) const
{
    renderInfo.getState()->get<GLExtensions>()->glDispatchCompute(_numGroupsX, _numGroupsY, _numGroupsZ);
}
