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

void DrawArrayLengths::draw() const
{
    GLint first = _first;
    for(VectorSizei::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        glDrawArrays(_mode,first,*itr);
        first += *itr;
    }
}

void DrawArrayLengths::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    GLint first = _first;
    for(VectorSizei::iterator itr=begin();
        itr!=end();
        ++itr)
    {
        functor.drawArrays(_mode,first,*itr);
        first += *itr;
    }
}



void DrawElementsUByte::draw() const 
{
    glDrawElements(_mode,size(),GL_UNSIGNED_BYTE,&front());
}

void DrawElementsUByte::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUByte::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}


void DrawElementsUShort::draw() const 
{
    glDrawElements(_mode,size(),GL_UNSIGNED_SHORT,&front());
}

void DrawElementsUShort::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUShort::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}


void DrawElementsUInt::draw() const 
{
    glDrawElements(_mode,size(),GL_UNSIGNED_INT,&front());
}

void DrawElementsUInt::applyPrimitiveOperation(Drawable::PrimitiveFunctor& functor)
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUInt::offsetIndices(int offset)
{
    for(iterator itr=begin();
        itr!=end();
        ++itr)
    {
        *itr += offset;
    }
}
