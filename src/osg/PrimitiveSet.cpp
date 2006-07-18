/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/PrimitiveSet>
#include <osg/BufferObject>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

unsigned int PrimitiveSet::getNumPrimitives() const
{
    switch(_mode)
    {
        case(POINTS): return getNumIndices();
        case(LINES): return getNumIndices()/2;
        case(TRIANGLES): return getNumIndices()/3;
        case(QUADS): return getNumIndices()/4;
        case(LINE_STRIP):
        case(LINE_LOOP):
        case(TRIANGLE_STRIP):
        case(TRIANGLE_FAN):
        case(QUAD_STRIP):
        case(POLYGON): return 1;
    }
    return 0;
}

void DrawArrays::draw(State&, bool) const 
{
    glDrawArrays(_mode,_first,_count);
}

void DrawArrays::accept(PrimitiveFunctor& functor) const
{
    functor.drawArrays(_mode,_first,_count);
}

void DrawArrays::accept(PrimitiveIndexFunctor& functor) const
{
    functor.drawArrays(_mode,_first,_count);
}

unsigned int DrawArrayLengths::getNumPrimitives() const
{
    switch(_mode)
    {
        case(POINTS): return getNumIndices();
        case(LINES): return getNumIndices()/2;
        case(TRIANGLES): return getNumIndices()/3;
        case(QUADS): return getNumIndices()/4;
        case(LINE_STRIP):
        case(LINE_LOOP):
        case(TRIANGLE_STRIP):
        case(TRIANGLE_FAN):
        case(QUAD_STRIP):
        case(POLYGON): return size();
    }
    return 0;
}

void DrawArrayLengths::draw(State&, bool) const
{
    GLint first = _first;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        glDrawArrays(_mode,first,*itr);
        first += *itr;
    }
}

void DrawArrayLengths::accept(PrimitiveFunctor& functor) const
{
    GLint first = _first;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        functor.drawArrays(_mode,first,*itr);
        first += *itr;
    }
}

void DrawArrayLengths::accept(PrimitiveIndexFunctor& functor) const
{
    GLint first = _first;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        functor.drawArrays(_mode,first,*itr);
        first += *itr;
    }
}

unsigned int DrawArrayLengths::getNumIndices() const
{
    unsigned int count = 0;
    for(vector_type::const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        count += *itr;
    }
    return count;
}

DrawElementsUByte::~DrawElementsUByte()
{
    releaseGLObjects();
}

void DrawElementsUByte::releaseGLObjects(State* state) const
{
    if (state)
    {
        unsigned int contextID = state->getContextID();
        if (_vboList[contextID]._objectID != 0)
        {
            BufferObject::deleteBufferObject(contextID,_vboList[contextID]._objectID);
            _vboList[contextID]._objectID = 0;
        }
    }
    else
    {
        for(unsigned int i=0;i<_vboList.size();++i)
        {
            if (_vboList[i]._objectID != 0)
            {
                BufferObject::deleteBufferObject(i,_vboList[i]._objectID);
                _vboList[i]._objectID = 0;
            }
        }
    }
}

void DrawElementsUByte::draw(State& state, bool useVertexBufferObjects) const 
{
    if (useVertexBufferObjects)
    {
        unsigned int contextID = state.getContextID();
        const BufferObject::Extensions* extensions = BufferObject::getExtensions(contextID, true);

        GLuint& buffer = _vboList[contextID]._objectID;
        if (!buffer)
        {
            extensions->glGenBuffers(1, &buffer);
            extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
            extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, size() * 1, &front(), GL_STATIC_DRAW_ARB);
            // osg::notify(osg::NOTICE)<<"Generating ubyte buffer"<<buffer<<std::endl;
            
            _vboList[contextID]._modifiedCount = _modifiedCount;
        }
        else
        {
            extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
        
            if (_vboList[contextID]._modifiedCount != _modifiedCount) 
            {
                extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, size() * 1, &front(), GL_STATIC_DRAW_ARB);
                _vboList[contextID]._modifiedCount = _modifiedCount;
            }
        }
    
        glDrawElements(_mode, size(), GL_UNSIGNED_BYTE, 0);
        
        extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    else 
    {
        glDrawElements(_mode, size(), GL_UNSIGNED_BYTE, &front());
    }
}

void DrawElementsUByte::accept(PrimitiveFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUByte::accept(PrimitiveIndexFunctor& functor) const
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


DrawElementsUShort::~DrawElementsUShort()
{
    releaseGLObjects();
}

void DrawElementsUShort::releaseGLObjects(State* state) const
{
    if (state)
    {
        unsigned int contextID = state->getContextID();
        if (_vboList[contextID]._objectID != 0)
        {
            BufferObject::deleteBufferObject(contextID,_vboList[contextID]._objectID);
            _vboList[contextID]._objectID = 0;
        }
    }
    else
    {
        for(unsigned int i=0;i<_vboList.size();++i)
        {
            if (_vboList[i]._objectID != 0)
            {
                BufferObject::deleteBufferObject(i,_vboList[i]._objectID);
                _vboList[i]._objectID = 0;
            }
        }
    }
}

void DrawElementsUShort::draw(State& state, bool useVertexBufferObjects) const 
{
    if (useVertexBufferObjects)
    {
        unsigned int contextID = state.getContextID();
        const BufferObject::Extensions* extensions = BufferObject::getExtensions(contextID, true);

        GLuint& buffer = _vboList[contextID]._objectID;
        if (!buffer)
        {
        
            extensions->glGenBuffers(1, &buffer);
            extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
            extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, size() * 2, &front(), GL_STATIC_DRAW_ARB);
            // osg::notify(osg::NOTICE)<<"Generating ushort buffer"<<buffer<<std::endl;
            
            _vboList[contextID]._modifiedCount = _modifiedCount;
        }
        else
        {
            extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
        
            if (_vboList[contextID]._modifiedCount != _modifiedCount) 
            {
                extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, size() * 2, &front(), GL_STATIC_DRAW_ARB);
                _vboList[contextID]._modifiedCount = _modifiedCount;
            }
        }
    
        glDrawElements(_mode, size(), GL_UNSIGNED_SHORT, 0);

        extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    else 
    {
        glDrawElements(_mode, size(), GL_UNSIGNED_SHORT, &front());
    }
}

void DrawElementsUShort::accept(PrimitiveFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUShort::accept(PrimitiveIndexFunctor& functor) const
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


DrawElementsUInt::~DrawElementsUInt()
{
    releaseGLObjects();
}

void DrawElementsUInt::releaseGLObjects(State* state) const
{
    if (state)
    {
        unsigned int contextID = state->getContextID();
        if (_vboList[contextID]._objectID != 0)
        {
            BufferObject::deleteBufferObject(contextID,_vboList[contextID]._objectID);
            _vboList[contextID]._objectID = 0;
        }
    }
    else
    {
        for(unsigned int i=0;i<_vboList.size();++i)
        {
            if (_vboList[i]._objectID != 0)
            {
                BufferObject::deleteBufferObject(i,_vboList[i]._objectID);
                _vboList[i]._objectID = 0;
            }
        }
    }
}

void DrawElementsUInt::draw(State& state, bool useVertexBufferObjects) const 
{
    if (useVertexBufferObjects)
    {
        unsigned int contextID = state.getContextID();
        const BufferObject::Extensions* extensions = BufferObject::getExtensions(contextID, true);
        
        GLuint& buffer = _vboList[contextID]._objectID;
        if (!buffer)
        {
            extensions->glGenBuffers(1, &buffer);
            extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
            extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, size() * 4, &front(), GL_STATIC_DRAW_ARB);
            // osg::notify(osg::NOTICE)<<"Generating buffer int"<<buffer<<std::endl;
            
            _vboList[contextID]._modifiedCount = _modifiedCount;
        }
        else
        {
            // osg::notify(osg::NOTICE)<<"binding buffer int"<<buffer<<std::endl;
            extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
        
            if (_vboList[contextID]._modifiedCount != _modifiedCount) 
            {
                extensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, size() * 4, &front(), GL_STATIC_DRAW_ARB);
                _vboList[contextID]._modifiedCount = _modifiedCount;
            }
        }
    
        glDrawElements(_mode, size(), GL_UNSIGNED_INT, 0);
        
        extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    else 
    {
        glDrawElements(_mode, size(), GL_UNSIGNED_INT, &front());
    }
}

void DrawElementsUInt::accept(PrimitiveFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,size(),&front());
}

void DrawElementsUInt::accept(PrimitiveIndexFunctor& functor) const
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
