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
#include <osg/ArrayDispatchers>
#include <osg/State>
#include <osg/Drawable>

#include <osg/Notify>
#include <osg/io_utils>


namespace osg
{

#if defined(OSG_GLES1_AVAILABLE)
inline void GL_APIENTRY glColor4ubv(const GLubyte* c) { glColor4ub(c[0], c[1], c[2], c[3]); }
inline void GL_APIENTRY glColor3fv(const GLfloat* c) { glColor4f(c[0], c[1], c[2], 1.0f); }
inline void GL_APIENTRY glColor4fv(const GLfloat* c) { glColor4f(c[0], c[1], c[2], c[3]); }
inline void GL_APIENTRY glColor3dv(const GLdouble* c) { glColor4f(c[0], c[1], c[2], 1.0f); }
inline void GL_APIENTRY glColor4dv(const GLdouble* c) { glColor4f(c[0], c[1], c[2], c[3]); }

inline void GL_APIENTRY glNormal3bv(const GLbyte* n) { const float div = 1.0f/128.0f; glNormal3f(float(n[0])*div, float(n[1])*div, float(n[3])*div); }
inline void GL_APIENTRY glNormal3sv(const GLshort* n) { const float div = 1.0f/32768.0f; glNormal3f(float(n[0])*div, float(n[1])*div, float(n[3])*div); }
inline void GL_APIENTRY glNormal3fv(const GLfloat* n) { glNormal3f(n[0], n[1], n[3]); }
inline void GL_APIENTRY glNormal3dv(const GLdouble* n) { glNormal3f(n[0], n[1], n[3]); }
#endif

template<typename T>
class TemplateAttributeDispatch : public AttributeDispatch
{
    public:

        typedef void (GL_APIENTRY * F) (const T*);

        TemplateAttributeDispatch(F functionPtr, unsigned int stride):
            _functionPtr(functionPtr), _stride(stride), _array(0) {}

        virtual void assign(const GLvoid* array, const IndexArray*)
        {
            _array = reinterpret_cast<const T*>(array);
        }

        virtual void operator () (unsigned int pos)
        {
            _functionPtr(&(_array[pos*_stride]));
        }

        F               _functionPtr;
        unsigned int    _stride;
        const T*        _array;
};

template<typename T>
class TemplateAttributeWithIndicesDispatch : public AttributeDispatch
{
    public:

        typedef void (GL_APIENTRY * F) (const T*);

        TemplateAttributeWithIndicesDispatch(F functionPtr, unsigned int stride):
            _functionPtr(functionPtr), _stride(stride), _array(0), _indices(0) {}

        virtual void assign(const GLvoid* array, const IndexArray* indices)
        {
            _array = reinterpret_cast<const T*>(array);
            _indices = indices;
        }

        virtual void operator () (unsigned int pos)
        {
            _functionPtr(&(_array[_indices->index(pos) * _stride]));
        }

        F                       _functionPtr;
        unsigned int            _stride;
        const T*                _array;
        const IndexArray*       _indices;
};

template<typename T>
class TemplateBeginEndAttributeDispatch : public AttributeDispatch
{
    public:

        typedef void (GLBeginEndAdapter::*F) (const T*);

        TemplateBeginEndAttributeDispatch(GLBeginEndAdapter* glBeginEndAdapter, F functionPtr, unsigned int stride):
            _glBeginEndAdapter(glBeginEndAdapter),
            _functionPtr(functionPtr), _stride(stride), _array(0) {}

        virtual void assign(const GLvoid* array, const IndexArray*)
        {
            _array = reinterpret_cast<const T*>(array);
        }

        virtual void operator () (unsigned int pos)
        {
            (_glBeginEndAdapter->*_functionPtr)(&(_array[pos*_stride]));
        }

        GLBeginEndAdapter*      _glBeginEndAdapter;
        F                       _functionPtr;
        unsigned int            _stride;
        const T*                _array;
};

template<typename T>
class TemplateBeginEndAttributeWithIndicesDispatch : public AttributeDispatch
{
    public:

        typedef void (GLBeginEndAdapter::*F) (const T*);

        TemplateBeginEndAttributeWithIndicesDispatch(GLBeginEndAdapter* glBeginEndAdapter, F functionPtr, unsigned int stride):
            _glBeginEndAdapter(glBeginEndAdapter),
            _functionPtr(functionPtr), _stride(stride), _array(0), _indices(0) {}

        virtual void assign(const GLvoid* array, const IndexArray* indices)
        {
            _array = reinterpret_cast<const T*>(array);
            _indices = indices;
        }

        virtual void operator () (unsigned int pos)
        {
            (_glBeginEndAdapter->*_functionPtr)(&(_array[_indices->index(pos) * _stride]));
        }

        GLBeginEndAdapter*      _glBeginEndAdapter;
        F                       _functionPtr;
        unsigned int            _stride;
        const T*                _array;
        const IndexArray*       _indices;
};


template<typename I, typename T>
class TemplateTargetAttributeDispatch : public AttributeDispatch
{
    public:

        typedef void (GL_APIENTRY * F) (I, const T*);

        TemplateTargetAttributeDispatch(I target, F functionPtr, unsigned int stride):
            _functionPtr(functionPtr), _target(target), _stride(stride), _array(0) {}

        virtual void assign(const GLvoid* array, const IndexArray*)
        {
            _array = reinterpret_cast<const T*>(array);
        }

        virtual void operator () (unsigned int pos)
        {
            _functionPtr(_target, &(_array[pos * _stride]));
        }

        F                       _functionPtr;
        I                       _target;
        unsigned int            _stride;
        const T*                _array;
};

template<typename I, typename T>
class TemplateTargetAttributeWithIndicesDispatch : public AttributeDispatch
{
    public:

        typedef void (GL_APIENTRY * F) (I, const T*);

        TemplateTargetAttributeWithIndicesDispatch(I target, F functionPtr, unsigned int stride):
            _functionPtr(functionPtr), _target(target), _stride(stride), _array(0), _indices(0) {}

        virtual void assign(const GLvoid* array, const IndexArray* indices)
        {
            _array = reinterpret_cast<const T*>(array);
            _indices = indices;
        }

        virtual void operator () (unsigned int pos)
        {
            _functionPtr(_target, &(_array[_indices->index(pos) * _stride]));
        }

        F                       _functionPtr;
        I                       _target;
        unsigned int            _stride;
        const T*                _array;
        const IndexArray*       _indices;
};


template<typename I, typename T>
class TemplateBeginEndTargetAttributeDispatch : public AttributeDispatch
{
    public:

        typedef void (GLBeginEndAdapter::*F) (I, const T*);

        TemplateBeginEndTargetAttributeDispatch(GLBeginEndAdapter* glBeginEndAdapter, I target, F functionPtr, unsigned int stride):
            _glBeginEndAdapter(glBeginEndAdapter),
            _functionPtr(functionPtr), _target(target), _stride(stride), _array(0) {}

        virtual void assign(const GLvoid* array, const IndexArray*)
        {
            _array = reinterpret_cast<const T*>(array);
        }

        virtual void operator () (unsigned int pos)
        {
            (_glBeginEndAdapter->*_functionPtr)(_target, &(_array[pos * _stride]));
        }

        GLBeginEndAdapter*      _glBeginEndAdapter;
        F                       _functionPtr;
        I                       _target;
        unsigned int            _stride;
        const T*                _array;
};

template<typename I, typename T>
class TemplateBeginEndTargetAttributeWithIndicesDispatch : public AttributeDispatch
{
    public:

        typedef void (GLBeginEndAdapter::*F) (I, const T*);

        TemplateBeginEndTargetAttributeWithIndicesDispatch(GLBeginEndAdapter* glBeginEndAdapter, I target, F functionPtr, unsigned int stride):
            _glBeginEndAdapter(glBeginEndAdapter),
            _functionPtr(functionPtr), _target(target), _stride(stride), _array(0), _indices(0) {}

        virtual void assign(const GLvoid* array, const IndexArray* indices)
        {
            _array = reinterpret_cast<const T*>(array);
            _indices = indices;
        }

        virtual void operator () (unsigned int pos)
        {
            (_glBeginEndAdapter->*_functionPtr)(_target, &(_array[_indices->index(pos) * _stride]));
        }

        GLBeginEndAdapter*      _glBeginEndAdapter;
        F                       _functionPtr;
        I                       _target;
        unsigned int            _stride;
        const T*                _array;
        const IndexArray*       _indices;
};

class AttributeDispatchMap
{
public:

    AttributeDispatchMap(GLBeginEndAdapter* glBeginEndAdapter):
        _glBeginEndAdapter(glBeginEndAdapter) {}

    template<typename T>
    void assign(Array::Type type, void (GL_APIENTRY *functionPtr) (const T*), unsigned int stride)
    {
        if ((unsigned int)type >= _attributeDispatchList.size()) _attributeDispatchList.resize(type+1);
        _attributeDispatchList[type] = functionPtr ? new TemplateAttributeDispatch<T>(functionPtr, stride) : 0;

        if ((unsigned int)type >= _attributeDispatchWithIndicesList.size()) _attributeDispatchWithIndicesList.resize(type+1);
        _attributeDispatchWithIndicesList[type] = functionPtr ? new TemplateAttributeWithIndicesDispatch<T>(functionPtr, stride) : 0;
    }

    template<typename I, typename T>
    void targetAssign(I target, Array::Type type, void (GL_APIENTRY *functionPtr) (I, const T*), unsigned int stride)
    {
        if ((unsigned int)type >= _attributeDispatchList.size()) _attributeDispatchList.resize(type+1);
        _attributeDispatchList[type] = functionPtr ? new TemplateTargetAttributeDispatch<I,T>(target, functionPtr, stride) : 0;

        if ((unsigned int)type >= _attributeDispatchWithIndicesList.size()) _attributeDispatchWithIndicesList.resize(type+1);
        _attributeDispatchWithIndicesList[type] = functionPtr ? new TemplateTargetAttributeWithIndicesDispatch<I,T>(target, functionPtr, stride) : 0;
    }

    template<typename T>
    void assignGLBeginEnd(Array::Type type, void (GLBeginEndAdapter::*functionPtr) (const T*), unsigned int stride)
    {
        if ((unsigned int)type >= _glBeginEndAttributeDispatchList.size()) _glBeginEndAttributeDispatchList.resize(type+1);
        _glBeginEndAttributeDispatchList[type] = functionPtr ? new TemplateBeginEndAttributeDispatch<T>(_glBeginEndAdapter, functionPtr, stride) : 0;

        if ((unsigned int)type >= _glBeginEndAttributeDispatchWithIndicesList.size()) _glBeginEndAttributeDispatchWithIndicesList.resize(type+1);
        _glBeginEndAttributeDispatchWithIndicesList[type] = functionPtr ? new TemplateBeginEndAttributeWithIndicesDispatch<T>(_glBeginEndAdapter, functionPtr, stride) : 0;
    }

    template<typename I, typename T>
    void targetGLBeginEndAssign(I target, Array::Type type, void (GLBeginEndAdapter::*functionPtr) (I, const T*), unsigned int stride)
    {
        if ((unsigned int)type >= _glBeginEndAttributeDispatchList.size()) _glBeginEndAttributeDispatchList.resize(type+1);
        _glBeginEndAttributeDispatchList[type] = functionPtr ? new TemplateBeginEndTargetAttributeDispatch<I,T>(_glBeginEndAdapter, target, functionPtr, stride) : 0;

        if ((unsigned int)type >= _glBeginEndAttributeDispatchWithIndicesList.size()) _glBeginEndAttributeDispatchWithIndicesList.resize(type+1);
        _glBeginEndAttributeDispatchWithIndicesList[type] = functionPtr ? new TemplateBeginEndTargetAttributeWithIndicesDispatch<I,T>(_glBeginEndAdapter, target, functionPtr, stride) : 0;
    }


    AttributeDispatch* dispatcher(bool useGLBeginEndAdapter, const Array* array, const IndexArray* indices)
    {
        // OSG_NOTICE<<"dispatcher("<<useGLBeginEndAdapter<<", "<<array<<", "<<indices<<")"<<std::endl;

        if (!array) return 0;

        Array::Type type = array->getType();
        AttributeDispatch* dispatcher = 0;

        // OSG_NOTICE<<"    array->getType()="<<type<<std::endl;
        // OSG_NOTICE<<"    _glBeginEndAttributeDispatchList.size()="<<_glBeginEndAttributeDispatchList.size()<<std::endl;
        // OSG_NOTICE<<"    _glBeginEndAttributeDispatchWithIndicesList.size()="<<_glBeginEndAttributeDispatchWithIndicesList.size()<<std::endl;
        // OSG_NOTICE<<"    _attributeDispatchIndicesList.size()="<<_attributeDispatchList.size()<<std::endl;
        // OSG_NOTICE<<"    _attributeDispatchWithIndicesList.size()="<<_attributeDispatchWithIndicesList.size()<<std::endl;

        if (useGLBeginEndAdapter)
        {
            if (indices)
            {
                if ((unsigned int)type<_glBeginEndAttributeDispatchWithIndicesList.size())
                {
                    dispatcher = _glBeginEndAttributeDispatchWithIndicesList[array->getType()].get();
                }
            }
            else if ((unsigned int)type<_glBeginEndAttributeDispatchList.size())
            {
                dispatcher = _glBeginEndAttributeDispatchList[array->getType()].get();
            }
        }
        else
        {
            if (indices)
            {
                if ((unsigned int)type<_attributeDispatchWithIndicesList.size())
                {
                    dispatcher = _attributeDispatchWithIndicesList[array->getType()].get();
                }
            }
            else if ((unsigned int)type<_attributeDispatchList.size())
            {
                dispatcher = _attributeDispatchList[array->getType()].get();
            }
        }

        if (dispatcher)
        {
            // OSG_NOTICE<<"   returning dispatcher="<<dispatcher<<std::endl;
            dispatcher->assign(array->getDataPointer(), indices);
            return dispatcher;
        }
        else
        {
            // OSG_NOTICE<<"   no dispatcher found"<<std::endl;
            return 0;
        }
    }

    typedef std::vector< ref_ptr<AttributeDispatch> >  AttributeDispatchList;
    GLBeginEndAdapter*                  _glBeginEndAdapter;
    AttributeDispatchList               _attributeDispatchList;
    AttributeDispatchList               _attributeDispatchWithIndicesList;
    AttributeDispatchList               _glBeginEndAttributeDispatchList;
    AttributeDispatchList               _glBeginEndAttributeDispatchWithIndicesList;
};

ArrayDispatchers::ArrayDispatchers():
    _initialized(false),
    _state(0),
    _glBeginEndAdapter(0),
    _vertexDispatchers(0),
    _normalDispatchers(0),
    _colorDispatchers(0),
    _secondaryColorDispatchers(0),
    _fogCoordDispatchers(0),
    _useVertexAttribAlias(false),
    _useGLBeginEndAdapter(false)
{

}

ArrayDispatchers::~ArrayDispatchers()
{
    delete _vertexDispatchers;
    delete _normalDispatchers;
    delete _colorDispatchers;
    delete _secondaryColorDispatchers;
    delete _fogCoordDispatchers;

    for(AttributeDispatchMapList::iterator itr = _texCoordDispatchers.begin();
        itr != _texCoordDispatchers.end();
        ++itr)
    {
        delete *itr;
    }

    for(AttributeDispatchMapList::iterator itr = _vertexAttribDispatchers.begin();
        itr != _vertexAttribDispatchers.end();
        ++itr)
    {
        delete *itr;
    }
}

void ArrayDispatchers::setState(osg::State* state)
{
    _state = state;
    _glBeginEndAdapter = &(state->getGLBeginEndAdapter());
}

void ArrayDispatchers::init()
{
    if (_initialized) return;

    _initialized = true;

    _vertexDispatchers = new AttributeDispatchMap(&(_state->getGLBeginEndAdapter()));
    _normalDispatchers = new AttributeDispatchMap(&(_state->getGLBeginEndAdapter()));
    _colorDispatchers = new AttributeDispatchMap(&(_state->getGLBeginEndAdapter()));
    _secondaryColorDispatchers  = new AttributeDispatchMap(&(_state->getGLBeginEndAdapter()));
    _fogCoordDispatchers = new AttributeDispatchMap(&(_state->getGLBeginEndAdapter()));

    _glBeginEndAdapter = &(_state->getGLBeginEndAdapter());
    _useGLBeginEndAdapter = false;

    _vertexDispatchers->assignGLBeginEnd<GLfloat>(Array::Vec3ArrayType, &GLBeginEndAdapter::Vertex3fv, 3);
    _vertexDispatchers->assignGLBeginEnd<GLdouble>(Array::Vec3dArrayType, &GLBeginEndAdapter::Vertex3dv, 3);
    _normalDispatchers->assignGLBeginEnd<GLfloat>(Array::Vec3ArrayType, &GLBeginEndAdapter::Normal3fv, 3);
    _colorDispatchers->assignGLBeginEnd<GLubyte>(Array::Vec4ubArrayType, &GLBeginEndAdapter::Color4ubv, 4);
    _colorDispatchers->assignGLBeginEnd<GLfloat>(Array::Vec3ArrayType, &GLBeginEndAdapter::Color3fv, 3);
    _colorDispatchers->assignGLBeginEnd<GLfloat>(Array::Vec4ArrayType, &GLBeginEndAdapter::Color4fv, 4);

#ifdef OSG_GL_VERTEX_FUNCS_AVAILABLE
    Drawable::Extensions* extensions = Drawable::getExtensions(_state->getContextID(),true);

    #ifndef OSG_GLES1_AVAILABLE
        _vertexDispatchers->assign<GLfloat>(Array::Vec2ArrayType, glVertex2fv, 2);
        _vertexDispatchers->assign<GLfloat>(Array::Vec3ArrayType, glVertex3fv, 3);
        _vertexDispatchers->assign<GLdouble>(Array::Vec2dArrayType, glVertex2dv, 2);
        _vertexDispatchers->assign<GLdouble>(Array::Vec3dArrayType, glVertex3dv, 3);
    #endif

    _normalDispatchers->assign<GLbyte>(Array::Vec3bArrayType, glNormal3bv, 3);
    _normalDispatchers->assign<GLshort>(Array::Vec3sArrayType, glNormal3sv, 3);
    _normalDispatchers->assign<GLfloat>(Array::Vec3ArrayType, glNormal3fv, 3);
    _normalDispatchers->assign<GLdouble>(Array::Vec3dArrayType, glNormal3dv, 3);

    _colorDispatchers->assign<GLubyte>(Array::Vec4ubArrayType, glColor4ubv, 4);
    _colorDispatchers->assign<GLfloat>(Array::Vec3ArrayType, glColor3fv, 3);
    _colorDispatchers->assign<GLfloat>(Array::Vec4ArrayType, glColor4fv, 4);
    _colorDispatchers->assign<GLdouble>(Array::Vec3dArrayType, glColor3dv, 3);
    _colorDispatchers->assign<GLdouble>(Array::Vec4dArrayType, glColor4dv, 4);

    _secondaryColorDispatchers->assign<GLfloat>(Array::Vec3ArrayType, extensions->_glSecondaryColor3fv, 3);

    _fogCoordDispatchers->assign<GLfloat>(Array::FloatArrayType, extensions->_glFogCoordfv, 1);
#endif

    // pre allocate.
    _activeDispatchList.resize(5);
}

AttributeDispatch* ArrayDispatchers::vertexDispatcher(Array* array, IndexArray* indices)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getVertexAlias()._location, array, indices) :
           _vertexDispatchers->dispatcher(_useGLBeginEndAdapter, array, indices);
}

AttributeDispatch* ArrayDispatchers::normalDispatcher(Array* array, IndexArray* indices)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getNormalAlias()._location, array, indices) :
           _normalDispatchers->dispatcher(_useGLBeginEndAdapter, array, indices);
}

AttributeDispatch* ArrayDispatchers::colorDispatcher(Array* array, IndexArray* indices)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getColorAlias()._location, array, indices) :
           _colorDispatchers->dispatcher(_useGLBeginEndAdapter, array, indices);
}

AttributeDispatch* ArrayDispatchers::secondaryColorDispatcher(Array* array, IndexArray* indices)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getSecondaryColorAlias()._location, array, indices) :
           _secondaryColorDispatchers->dispatcher(_useGLBeginEndAdapter, array, indices);
}

AttributeDispatch* ArrayDispatchers::fogCoordDispatcher(Array* array, IndexArray* indices)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getFogCoordAlias()._location, array, indices) :
           _fogCoordDispatchers->dispatcher(_useGLBeginEndAdapter, array, indices);
}

AttributeDispatch* ArrayDispatchers::texCoordDispatcher(unsigned int unit, Array* array, IndexArray* indices)
{
    if (_useVertexAttribAlias) return vertexAttribDispatcher(_state->getTexCoordAliasList()[unit]._location, array, indices);

    if (unit>=_texCoordDispatchers.size()) assignTexCoordDispatchers(unit);
    return _texCoordDispatchers[unit]->dispatcher(_useGLBeginEndAdapter, array, indices);
}

AttributeDispatch* ArrayDispatchers::vertexAttribDispatcher(unsigned int unit, Array* array, IndexArray* indices)
{
    if (unit>=_vertexAttribDispatchers.size()) assignVertexAttribDispatchers(unit);
    return _vertexAttribDispatchers[unit]->dispatcher(_useGLBeginEndAdapter, array, indices);
}

void ArrayDispatchers::assignTexCoordDispatchers(unsigned int unit)
{
    #if defined(OSG_GL_VERTEX_FUNCS_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
    Drawable::Extensions* extensions = Drawable::getExtensions(_state->getContextID(),true);
    #endif

    for(unsigned int i=_texCoordDispatchers.size(); i<=unit; ++i)
    {
        _texCoordDispatchers.push_back(new AttributeDispatchMap(_glBeginEndAdapter));
        AttributeDispatchMap& texCoordDispatcher = *_texCoordDispatchers[i];
        if (i==0)
        {
            #if defined(OSG_GL_VERTEX_FUNCS_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
            texCoordDispatcher.assign<GLfloat>(Array::FloatArrayType, glTexCoord1fv, 1);
            texCoordDispatcher.assign<GLfloat>(Array::Vec2ArrayType, glTexCoord2fv, 2);
            texCoordDispatcher.assign<GLfloat>(Array::Vec3ArrayType, glTexCoord3fv, 3);
            texCoordDispatcher.assign<GLfloat>(Array::Vec4ArrayType, glTexCoord4fv, 4);
            #endif
            texCoordDispatcher.assignGLBeginEnd<GLfloat>(Array::FloatArrayType, &GLBeginEndAdapter::TexCoord1fv, 1);
            texCoordDispatcher.assignGLBeginEnd<GLfloat>(Array::Vec2ArrayType, &GLBeginEndAdapter::TexCoord2fv, 2);
            texCoordDispatcher.assignGLBeginEnd<GLfloat>(Array::Vec3ArrayType, &GLBeginEndAdapter::TexCoord3fv, 3);
            texCoordDispatcher.assignGLBeginEnd<GLfloat>(Array::Vec4ArrayType, &GLBeginEndAdapter::TexCoord4fv, 4);
        }
        else
        {
            #if defined(OSG_GL_VERTEX_FUNCS_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
            texCoordDispatcher.targetAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::FloatArrayType, extensions->_glMultiTexCoord1fv, 1);
            texCoordDispatcher.targetAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::Vec2ArrayType, extensions->_glMultiTexCoord2fv, 2);
            texCoordDispatcher.targetAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::Vec3ArrayType, extensions->_glMultiTexCoord3fv, 3);
            texCoordDispatcher.targetAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::Vec4ArrayType, extensions->_glMultiTexCoord4fv, 4);
            #endif
            texCoordDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::FloatArrayType, &GLBeginEndAdapter::MultiTexCoord1fv, 1);
            texCoordDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::Vec2ArrayType, &GLBeginEndAdapter::MultiTexCoord2fv, 2);
            texCoordDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::Vec3ArrayType, &GLBeginEndAdapter::MultiTexCoord3fv, 3);
            texCoordDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>((GLenum)(GL_TEXTURE0+i), Array::Vec4ArrayType, &GLBeginEndAdapter::MultiTexCoord4fv, 4);
        }
    }

}

void ArrayDispatchers::assignVertexAttribDispatchers(unsigned int unit)
{
    Drawable::Extensions* extensions = Drawable::getExtensions(_state->getContextID(),true);

    for(unsigned int i=_vertexAttribDispatchers.size(); i<=unit; ++i)
    {
        _vertexAttribDispatchers.push_back(new AttributeDispatchMap(_glBeginEndAdapter));
        AttributeDispatchMap& vertexAttribDispatcher = *_vertexAttribDispatchers[i];
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::FloatArrayType, extensions->_glVertexAttrib1fv, 1);
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::Vec2ArrayType, extensions->_glVertexAttrib2fv, 2);
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::Vec3ArrayType, extensions->_glVertexAttrib3fv, 3);
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::Vec4ArrayType, extensions->_glVertexAttrib4fv, 4);
        vertexAttribDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>(i, Array::FloatArrayType, &GLBeginEndAdapter::VertexAttrib1fv, 1);
        vertexAttribDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>(i, Array::Vec2ArrayType, &GLBeginEndAdapter::VertexAttrib2fv, 2);
        vertexAttribDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>(i, Array::Vec3ArrayType, &GLBeginEndAdapter::VertexAttrib3fv, 3);
        vertexAttribDispatcher.targetGLBeginEndAssign<GLenum, GLfloat>(i, Array::Vec4ArrayType, &GLBeginEndAdapter::VertexAttrib4fv, 4);
    }
}

void ArrayDispatchers::reset()
{
    if (!_initialized) init();

    _useVertexAttribAlias = false;
    
    _useGLBeginEndAdapter = false;    
    _glBeginEndAdapter->reset();

    for(ActiveDispatchList::iterator itr = _activeDispatchList.begin();
        itr != _activeDispatchList.end();
        ++itr)
    {
        (*itr).clear();
    }
}

}
