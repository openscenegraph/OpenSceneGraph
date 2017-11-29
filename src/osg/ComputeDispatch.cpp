#include <osg/ComputeDispatch>

using namespace osg;

ComputeDispatch::ComputeDispatch(const ComputeDispatch&o,const osg::CopyOp& copyop): 
    Drawable(o,copyop),
    _numGroupsX(o._numGroupsX),
    _numGroupsY(o._numGroupsY),
    _numGroupsZ(o._numGroupsZ)
{
}

void ComputeDispatch::drawImplementation(RenderInfo& renderInfo) const
{
    renderInfo.getState()->get<GLExtensions>()->glDispatchCompute(_numGroupsX, _numGroupsY, _numGroupsZ);
}
