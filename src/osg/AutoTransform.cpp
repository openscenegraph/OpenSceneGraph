#include <osg/AutoTransform>

using namespace osg;

AutoTransform::AutoTransform()
{
}

AutoTransform::AutoTransform(const AutoTransform& transform,const CopyOp& copyop):
    Group(transform,copyop),
    _calcTransformCallback(dynamic_cast<CalcTransformCallback*>(copyop(transform._calcTransformCallback.get())))
{    
}


AutoTransform::~AutoTransform()
{
}

const bool AutoTransform::computeBound() const
{
    // can't calc the bound of a automatically transform object,
    // it position isn't know until cull time.
    return false;
}
