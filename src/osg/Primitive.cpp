#include <osg/Primitive>

using namespace osg;

void DrawArrays::draw() const 
{
    glDrawArrays(_mode,_first,_count);
}

void DrawArrays::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    functor.drawArrays(_mode,_first,_count);
}


void UByteDrawElements::draw() const 
{
    glDrawElements(_mode,size(),GL_UNSIGNED_BYTE,&front());
}

void UByteDrawElements::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}


void UShortDrawElements::draw() const 
{
    glDrawElements(_mode,size(),GL_UNSIGNED_SHORT,&front());
}

void UShortDrawElements::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}



void UIntDrawElements::draw() const 
{
    glDrawElements(_mode,size(),GL_UNSIGNED_INT,&front());
}

void UIntDrawElements::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}
