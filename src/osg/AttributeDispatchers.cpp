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
#include <osg/AttributeDispatchers>
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

        virtual void assign(const GLvoid* array)
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


template<typename I, typename T>
class TemplateTargetAttributeDispatch : public AttributeDispatch
{
    public:

        typedef void (GL_APIENTRY * F) (I, const T*);

        TemplateTargetAttributeDispatch(I target, F functionPtr, unsigned int stride):
            _functionPtr(functionPtr), _target(target), _stride(stride), _array(0) {}

        virtual void assign(const GLvoid* array)
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


class AttributeDispatchMap
{
public:

    AttributeDispatchMap() {}

    template<typename T>
    void assign(Array::Type type, void (GL_APIENTRY *functionPtr) (const T*), unsigned int stride)
    {
        if ((unsigned int)type >= _attributeDispatchList.size()) _attributeDispatchList.resize(type+1);
        _attributeDispatchList[type] = functionPtr ? new TemplateAttributeDispatch<T>(functionPtr, stride) : 0;
    }

    template<typename I, typename T>
    void targetAssign(I target, Array::Type type, void (GL_APIENTRY *functionPtr) (I, const T*), unsigned int stride)
    {
        if ((unsigned int)type >= _attributeDispatchList.size()) _attributeDispatchList.resize(type+1);
        _attributeDispatchList[type] = functionPtr ? new TemplateTargetAttributeDispatch<I,T>(target, functionPtr, stride) : 0;
    }

    AttributeDispatch* dispatcher(const Array* array)
    {
        // OSG_NOTICE<<"dispatcher("<<array<<")"<<std::endl;

        if (!array) return 0;

        Array::Type type = array->getType();
        AttributeDispatch* dispatcher = 0;

        // OSG_NOTICE<<"    array->getType()="<<type<<std::endl;
        // OSG_NOTICE<<"    _attributeDispatchList.size()="<<_attributeDispatchList.size()<<std::endl;

        if ((unsigned int)type<_attributeDispatchList.size())
        {
            dispatcher = _attributeDispatchList[array->getType()].get();
        }

        if (dispatcher)
        {
            // OSG_NOTICE<<"   returning dispatcher="<<dispatcher<<std::endl;
            dispatcher->assign(array->getDataPointer());
            return dispatcher;
        }
        else
        {
            // OSG_NOTICE<<"   no dispatcher found"<<std::endl;
            return 0;
        }
    }

    typedef std::vector< ref_ptr<AttributeDispatch> >  AttributeDispatchList;
    AttributeDispatchList               _attributeDispatchList;
};

AttributeDispatchers::AttributeDispatchers():
    _initialized(false),
    _state(0),
    _normalDispatchers(0),
    _colorDispatchers(0),
    _secondaryColorDispatchers(0),
    _fogCoordDispatchers(0),
    _useVertexAttribAlias(false)
{

}

AttributeDispatchers::~AttributeDispatchers()
{
    delete _normalDispatchers;
    delete _colorDispatchers;
    delete _secondaryColorDispatchers;
    delete _fogCoordDispatchers;

    for(AttributeDispatchMapList::iterator itr = _vertexAttribDispatchers.begin();
        itr != _vertexAttribDispatchers.end();
        ++itr)
    {
        delete *itr;
    }
}

void AttributeDispatchers::setState(osg::State* state)
{
    _state = state;
}

void AttributeDispatchers::init()
{
    if (_initialized) return;

    _initialized = true;

    _normalDispatchers = new AttributeDispatchMap();
    _colorDispatchers = new AttributeDispatchMap();
    _secondaryColorDispatchers  = new AttributeDispatchMap();
    _fogCoordDispatchers = new AttributeDispatchMap();


#ifdef OSG_GL_VERTEX_FUNCS_AVAILABLE
    GLExtensions* extensions = _state->get<GLExtensions>();

    _normalDispatchers->assign<GLbyte>(Array::Vec3bArrayType, glNormal3bv, 3);
    _normalDispatchers->assign<GLshort>(Array::Vec3sArrayType, glNormal3sv, 3);
    _normalDispatchers->assign<GLfloat>(Array::Vec3ArrayType, glNormal3fv, 3);
    _normalDispatchers->assign<GLdouble>(Array::Vec3dArrayType, glNormal3dv, 3);

    _colorDispatchers->assign<GLubyte>(Array::Vec4ubArrayType, glColor4ubv, 4);
    _colorDispatchers->assign<GLfloat>(Array::Vec3ArrayType, glColor3fv, 3);
    _colorDispatchers->assign<GLfloat>(Array::Vec4ArrayType, glColor4fv, 4);
    _colorDispatchers->assign<GLdouble>(Array::Vec3dArrayType, glColor3dv, 3);
    _colorDispatchers->assign<GLdouble>(Array::Vec4dArrayType, glColor4dv, 4);

    _secondaryColorDispatchers->assign<GLfloat>(Array::Vec3ArrayType, extensions->glSecondaryColor3fv, 3);

    _fogCoordDispatchers->assign<GLfloat>(Array::FloatArrayType, extensions->glFogCoordfv, 1);
#endif

    // pre allocate.
    _activeDispatchList.resize(5);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  With inidices
//

AttributeDispatch* AttributeDispatchers::normalDispatcher(Array* array)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getNormalAlias()._location, array) :
           _normalDispatchers->dispatcher(array);
}

AttributeDispatch* AttributeDispatchers::colorDispatcher(Array* array)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getColorAlias()._location, array) :
           _colorDispatchers->dispatcher(array);
}

AttributeDispatch* AttributeDispatchers::secondaryColorDispatcher(Array* array)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getSecondaryColorAlias()._location, array) :
           _secondaryColorDispatchers->dispatcher(array);
}

AttributeDispatch* AttributeDispatchers::fogCoordDispatcher(Array* array)
{
    return _useVertexAttribAlias ?
           vertexAttribDispatcher(_state->getFogCoordAlias()._location, array) :
           _fogCoordDispatchers->dispatcher(array);
}

AttributeDispatch* AttributeDispatchers::vertexAttribDispatcher(unsigned int unit, Array* array)
{
    if (unit>=_vertexAttribDispatchers.size()) assignVertexAttribDispatchers(unit);
    return _vertexAttribDispatchers[unit]->dispatcher(array);
}

void AttributeDispatchers::assignVertexAttribDispatchers(unsigned int unit)
{
    GLExtensions* extensions = _state->get<GLExtensions>();

    for(unsigned int i=_vertexAttribDispatchers.size(); i<=unit; ++i)
    {
        _vertexAttribDispatchers.push_back(new AttributeDispatchMap());
        AttributeDispatchMap& vertexAttribDispatcher = *_vertexAttribDispatchers[i];
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::FloatArrayType, extensions->glVertexAttrib1fv, 1);
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::Vec2ArrayType, extensions->glVertexAttrib2fv, 2);
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::Vec3ArrayType, extensions->glVertexAttrib3fv, 3);
        vertexAttribDispatcher.targetAssign<GLuint, GLfloat>(i, Array::Vec4ArrayType, extensions->glVertexAttrib4fv, 4);
    }
}

void AttributeDispatchers::reset()
{
    if (!_initialized) init();

    _useVertexAttribAlias = false;

    _activeDispatchList.clear();
}

}
