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
 *
 * osg/PrimitiveSetIndirect
 * Author: Julien Valentin 2016-2017
*/

#ifndef OSG_INDIRECTPRIMITIVESET
#define OSG_INDIRECTPRIMITIVESET 1

#include <osg/PrimitiveSet>


namespace osg {

///common interface for IndirectCommandDrawArrayss
class OSG_EXPORT IndirectCommandDrawArrays: public BufferData
{
public:
    IndirectCommandDrawArrays() : BufferData() { setBufferObject(new DrawIndirectBufferObject()); }

    IndirectCommandDrawArrays(const IndirectCommandDrawArrays& copy,const CopyOp& copyop/*=CopyOp::SHALLOW_COPY*/) :
        BufferData(copy, copyop) {}

    virtual unsigned int getTotalDataSize() const { return getNumElements()*getElementSize(); }

    virtual unsigned int & count(const unsigned int&index)=0;
    virtual unsigned int & instanceCount(const unsigned int&index)=0;
    virtual unsigned int & first(const unsigned int&index)=0;
    virtual unsigned int & baseInstance(const unsigned int&index)=0;

    virtual unsigned int getElementSize() const = 0;
    virtual unsigned int getNumElements() const = 0;
    virtual void reserveElements(const unsigned int) = 0;
    virtual void resizeElements(const unsigned int) = 0;
};

class OSG_EXPORT IndirectCommandDrawElements: public BufferData
{
public:
    IndirectCommandDrawElements() : BufferData() { setBufferObject(new DrawIndirectBufferObject()); }

    IndirectCommandDrawElements(const IndirectCommandDrawElements& copy,const CopyOp& copyop/*=CopyOp::SHALLOW_COPY*/)
        : BufferData(copy, copyop) {}

    virtual unsigned int getTotalDataSize() const { return getNumElements()*getElementSize(); }

    virtual unsigned int & count(const unsigned int&index)=0;
    virtual unsigned int & instanceCount(const unsigned int&index)=0;
    virtual unsigned int & firstIndex(const unsigned int&index)=0;
    virtual unsigned int & baseVertex(const unsigned int&index)=0;
    virtual unsigned int & baseInstance(const unsigned int&index)=0;

    virtual unsigned int getElementSize()const = 0;
    virtual unsigned int getNumElements() const = 0;
    virtual void reserveElements(const unsigned int) = 0;
    virtual void resizeElements(const unsigned int) = 0;
};



///  DrawArraysCommand
struct DrawArraysIndirectCommand
{
    DrawArraysIndirectCommand(unsigned int pcount = 0, unsigned int pinstanceCount = 0, unsigned int pfirst = 0, unsigned int pbaseInstance = 0) :
        count(pcount), instanceCount(pinstanceCount), first(pfirst), baseInstance(pbaseInstance) {}

    unsigned int  count;
    unsigned int  instanceCount;
    unsigned int  first;
    unsigned int  baseInstance;
};

/// default implementation of IndirectCommandDrawArrays
/// DefaultIndirectCommandDrawArrays to be hosted on GPU
class DefaultIndirectCommandDrawArrays: public IndirectCommandDrawArrays, public  MixinVector<DrawArraysIndirectCommand>
{
public:
    META_Object(osg,DefaultIndirectCommandDrawArrays)

    DefaultIndirectCommandDrawArrays() : IndirectCommandDrawArrays(), MixinVector<DrawArraysIndirectCommand>() {}
    DefaultIndirectCommandDrawArrays(const DefaultIndirectCommandDrawArrays& copy,const CopyOp& copyop/*=CopyOp::SHALLOW_COPY*/) :
        IndirectCommandDrawArrays(copy, copyop),MixinVector<DrawArraysIndirectCommand>() {}

    virtual const GLvoid* getDataPointer() const { return empty()?0:&front(); }
    virtual unsigned int getElementSize()const { return 16u; };
    virtual unsigned int getNumElements() const { return static_cast<unsigned int>(size()); }
    virtual void reserveElements(const unsigned int n) {reserve(n);}
    virtual void resizeElements(const unsigned int n) {resize(n);}

    virtual unsigned int & count(const unsigned int&index) { return at(index).count; }
    virtual unsigned int & instanceCount(const unsigned int&index) { return at(index).instanceCount; }
    virtual unsigned int & first(const unsigned int&index) { return at(index).first; }
    virtual unsigned int & baseInstance(const unsigned int&index) { return at(index).baseInstance; }

};


/// default implementation of IndirectCommandDrawElements
/// DrawElementsCommand
struct DrawElementsIndirectCommand
{
    DrawElementsIndirectCommand(unsigned int pcount = 0, unsigned int pinstanceCount = 0, unsigned int pfirstIndex = 0, unsigned int pbaseVertex = 0, unsigned int pbaseInstance = 0) :
        count(pcount), instanceCount(pinstanceCount), firstIndex(pfirstIndex), baseVertex(pbaseVertex), baseInstance(pbaseInstance) {}

    unsigned int  count;
    unsigned int  instanceCount;
    unsigned int  firstIndex;
    unsigned int  baseVertex;
    unsigned int  baseInstance;
};

/// vector of DrawElementsCommand to be hosted on GPU
class DefaultIndirectCommandDrawElements: public IndirectCommandDrawElements, public MixinVector<DrawElementsIndirectCommand>
{
public:
    META_Object(osg,DefaultIndirectCommandDrawElements)

    DefaultIndirectCommandDrawElements() : IndirectCommandDrawElements(), MixinVector<DrawElementsIndirectCommand>() {}

    DefaultIndirectCommandDrawElements(const DefaultIndirectCommandDrawElements& copy,const CopyOp& copyop/*=CopyOp::SHALLOW_COPY*/) :
        IndirectCommandDrawElements(copy, copyop), MixinVector<DrawElementsIndirectCommand>() {}

    virtual const GLvoid*   getDataPointer() const { return empty()?0:&front(); }
    virtual unsigned int getNumElements() const { return static_cast<unsigned int>(size()); }
    virtual unsigned int getElementSize()const { return 20u; };
    virtual void reserveElements(const unsigned int n) {reserve(n);}
    virtual void resizeElements(const unsigned int n) {resize(n);}

    virtual unsigned int & count(const unsigned int&index) { return at(index).count; }
    virtual unsigned int & instanceCount(const unsigned int&index) { return at(index).instanceCount; }
    virtual unsigned int & firstIndex(const unsigned int&index) { return at(index).firstIndex; }
    virtual unsigned int & baseVertex(const unsigned int&index) { return at(index).baseVertex; }
    virtual unsigned int & baseInstance(const unsigned int&index) { return at(index).baseInstance; }


};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief The DrawElementsIndirect base PrimitiveSet
///
class OSG_EXPORT DrawElementsIndirect : public DrawElements
{
public:

    DrawElementsIndirect(Type primType=PrimitiveType, GLenum mode = 0,unsigned int firstCommand = 0, GLsizei stride = 0) :
        DrawElements(primType,mode, 0),_firstCommand(firstCommand),_stride(stride) { setIndirectCommandArray(new DefaultIndirectCommandDrawElements()); }

    DrawElementsIndirect(const DrawElementsIndirect& rhs,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        DrawElements(rhs,copyop),_firstCommand(rhs._firstCommand), _stride(rhs._stride) { _indirectCommandArray=(DefaultIndirectCommandDrawElements*)copyop(rhs._indirectCommandArray.get()); }

    /// set command array of this indirect primitive set
    inline void setIndirectCommandArray(IndirectCommandDrawElements*idc)
    {
        _indirectCommandArray = idc;
        //ensure bo of idc is of the correct type
        if(!dynamic_cast<DrawIndirectBufferObject* >(_indirectCommandArray->getBufferObject()))
            _indirectCommandArray->setBufferObject(new DrawIndirectBufferObject());
    }

    /// get command array of this indirect primitive set
    inline IndirectCommandDrawElements* getIndirectCommandArray() { return _indirectCommandArray.get(); }
    inline const IndirectCommandDrawElements* getIndirectCommandArray() const { return _indirectCommandArray.get(); }

    ///Further methods are for advanced DI when you plan to use your own IndirectCommandElement (stride)
    ///or if you want to draw a particular command index of the IndirectCommandElement(FirstCommandToDraw)

    /// set offset of the first command to draw in the IndirectCommandDrawArrays
    inline void setFirstCommandToDraw( unsigned int  i) { _firstCommand = i; }

    /// get offset of the first command in the IndirectCommandDrawArrays
    inline unsigned int getFirstCommandToDraw() const { return _firstCommand; }

    /// stride (to set if you use custom CommandArray)
    inline void setStride( GLsizei  i) { _stride=i; }

    /// stride (to set if you use custom CommandArray)
    inline GLsizei getStride() const { return _stride; }

    virtual unsigned int getNumPrimitives() const=0;

protected:
    virtual ~DrawElementsIndirect() {}

    unsigned int _firstCommand;
    GLsizei _stride;
    ref_ptr<IndirectCommandDrawElements> _indirectCommandArray;
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief The DrawElementsIndirectUByte PrimitiveSet
///
class OSG_EXPORT DrawElementsIndirectUByte : public DrawElementsIndirect, public VectorGLubyte
{
public:

    typedef VectorGLubyte vector_type;

    DrawElementsIndirectUByte(GLenum mode = 0/*,unsigned int firstCommand = 0, GLsizei stride = 0*/) :
        DrawElementsIndirect(DrawElementsUByteIndirectPrimitiveType,mode) {}

    DrawElementsIndirectUByte(const DrawElementsIndirectUByte& array, const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        DrawElementsIndirect(array,copyop),
        vector_type(array) {}

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     * \param ptr Pointer to a GLubyte to copy index data from.
     */
    DrawElementsIndirectUByte(GLenum mode, unsigned int no, const GLubyte* ptr) :
        DrawElementsIndirect(DrawElementsUByteIndirectPrimitiveType,mode),
        vector_type(ptr,ptr+no) {}

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     */
    DrawElementsIndirectUByte(GLenum mode, unsigned int no) :
        DrawElementsIndirect(DrawElementsUByteIndirectPrimitiveType,mode),
        vector_type(no) {}

    virtual Object* cloneType() const { return new DrawElementsIndirectUByte(); }
    virtual Object* clone(const CopyOp& copyop) const { return new DrawElementsIndirectUByte(*this,copyop); }
    virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const DrawElementsIndirectUByte*>(obj)!=NULL; }
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "DrawElementsIndirectUByte"; }

    virtual const GLvoid*   getDataPointer() const { return empty()?0:&front(); }
    virtual unsigned int    getTotalDataSize() const { return static_cast<unsigned int>(size()); }
    virtual bool            supportsBufferObject() const { return false; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;

    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;

    virtual unsigned int getNumIndices() const { return static_cast<unsigned int>(size()); }
    virtual unsigned int index(unsigned int pos) const { return (*this)[pos]; }
    virtual void offsetIndices(int offset);

    virtual GLenum getDataType() { return GL_UNSIGNED_BYTE; }

    virtual void resizeElements(unsigned int numIndices) { resize(numIndices); }
    virtual void reserveElements(unsigned int numIndices) { reserve(numIndices); }

    virtual void setElement(unsigned int i, unsigned int v) { (*this)[i] = v; }
    virtual unsigned int getElement(unsigned int i) { return (*this)[i]; }

    virtual void addElement(unsigned int v) { push_back(GLubyte(v)); }
    virtual unsigned int getNumPrimitives() const;

protected:

    virtual ~DrawElementsIndirectUByte();
};


///////////////////////////////////////////////////////////////////////////////////////
/// \brief The DrawElementsIndirectUShort PrimitiveSet
///
class OSG_EXPORT DrawElementsIndirectUShort : public DrawElementsIndirect, public VectorGLushort
{
public:

    typedef VectorGLushort vector_type;

    DrawElementsIndirectUShort(GLenum mode = 0/*,unsigned int firstCommand = 0, GLsizei stride = 0*/) :
        DrawElementsIndirect(DrawElementsUShortIndirectPrimitiveType,mode) {}

    DrawElementsIndirectUShort(const DrawElementsIndirectUShort& array,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        DrawElementsIndirect(array,copyop),
        vector_type(array) {}

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     * \param ptr Pointer to a GLushort to copy index data from.
     */
    DrawElementsIndirectUShort(GLenum mode, unsigned int no, const GLushort* ptr) :
        DrawElementsIndirect(DrawElementsUShortIndirectPrimitiveType,mode),
        vector_type(ptr,ptr+no) {}

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     */
    DrawElementsIndirectUShort(GLenum mode, unsigned int no) :
        DrawElementsIndirect(DrawElementsUShortIndirectPrimitiveType,mode),
        vector_type(no) {}

    template <class InputIterator>
    DrawElementsIndirectUShort(GLenum mode, InputIterator first,InputIterator last) :
        DrawElementsIndirect(DrawElementsUShortIndirectPrimitiveType,mode),
        vector_type(first,last) {}

    virtual Object* cloneType() const { return new DrawElementsIndirectUShort(); }
    virtual Object* clone(const CopyOp& copyop) const { return new DrawElementsIndirectUShort(*this,copyop); }
    virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const DrawElementsIndirectUShort*>(obj)!=NULL; }
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "DrawElementsIndirectUShort"; }

    virtual const GLvoid*   getDataPointer() const { return empty()?0:&front(); }
    virtual unsigned int    getTotalDataSize() const { return 2u*static_cast<unsigned int>(size()); }
    virtual bool            supportsBufferObject() const { return false; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;

    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;

    virtual unsigned int getNumIndices() const { return static_cast<unsigned int>(size()); }
    virtual unsigned int index(unsigned int pos) const { return (*this)[pos]; }
    virtual void offsetIndices(int offset);

    virtual GLenum getDataType() { return GL_UNSIGNED_SHORT; }
    virtual void resizeElements(unsigned int numIndices) { resize(numIndices); }
    virtual void reserveElements(unsigned int numIndices) { reserve(numIndices); }

    virtual void setElement(unsigned int i, unsigned int v) { (*this)[i] = v; }
    virtual unsigned int getElement(unsigned int i) { return (*this)[i]; }

    virtual void addElement(unsigned int v) { push_back(GLushort(v)); }
    virtual unsigned int getNumPrimitives() const;

protected:

    virtual ~DrawElementsIndirectUShort();
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief The DrawElementsIndirectUInt PrimitiveSet
///
class OSG_EXPORT DrawElementsIndirectUInt : public DrawElementsIndirect, public VectorGLuint
{
public:

    typedef VectorGLuint vector_type;

    DrawElementsIndirectUInt(GLenum mode = 0/*,unsigned int firstCommand = 0, GLsizei stride = 0*/) :
        DrawElementsIndirect(DrawElementsUIntIndirectPrimitiveType,mode) {}

    DrawElementsIndirectUInt(const DrawElementsIndirectUInt& array,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        DrawElementsIndirect(array,copyop),
        vector_type(array) {}

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     * \param ptr Pointer to a GLunsigned int to copy index data from.
     */
    DrawElementsIndirectUInt(GLenum mode, unsigned int no, const GLuint* ptr) :
        DrawElementsIndirect(DrawElementsUIntIndirectPrimitiveType,mode),
        vector_type(ptr,ptr+no) {}

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     */
    DrawElementsIndirectUInt(GLenum mode, unsigned int no) :
        DrawElementsIndirect(DrawElementsUIntIndirectPrimitiveType,mode),
        vector_type(no) {}

    template <class InputIterator>
    DrawElementsIndirectUInt(GLenum mode, InputIterator first,InputIterator last) :
        DrawElementsIndirect(DrawElementsUIntIndirectPrimitiveType,mode),
        vector_type(first,last) {}

    virtual Object* cloneType() const { return new DrawElementsIndirectUInt(); }
    virtual Object* clone(const CopyOp& copyop) const { return new DrawElementsIndirectUInt(*this,copyop); }
    virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const DrawElementsIndirectUInt*>(obj)!=NULL; }
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "DrawElementsIndirectUInt"; }

    virtual const GLvoid*   getDataPointer() const { return empty()?0:&front(); }
    virtual unsigned int    getTotalDataSize() const { return 4u*static_cast<unsigned int>(size()); }
    virtual bool            supportsBufferObject() const { return false; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;

    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;

    virtual unsigned int getNumIndices() const { return static_cast<unsigned int>(size()); }
    virtual unsigned int index(unsigned int pos) const { return (*this)[pos]; }
    virtual void offsetIndices(int offset);

    virtual GLenum getDataType() { return GL_UNSIGNED_INT; }
    virtual void resizeElements(unsigned int numIndices) { resize(numIndices); }
    virtual void reserveElements(unsigned int numIndices) { reserve(numIndices); }
    virtual void setElement(unsigned int i, unsigned int v) { (*this)[i] = v; }
    virtual unsigned int getElement(unsigned int i) { return (*this)[i]; }
    virtual void addElement(unsigned int v) { push_back(GLuint(v)); }

    virtual unsigned int getNumPrimitives() const;

protected:

    virtual ~DrawElementsIndirectUInt();
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief The MultiDrawElementsIndirect PrimitiveSets
///
class OSG_EXPORT MultiDrawElementsIndirectUShort : public DrawElementsIndirectUShort
{
public:
    MultiDrawElementsIndirectUShort(GLenum mode = 0/*,unsigned int firstCommand = 0, GLsizei stride = 0*/) :
        DrawElementsIndirectUShort(mode),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUShortIndirectPrimitiveType));
    }

    MultiDrawElementsIndirectUShort(const MultiDrawElementsIndirectUShort& mdi,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        DrawElementsIndirectUShort(mdi,copyop),
        _count(mdi._count)
    {
    }

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     * \param ptr Pointer to a GLunsigned int to copy index data from.
     */
    MultiDrawElementsIndirectUShort(GLenum mode, unsigned int no, const GLushort* ptr) :
        DrawElementsIndirectUShort(mode,no,ptr),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUShortIndirectPrimitiveType));
    }

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     */
    MultiDrawElementsIndirectUShort(GLenum mode, unsigned int no) :
        DrawElementsIndirectUShort(mode,no),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUShortIndirectPrimitiveType));
    }

    template <class InputIterator>
    MultiDrawElementsIndirectUShort(GLenum mode, InputIterator first,InputIterator last) :
        DrawElementsIndirectUShort(mode,first,last),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUShortIndirectPrimitiveType));
    }

    virtual osg::Object* cloneType() const { return new MultiDrawElementsIndirectUShort(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new MultiDrawElementsIndirectUShort(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MultiDrawElementsIndirectUShort*>(obj)!=NULL; }
    virtual const char* className() const { return "MultiDrawElementsIndirectUShort"; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;
    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;
    virtual unsigned int getNumPrimitives() const;

    ///if you want to draw a subset of the IndirectCommandElement(FirstCommandToDraw,NumCommandsToDraw)

    /// count of Indirect Command to execute
    inline void setNumCommandsToDraw( unsigned int i) { _count=i; }
    /// count of Indirect Command to execute
    inline unsigned int getNumCommandsToDraw()const { return _count; }

protected:
    unsigned int _count;
    virtual ~MultiDrawElementsIndirectUShort();
};

class OSG_EXPORT MultiDrawElementsIndirectUByte : public DrawElementsIndirectUByte
{
public:
    MultiDrawElementsIndirectUByte(GLenum mode = 0) :
        DrawElementsIndirectUByte(mode),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUByteIndirectPrimitiveType));

    }

    MultiDrawElementsIndirectUByte(const MultiDrawElementsIndirectUByte& mdi,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        DrawElementsIndirectUByte(mdi,copyop),
        _count(mdi._count)
    {
    }

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     * \param ptr Pointer to a GLunsigned int to copy index data from.
     */
    MultiDrawElementsIndirectUByte(GLenum mode, unsigned int no, const GLubyte* ptr) :
        DrawElementsIndirectUByte(mode,no,ptr),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUByteIndirectPrimitiveType));
    }

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     */
    MultiDrawElementsIndirectUByte(GLenum mode, unsigned int no) :
        DrawElementsIndirectUByte(mode,no),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUByteIndirectPrimitiveType));
    }

    template <class InputIterator>
    MultiDrawElementsIndirectUByte(GLenum mode, InputIterator first,InputIterator last) :
        DrawElementsIndirectUByte(mode,first,last),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUByteIndirectPrimitiveType));
    }

    virtual osg::Object* cloneType() const { return new MultiDrawElementsIndirectUByte(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new MultiDrawElementsIndirectUByte(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MultiDrawElementsIndirectUByte*>(obj)!=NULL; }
    virtual const char* className() const { return "MultiDrawElementsIndirectUByte"; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;
    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;
    virtual unsigned int getNumPrimitives() const;

    /// count of Indirect Command to execute
    inline void setNumCommandsToDraw( unsigned int i) { _count=i; }
    /// count of Indirect Command to execute
    inline unsigned int getNumCommandsToDraw()const { return _count; }

protected:
    unsigned int _count;
    virtual ~MultiDrawElementsIndirectUByte();
};

class OSG_EXPORT MultiDrawElementsIndirectUInt : public DrawElementsIndirectUInt
{
public:
    MultiDrawElementsIndirectUInt(GLenum mode = 0) :
        DrawElementsIndirectUInt(mode),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUIntIndirectPrimitiveType));
    }

    MultiDrawElementsIndirectUInt(const MultiDrawElementsIndirectUInt& mdi,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        DrawElementsIndirectUInt(mdi,copyop),
        _count(mdi._count)
    {}

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     * \param ptr Pointer to a GLunsigned int to copy index data from.
     */
    MultiDrawElementsIndirectUInt(GLenum mode, unsigned int no, const GLuint* ptr) :
        DrawElementsIndirectUInt(mode,no,ptr),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUIntIndirectPrimitiveType));
    }

    /**
     * \param mode One of osg::PrimitiveSet::Mode. Determines the type of primitives used.
     * \param no Number of intended elements. This will be the size of the underlying vector.
     */
    MultiDrawElementsIndirectUInt(GLenum mode, unsigned int no) :
        DrawElementsIndirectUInt(mode,no),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUIntIndirectPrimitiveType));
    }

    template <class InputIterator>
    MultiDrawElementsIndirectUInt(GLenum mode, InputIterator first,InputIterator last) :
        DrawElementsIndirectUInt(mode,first,last),
        _count(0)
    {
        _primitiveType=(Type(MultiDrawElementsUIntIndirectPrimitiveType));
    }

    virtual osg::Object* cloneType() const { return new MultiDrawElementsIndirectUInt(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new MultiDrawElementsIndirectUInt(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MultiDrawElementsIndirectUInt*>(obj)!=NULL; }
    virtual const char* className() const { return "MultiDrawElementsIndirectUInt"; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;
    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;
    virtual unsigned int getNumPrimitives() const;

    /// count of Indirect Command to execute
    inline void setNumCommandsToDraw( unsigned int i) { _count=i; }
    /// count of Indirect Command to execute
    inline unsigned int getNumCommandsToDraw()const { return _count; }

protected:
    unsigned int _count;
    virtual ~MultiDrawElementsIndirectUInt();
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief The MultiDrawArraysIndirect PrimitiveSet
///
class OSG_EXPORT DrawArraysIndirect : public osg::PrimitiveSet
{
public:

    DrawArraysIndirect(GLenum mode=0, unsigned int firstcommand = 0, GLsizei stride = 0) :
        osg::PrimitiveSet(Type(DrawArraysIndirectPrimitiveType), mode),
        _firstCommand(firstcommand),
        _stride(stride)
    {
        setIndirectCommandArray(new DefaultIndirectCommandDrawArrays);
    }

    DrawArraysIndirect(const DrawArraysIndirect& dal,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        osg::PrimitiveSet(dal,copyop),
        _firstCommand(dal._firstCommand),
        _stride(dal._stride),
        _indirectCommandArray((DefaultIndirectCommandDrawArrays*)copyop( dal._indirectCommandArray.get()))
    {
    }

    virtual osg::Object* cloneType() const { return new DrawArraysIndirect(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new DrawArraysIndirect(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const DrawArraysIndirect*>(obj)!=NULL; }
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "DrawArraysIndirect"; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;

    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;

    virtual unsigned int getNumIndices() const;
    virtual unsigned int index(unsigned int pos) const;
    virtual void offsetIndices(int offset);

    virtual unsigned int getNumPrimitives() const;

    /// stride (to set if you use custom CommandArray)
    inline void setStride( GLsizei i) { _stride=i; }

    /// stride (to set if you use custom CommandArray)
    inline GLsizei getStride()const { return _stride; }

    /// set offset of the first command in the IndirectCommandDrawArrays
    inline void setFirstCommandToDraw( unsigned int i) { _firstCommand=i; }

    /// get offset of the first command in the IndirectCommandDrawArrays
    inline unsigned int getFirstCommandToDraw() const { return _firstCommand; }

    inline void setIndirectCommandArray(IndirectCommandDrawArrays*idc)
    {
        _indirectCommandArray = idc;
        //ensure bo of idc is of the correct type
        if(!dynamic_cast<DrawIndirectBufferObject* >(_indirectCommandArray->getBufferObject()))
            _indirectCommandArray->setBufferObject(new DrawIndirectBufferObject());
    }
    inline const IndirectCommandDrawArrays* getIndirectCommandArray() const { return _indirectCommandArray.get(); }
    inline IndirectCommandDrawArrays* getIndirectCommandArray() { return _indirectCommandArray.get(); }

protected:

    unsigned int _firstCommand;
    GLsizei _stride;
    ref_ptr<IndirectCommandDrawArrays> _indirectCommandArray;

};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief The MultiDrawArraysIndirect PrimitiveSet
///
class OSG_EXPORT MultiDrawArraysIndirect : public DrawArraysIndirect
{
public:

    MultiDrawArraysIndirect(GLenum mode=0, unsigned int firstcommand = 0, unsigned int count = 0, GLsizei stride = 0) :
        osg::DrawArraysIndirect(mode, firstcommand, stride),
        _count(count)
    {
        _primitiveType=Type(MultiDrawArraysIndirectPrimitiveType);
    }

    MultiDrawArraysIndirect(const MultiDrawArraysIndirect& dal,const CopyOp& copyop=CopyOp::SHALLOW_COPY) :
        osg::DrawArraysIndirect(dal,copyop),
        _count(dal._count)
    {}

    virtual osg::Object* cloneType() const { return new MultiDrawArraysIndirect(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new MultiDrawArraysIndirect(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MultiDrawArraysIndirect*>(obj)!=NULL; }
    virtual const char* className() const { return "MultiDrawArraysIndirect"; }

    virtual void draw(State& state, bool useVertexBufferObjects) const;

    virtual void accept(PrimitiveFunctor& functor) const;
    virtual void accept(PrimitiveIndexFunctor& functor) const;

    virtual unsigned int getNumIndices() const;
    virtual unsigned int index(unsigned int pos) const;
    virtual void offsetIndices(int offset);

    virtual unsigned int getNumPrimitives() const;

    /// count of Indirect Command to execute
    inline void setNumCommandsToDraw( unsigned int i) { _count=i; }
    /// count of Indirect Command to execute
    inline unsigned int getNumCommandsToDraw()const { return _count; }

protected:
    unsigned int _count;

};

}

#endif
