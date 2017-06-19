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

#ifndef OSG_ARRAY
#define OSG_ARRAY 1

#include <osg/MixinVector>

#include <osg/Vec2b>
#include <osg/Vec3b>
#include <osg/Vec4b>
#include <osg/Vec2s>
#include <osg/Vec3s>
#include <osg/Vec4s>
#include <osg/Vec2i>
#include <osg/Vec3i>
#include <osg/Vec4i>
#include <osg/Vec2ub>
#include <osg/Vec3ub>
#include <osg/Vec4ub>
#include <osg/Vec2us>
#include <osg/Vec3us>
#include <osg/Vec4us>
#include <osg/Vec2ui>
#include <osg/Vec3ui>
#include <osg/Vec4ui>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Vec2d>
#include <osg/Vec3d>
#include <osg/Vec4d>
#include <osg/Matrix>
#include <osg/Matrixd>
#include <osg/Quat>

#include <osg/BufferObject>

#include <osg/Object>
#include <osg/GL>

namespace osg {

class ArrayVisitor;
class ConstArrayVisitor;

class ValueVisitor;
class ConstValueVisitor;

class OSG_EXPORT Array : public BufferData
{

    public:

        /// The type of data stored in this array.
        enum Type
        {
            ArrayType = 0,

            ByteArrayType     = 1,
            ShortArrayType    = 2,
            IntArrayType      = 3,

            UByteArrayType    = 4,
            UShortArrayType   = 5,
            UIntArrayType     = 6,

            FloatArrayType    = 7,
            DoubleArrayType   = 8,

            Vec2bArrayType    = 9,
            Vec3bArrayType    = 10,
            Vec4bArrayType    = 11,

            Vec2sArrayType    = 12,
            Vec3sArrayType    = 13,
            Vec4sArrayType    = 14,

            Vec2iArrayType    = 15,
            Vec3iArrayType    = 16,
            Vec4iArrayType    = 17,

            Vec2ubArrayType   = 18,
            Vec3ubArrayType   = 19,
            Vec4ubArrayType   = 20,

            Vec2usArrayType   = 21,
            Vec3usArrayType   = 22,
            Vec4usArrayType   = 23,

            Vec2uiArrayType   = 24,
            Vec3uiArrayType   = 25,
            Vec4uiArrayType   = 26,

            Vec2ArrayType     = 27,
            Vec3ArrayType     = 28,
            Vec4ArrayType     = 29,

            Vec2dArrayType    = 30,
            Vec3dArrayType    = 31,
            Vec4dArrayType    = 32,

            MatrixArrayType   = 33,
            MatrixdArrayType  = 34,

            QuatArrayType     = 35,

            UInt64ArrayType   = 36,
            Int64ArrayType    = 37,

            LastArrayType     = 37
                // If new array types are added, update this and
                // update Array::className() in src/osg/Array.cpp.
                // Array::Type values are from ArrayType to
                // LastArrayType, inclusive.
        };

        /// The scope of applicability of the values in this array
        enum Binding
        {
            BIND_UNDEFINED=-1,
            BIND_OFF=0,
            BIND_OVERALL=1,
            BIND_PER_PRIMITIVE_SET=2,
            BIND_PER_VERTEX=4
        };

        Array(Type arrayType=ArrayType,GLint dataSize=0,GLenum dataType=0, Binding binding=BIND_UNDEFINED):
            _arrayType(arrayType),
            _dataSize(dataSize),
            _dataType(dataType),
            _binding(binding),
            _normalize(false),
            _preserveDataType(false) {}

       Array(const Array& array,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            BufferData(array,copyop),
            _arrayType(array._arrayType),
            _dataSize(array._dataSize),
            _dataType(array._dataType),
            _binding(array._binding),
            _normalize(array._normalize),
            _preserveDataType(array._preserveDataType) {}

        virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const Array*>(obj)!=NULL; }
        virtual const char* libraryName() const { return "osg"; }

        /// Get the class name of this array.  Defined in src/osg/Array.cpp
        /// for all concrete array types listed below --- doesn't use traits.
        virtual const char* className() const;

        virtual void accept(ArrayVisitor&) = 0;
        virtual void accept(ConstArrayVisitor&) const = 0;

        virtual void accept(unsigned int index,ValueVisitor&) = 0;
        virtual void accept(unsigned int index,ConstValueVisitor&) const = 0;

        /** Return -1 if lhs element is less than rhs element, 0 if equal,
          * 1 if lhs element is greater than rhs element. */
        virtual int compare(unsigned int lhs,unsigned int rhs) const = 0;

        Type getType() const { return _arrayType; }
        GLint getDataSize() const { return _dataSize; }
        GLenum getDataType() const { return _dataType; }

        virtual osg::Array* asArray() { return this; }
        virtual const osg::Array* asArray() const { return this; }

        virtual unsigned int getElementSize() const = 0;
        virtual const GLvoid* getDataPointer() const = 0;
        virtual const GLvoid* getDataPointer(unsigned int index) const = 0;
        virtual unsigned int getTotalDataSize() const = 0;
        virtual unsigned int getNumElements() const = 0;
        virtual void reserveArray(unsigned int num) = 0;
        virtual void resizeArray(unsigned int num) = 0;


        /** Specify how this array should be passed to OpenGL.*/
        void setBinding(Binding binding) { _binding = binding; }

        /** Get how this array should be passed to OpenGL.*/
        Binding getBinding() const { return _binding; }


        /** Specify whether the array data should be normalized by OpenGL.*/
        void setNormalize(bool normalize) { _normalize = normalize; }

        /** Get whether the array data should be normalized by OpenGL.*/
        bool getNormalize() const { return _normalize; }


        /** Set hint to ask that the array data is passed via integer or double, or normal setVertexAttribPointer function.*/
        void setPreserveDataType(bool preserve) { _preserveDataType = preserve; }

        /** Get hint to ask that the array data is passed via integer or double, or normal setVertexAttribPointer function.*/
        bool getPreserveDataType() const { return _preserveDataType; }


        /** Frees unused space on this vector - i.e. the difference between size() and max_size() of the underlying vector.*/
        virtual void trim() {}

        /** Set the VertexBufferObject.*/
        inline void setVertexBufferObject(osg::VertexBufferObject* vbo) { setBufferObject(vbo); }

        /** Get the VertexBufferObject. If no VBO is assigned returns NULL*/
        inline osg::VertexBufferObject* getVertexBufferObject() { return dynamic_cast<osg::VertexBufferObject*>(_bufferObject.get()); }

        /** Get the const VertexBufferObject. If no VBO is assigned returns NULL*/
        inline const osg::VertexBufferObject* getVertexBufferObject() const { return dynamic_cast<const osg::VertexBufferObject*>(_bufferObject.get());  }

    protected:

        virtual ~Array() {}

        Type    _arrayType;
        GLint   _dataSize;
        GLenum  _dataType;
        Binding _binding;
        bool    _normalize;
        bool    _preserveDataType;
};

/** convenience function for getting the binding of array via a ptr that may be null.*/
inline osg::Array::Binding getBinding(const osg::Array* array) { return array ? array->getBinding() : osg::Array::BIND_OFF; }

/** convenience function for getting the binding of array via a ptr that may be null.*/
inline bool getNormalize(const osg::Array* array) { return array ? array->getNormalize() : false; }


/// A concrete array holding elements of type T.
template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
class TemplateArray : public Array, public MixinVector<T>
{
    public:

        TemplateArray(Binding binding=BIND_UNDEFINED) : Array(ARRAYTYPE,DataSize,DataType, binding) {}

        TemplateArray(const TemplateArray& ta,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            Array(ta,copyop),
            MixinVector<T>(ta) {}

        TemplateArray(unsigned int no) :
            Array(ARRAYTYPE,DataSize,DataType),
            MixinVector<T>(no) {}

        TemplateArray(unsigned int no,const T* ptr) :
            Array(ARRAYTYPE,DataSize,DataType),
            MixinVector<T>(ptr,ptr+no) {}

        TemplateArray(Binding binding, unsigned int no) :
            Array(ARRAYTYPE,DataSize,DataType, binding),
            MixinVector<T>(no) {}

        TemplateArray(Binding binding, unsigned int no,const T* ptr) :
            Array(ARRAYTYPE,DataSize,DataType, binding),
            MixinVector<T>(ptr,ptr+no) {}

        template <class InputIterator>
        TemplateArray(InputIterator first,InputIterator last) :
            Array(ARRAYTYPE,DataSize,DataType),
            MixinVector<T>(first,last) {}

        TemplateArray& operator = (const TemplateArray& array)
        {
            if (this==&array) return *this;
            this->assign(array.begin(),array.end());
            return *this;
        }

        virtual Object* cloneType() const { return new TemplateArray(); }
        virtual Object* clone(const CopyOp& copyop) const { return new TemplateArray(*this,copyop); }

        inline virtual void accept(ArrayVisitor& av);
        inline virtual void accept(ConstArrayVisitor& av) const;

        inline virtual void accept(unsigned int index,ValueVisitor& vv);
        inline virtual void accept(unsigned int index,ConstValueVisitor& vv) const;

        virtual int compare(unsigned int lhs,unsigned int rhs) const
        {
            const T& elem_lhs = (*this)[lhs];
            const T& elem_rhs = (*this)[rhs];
            if (elem_lhs<elem_rhs) return -1;
            if (elem_rhs<elem_lhs) return 1;
            return 0;
        }

        /** Frees unused space on this vector - i.e. the difference between size() and max_size() of the underlying vector.*/
        virtual void trim()
        {
            MixinVector<T>( *this ).swap( *this );
        }

        virtual unsigned int    getElementSize() const { return sizeof(ElementDataType); }
        virtual const GLvoid*   getDataPointer() const { if (!this->empty()) return &this->front(); else return 0; }
        virtual const GLvoid*   getDataPointer(unsigned int index) const { if (!this->empty()) return &((*this)[index]); else return 0; }
        virtual unsigned int    getTotalDataSize() const { return static_cast<unsigned int>(this->size()*sizeof(ElementDataType)); }
        virtual unsigned int    getNumElements() const { return static_cast<unsigned int>(this->size()); }
        virtual void reserveArray(unsigned int num) { this->reserve(num); }
        virtual void resizeArray(unsigned int num) { this->resize(num); }

        typedef T ElementDataType; // expose T

    protected:

        virtual ~TemplateArray() {}
};

class OSG_EXPORT IndexArray : public Array
{

    public:

        IndexArray(Type arrayType=ArrayType,GLint dataSize=0,GLenum dataType=0):
            Array(arrayType,dataSize,dataType) {}

        IndexArray(const Array& array,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            Array(array,copyop) {}

        virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const IndexArray*>(obj)!=NULL; }

        virtual unsigned int index(unsigned int pos) const = 0;

    protected:

        virtual ~IndexArray() {}
};

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
class TemplateIndexArray : public IndexArray, public MixinVector<T>
{
    public:

        TemplateIndexArray() : IndexArray(ARRAYTYPE,DataSize,DataType) {}

        TemplateIndexArray(const TemplateIndexArray& ta,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            IndexArray(ta,copyop),
            MixinVector<T>(ta) {}

        TemplateIndexArray(unsigned int no) :
            IndexArray(ARRAYTYPE,DataSize,DataType),
            MixinVector<T>(no) {}

        TemplateIndexArray(unsigned int no,T* ptr) :
            IndexArray(ARRAYTYPE,DataSize,DataType),
            MixinVector<T>(ptr,ptr+no) {}

        template <class InputIterator>
        TemplateIndexArray(InputIterator first,InputIterator last) :
            IndexArray(ARRAYTYPE,DataSize,DataType),
            MixinVector<T>(first,last) {}

        TemplateIndexArray& operator = (const TemplateIndexArray& array)
        {
            if (this==&array) return *this;
            this->assign(array.begin(),array.end());
            return *this;
        }

        virtual Object* cloneType() const { return new TemplateIndexArray(); }
        virtual Object* clone(const CopyOp& copyop) const { return new TemplateIndexArray(*this,copyop); }

        inline virtual void accept(ArrayVisitor& av);
        inline virtual void accept(ConstArrayVisitor& av) const;

        inline virtual void accept(unsigned int index,ValueVisitor& vv);
        inline virtual void accept(unsigned int index,ConstValueVisitor& vv) const;

        virtual int compare(unsigned int lhs,unsigned int rhs) const
        {
            const T& elem_lhs = (*this)[lhs];
            const T& elem_rhs = (*this)[rhs];
            if (elem_lhs<elem_rhs) return -1;
            if (elem_rhs<elem_lhs) return 1;
            return 0;
        }

        /** Frees unused space on this vector - i.e. the difference between size() and max_size() of the underlying vector.*/
        virtual void trim()
        {
            MixinVector<T>( *this ).swap( *this );
        }

        virtual unsigned int getElementSize() const { return sizeof(ElementDataType); }
        virtual const GLvoid*   getDataPointer() const { if (!this->empty()) return &this->front(); else return 0; }
        virtual const GLvoid*   getDataPointer(unsigned int index) const { if (!this->empty()) return &((*this)[index]); else return 0; }
        virtual unsigned int    getTotalDataSize() const { return static_cast<unsigned int>(this->size()*sizeof(T)); }
        virtual unsigned int    getNumElements() const { return static_cast<unsigned int>(this->size()); }
        virtual void reserveArray(unsigned int num) { this->reserve(num); }
        virtual void resizeArray(unsigned int num) { this->resize(num); }

        virtual unsigned int    index(unsigned int pos) const { return (*this)[pos]; }

        typedef T ElementDataType; // expose T

    protected:

        virtual ~TemplateIndexArray() {}
};

// The predefined array types

typedef TemplateIndexArray<GLbyte,Array::ByteArrayType,1,GL_BYTE>               ByteArray;
typedef TemplateIndexArray<GLshort,Array::ShortArrayType,1,GL_SHORT>            ShortArray;
typedef TemplateIndexArray<GLint,Array::IntArrayType,1,GL_INT>                  IntArray;

typedef TemplateIndexArray<GLubyte,Array::UByteArrayType,1,GL_UNSIGNED_BYTE>    UByteArray;
typedef TemplateIndexArray<GLushort,Array::UShortArrayType,1,GL_UNSIGNED_SHORT> UShortArray;
typedef TemplateIndexArray<GLuint,Array::UIntArrayType,1,GL_UNSIGNED_INT>       UIntArray;

typedef TemplateArray<GLfloat,Array::FloatArrayType,1,GL_FLOAT>                 FloatArray;
typedef TemplateArray<GLdouble,Array::DoubleArrayType,1,GL_DOUBLE>              DoubleArray;

typedef TemplateArray<Vec2b,Array::Vec2bArrayType,2,GL_BYTE>                    Vec2bArray;
typedef TemplateArray<Vec3b,Array::Vec3bArrayType,3,GL_BYTE>                    Vec3bArray;
typedef TemplateArray<Vec4b,Array::Vec4bArrayType,4,GL_BYTE>                    Vec4bArray;

typedef TemplateArray<Vec2s,Array::Vec2sArrayType,2,GL_SHORT>                   Vec2sArray;
typedef TemplateArray<Vec3s,Array::Vec3sArrayType,3,GL_SHORT>                   Vec3sArray;
typedef TemplateArray<Vec4s,Array::Vec4sArrayType,4,GL_SHORT>                   Vec4sArray;

typedef TemplateArray<Vec2i,Array::Vec2iArrayType,2,GL_INT>                     Vec2iArray;
typedef TemplateArray<Vec3i,Array::Vec3iArrayType,3,GL_INT>                     Vec3iArray;
typedef TemplateArray<Vec4i,Array::Vec4iArrayType,4,GL_INT>                     Vec4iArray;

typedef TemplateArray<Vec2ub,Array::Vec2ubArrayType,2,GL_UNSIGNED_BYTE>         Vec2ubArray;
typedef TemplateArray<Vec3ub,Array::Vec3ubArrayType,3,GL_UNSIGNED_BYTE>         Vec3ubArray;
typedef TemplateArray<Vec4ub,Array::Vec4ubArrayType,4,GL_UNSIGNED_BYTE>         Vec4ubArray;

typedef TemplateArray<Vec2us,Array::Vec2usArrayType,2,GL_UNSIGNED_SHORT>        Vec2usArray;
typedef TemplateArray<Vec3us,Array::Vec3usArrayType,3,GL_UNSIGNED_SHORT>        Vec3usArray;
typedef TemplateArray<Vec4us,Array::Vec4usArrayType,4,GL_UNSIGNED_SHORT>        Vec4usArray;

typedef TemplateArray<Vec2ui,Array::Vec2uiArrayType,2,GL_UNSIGNED_INT>          Vec2uiArray;
typedef TemplateArray<Vec3ui,Array::Vec3uiArrayType,3,GL_UNSIGNED_INT>          Vec3uiArray;
typedef TemplateArray<Vec4ui,Array::Vec4uiArrayType,4,GL_UNSIGNED_INT>          Vec4uiArray;

typedef TemplateArray<Vec2,Array::Vec2ArrayType,2,GL_FLOAT>                     Vec2Array;
typedef TemplateArray<Vec3,Array::Vec3ArrayType,3,GL_FLOAT>                     Vec3Array;
typedef TemplateArray<Vec4,Array::Vec4ArrayType,4,GL_FLOAT>                     Vec4Array;

typedef TemplateArray<Vec2d,Array::Vec2dArrayType,2,GL_DOUBLE>                  Vec2dArray;
typedef TemplateArray<Vec3d,Array::Vec3dArrayType,3,GL_DOUBLE>                  Vec3dArray;
typedef TemplateArray<Vec4d,Array::Vec4dArrayType,4,GL_DOUBLE>                  Vec4dArray;

typedef TemplateArray<Matrixf,Array::MatrixArrayType,16,GL_FLOAT>               MatrixfArray;
typedef TemplateArray<Matrixd,Array::MatrixdArrayType,16,GL_DOUBLE>             MatrixdArray;

typedef TemplateArray<Quat,Array::QuatArrayType,4,GL_DOUBLE>                    QuatArray;

typedef TemplateIndexArray<GLuint64,Array::UInt64ArrayType,1,GL_UNSIGNED_INT64_ARB> UInt64Array;
typedef TemplateIndexArray<GLint64,Array::Int64ArrayType,1,GL_INT64_ARB>            Int64Array;

class ArrayVisitor
{
    public:
        ArrayVisitor() {}
        virtual ~ArrayVisitor() {}

        virtual void apply(Array&) {}

        virtual void apply(ByteArray&) {}
        virtual void apply(ShortArray&) {}
        virtual void apply(IntArray&) {}

        virtual void apply(UByteArray&) {}
        virtual void apply(UShortArray&) {}
        virtual void apply(UIntArray&) {}

        virtual void apply(FloatArray&) {}
        virtual void apply(DoubleArray&) {}


        virtual void apply(Vec2bArray&) {}
        virtual void apply(Vec3bArray&) {}
        virtual void apply(Vec4bArray&) {}

        virtual void apply(Vec2sArray&) {}
        virtual void apply(Vec3sArray&) {}
        virtual void apply(Vec4sArray&) {}

        virtual void apply(Vec2iArray&) {}
        virtual void apply(Vec3iArray&) {}
        virtual void apply(Vec4iArray&) {}

        virtual void apply(Vec2ubArray&) {}
        virtual void apply(Vec3ubArray&) {}
        virtual void apply(Vec4ubArray&) {}

        virtual void apply(Vec2usArray&) {}
        virtual void apply(Vec3usArray&) {}
        virtual void apply(Vec4usArray&) {}

        virtual void apply(Vec2uiArray&) {}
        virtual void apply(Vec3uiArray&) {}
        virtual void apply(Vec4uiArray&) {}

        virtual void apply(Vec2Array&) {}
        virtual void apply(Vec3Array&) {}
        virtual void apply(Vec4Array&) {}

        virtual void apply(Vec2dArray&) {}
        virtual void apply(Vec3dArray&) {}
        virtual void apply(Vec4dArray&) {}

        virtual void apply(MatrixfArray&) {}
        virtual void apply(MatrixdArray&) {}

        virtual void apply(UInt64Array&) {}
        virtual void apply(Int64Array&)  {}
};

class ConstArrayVisitor
{
    public:
        ConstArrayVisitor() {}
        virtual ~ConstArrayVisitor() {}

        virtual void apply(const Array&) {}

        virtual void apply(const ByteArray&) {}
        virtual void apply(const ShortArray&) {}
        virtual void apply(const IntArray&) {}

        virtual void apply(const UByteArray&) {}
        virtual void apply(const UShortArray&) {}
        virtual void apply(const UIntArray&) {}

        virtual void apply(const FloatArray&) {}
        virtual void apply(const DoubleArray&) {}


        virtual void apply(const Vec2bArray&) {}
        virtual void apply(const Vec3bArray&) {}
        virtual void apply(const Vec4bArray&) {}

        virtual void apply(const Vec2sArray&) {}
        virtual void apply(const Vec3sArray&) {}
        virtual void apply(const Vec4sArray&) {}

        virtual void apply(const Vec2iArray&) {}
        virtual void apply(const Vec3iArray&) {}
        virtual void apply(const Vec4iArray&) {}

        virtual void apply(const Vec2ubArray&) {}
        virtual void apply(const Vec3ubArray&) {}
        virtual void apply(const Vec4ubArray&) {}

        virtual void apply(const Vec2usArray&) {}
        virtual void apply(const Vec3usArray&) {}
        virtual void apply(const Vec4usArray&) {}

        virtual void apply(const Vec2uiArray&) {}
        virtual void apply(const Vec3uiArray&) {}
        virtual void apply(const Vec4uiArray&) {}

        virtual void apply(const Vec2Array&) {}
        virtual void apply(const Vec3Array&) {}
        virtual void apply(const Vec4Array&) {}

        virtual void apply(const Vec2dArray&) {}
        virtual void apply(const Vec3dArray&) {}
        virtual void apply(const Vec4dArray&) {}

        virtual void apply(const MatrixfArray&) {}
        virtual void apply(const MatrixdArray&) {}

        virtual void apply(const UInt64Array&) {}
        virtual void apply(const Int64Array&)  {}
};


class ValueVisitor
{
    public:
        ValueVisitor() {}
        virtual ~ValueVisitor() {}

        virtual void apply(GLbyte&) {}
        virtual void apply(GLshort&) {}
        virtual void apply(GLint&) {}

        virtual void apply(GLushort&) {}
        virtual void apply(GLubyte&) {}
        virtual void apply(GLuint&) {}

        virtual void apply(GLfloat&) {}
        virtual void apply(GLdouble&) {}


        virtual void apply(Vec2b&) {}
        virtual void apply(Vec3b&) {}
        virtual void apply(Vec4b&) {}

        virtual void apply(Vec2s&) {}
        virtual void apply(Vec3s&) {}
        virtual void apply(Vec4s&) {}

        virtual void apply(Vec2i&) {}
        virtual void apply(Vec3i&) {}
        virtual void apply(Vec4i&) {}

        virtual void apply(Vec2ub&) {}
        virtual void apply(Vec3ub&) {}
        virtual void apply(Vec4ub&) {}

        virtual void apply(Vec2us&) {}
        virtual void apply(Vec3us&) {}
        virtual void apply(Vec4us&) {}

        virtual void apply(Vec2ui&) {}
        virtual void apply(Vec3ui&) {}
        virtual void apply(Vec4ui&) {}

        virtual void apply(Vec2&) {}
        virtual void apply(Vec3&) {}
        virtual void apply(Vec4&) {}

        virtual void apply(Vec2d&) {}
        virtual void apply(Vec3d&) {}
        virtual void apply(Vec4d&) {}

        virtual void apply(Matrixf&) {}
        virtual void apply(Matrixd&) {}

        virtual void apply(Quat&) {}

        virtual void apply(GLuint64&){}
        virtual void apply(GLint64&){}
};

class ConstValueVisitor
{
    public:
        ConstValueVisitor() {}
        virtual ~ConstValueVisitor() {}

        virtual void apply(const GLbyte&) {}
        virtual void apply(const GLshort&) {}
        virtual void apply(const GLint&) {}

        virtual void apply(const GLushort&) {}
        virtual void apply(const GLubyte&) {}
        virtual void apply(const GLuint&) {}

        virtual void apply(const GLfloat&) {}
        virtual void apply(const GLdouble&) {}


        virtual void apply(const Vec2b&) {}
        virtual void apply(const Vec3b&) {}
        virtual void apply(const Vec4b&) {}

        virtual void apply(const Vec2s&) {}
        virtual void apply(const Vec3s&) {}
        virtual void apply(const Vec4s&) {}

        virtual void apply(const Vec2i&) {}
        virtual void apply(const Vec3i&) {}
        virtual void apply(const Vec4i&) {}

        virtual void apply(const Vec2ub&) {}
        virtual void apply(const Vec3ub&) {}
        virtual void apply(const Vec4ub&) {}

        virtual void apply(const Vec2us&) {}
        virtual void apply(const Vec3us&) {}
        virtual void apply(const Vec4us&) {}

        virtual void apply(const Vec2ui&) {}
        virtual void apply(const Vec3ui&) {}
        virtual void apply(const Vec4ui&) {}

        virtual void apply(const Vec2&) {}
        virtual void apply(const Vec3&) {}
        virtual void apply(const Vec4&) {}

        virtual void apply(const Vec2d&) {}
        virtual void apply(const Vec3d&) {}
        virtual void apply(const Vec4d&) {}

        virtual void apply(const Matrixf&) {}
        virtual void apply(const Matrixd&) {}

        virtual void apply(const Quat&) {}

        virtual void apply(const GLuint64&){}
        virtual void apply(const GLint64&){}
};

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateArray<T,ARRAYTYPE,DataSize,DataType>::accept(ArrayVisitor& av) { av.apply(*this); }

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateArray<T,ARRAYTYPE,DataSize,DataType>::accept(ConstArrayVisitor& av) const { av.apply(*this); }

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateArray<T,ARRAYTYPE,DataSize,DataType>::accept(unsigned int index,ValueVisitor& vv) { vv.apply( (*this)[index] ); }

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateArray<T,ARRAYTYPE,DataSize,DataType>::accept(unsigned int index,ConstValueVisitor& vv) const {  vv.apply( (*this)[index] );}

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateIndexArray<T,ARRAYTYPE,DataSize,DataType>::accept(ArrayVisitor& av) { av.apply(*this); }

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateIndexArray<T,ARRAYTYPE,DataSize,DataType>::accept(ConstArrayVisitor& av) const { av.apply(*this); }

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateIndexArray<T,ARRAYTYPE,DataSize,DataType>::accept(unsigned int index,ValueVisitor& vv) { vv.apply( (*this)[index] ); }

template<typename T, Array::Type ARRAYTYPE, int DataSize, int DataType>
inline void TemplateIndexArray<T,ARRAYTYPE,DataSize,DataType>::accept(unsigned int index,ConstValueVisitor& vv) const {  vv.apply( (*this)[index] );}

}

#endif
