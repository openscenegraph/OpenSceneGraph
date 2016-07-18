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

#include <osg/VertexArrayState>
#include <osg/State>

using namespace osg;

void VertexArrayState::ArrayDispatch::dispatchDeprecated(osg::State&, unsigned int)
{
    OSG_INFO<<typeid(*this).name()<<"::dispatchDeprecated(..) Not implementation yet"<<std::endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// VertexArrayState
//
VertexArrayState::VertexArrayState(osg::GLExtensions* ext):
    _ext(ext),
    _vertexArrayObject(0),
    _currentVBO(0),
    _currentEBO(0)
{
}

osg::GLBufferObject* VertexArrayState::getGLBufferObject(osg::Array* array)
{
    if (_ext->isBufferObjectSupported && array->getBufferObject())
    {
        return array->getBufferObject()->getOrCreateGLBufferObject(_ext->contextID);
    }
    else
    {
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
//  VertexArrayDispatch
//
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
struct VertexArrayDispatch : public VertexArrayState::ArrayDispatch
{
    VertexArrayDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int)
    {
        OSG_INFO<<"VertexArrayDispatch::dispatch() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(array->getDataSize(), array->getDataType(), 0, array->getDataPointer());

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"VertexArrayDispatch::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setVertexPointer(array);

        modifiedCount = array->getModifiedCount();
    }
};

struct VertexArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    VertexArrayWithVBODispatch(Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        OSG_NOTICE<<"VertexArrayWithVBODispatch::dispatch() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else

        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(array->getDataSize(), array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_NOTICE<<"VertexArrayWithVBODispatch::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setVertexPointer(array);

        modifiedCount = array->getModifiedCount();
    }

    GLBufferObject* vbo;
};

struct VertexVec2fDispatch : public VertexArrayState::ArrayDispatch
{
    VertexVec2fDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        // OSG_INFO<<"VertexVec2fDispatch::dispatch()"<<std::endl;
        glVertex2fv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};

struct VertexVec3fDispatch : public VertexArrayState::ArrayDispatch
{
    VertexVec3fDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        // OSG_INFO<<"VertexVec3fDispatch::dispatch()"<<std::endl;
        glVertex3fv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};

struct VertexVec4fDispatch : public VertexArrayState::ArrayDispatch
{
    VertexVec4fDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        // OSG_INFO<<"VertexVec4fDispatch::dispatch()"<<std::endl;
        glVertex4fv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};
#endif

void VertexArrayState::assignVertexArray(osg::Array* array)
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            GLBufferObject* vbo = getGLBufferObject(array);
            osg::ref_ptr<ArrayDispatch> dispatcher;
            if (vbo) dispatcher = new VertexArrayWithVBODispatch(array, vbo);
            else dispatcher = new VertexArrayDispatch(array);
            _dispatchArrays.push_back(dispatcher.get());
        }
        else
        {
            osg::ref_ptr<ArrayDispatch> dispatcher;
            switch(array->getType())
            {
                case(Array::Vec2ArrayType): dispatcher = new VertexVec2fDispatch(array); break;
                case(Array::Vec3ArrayType): dispatcher = new VertexVec3fDispatch(array); break;
                case(Array::Vec4ArrayType): dispatcher = new VertexVec4fDispatch(array); break;
                default: break; // unsupported type
            }

            if (dispatcher)
            {
                getArrayDispatchList(array->getBinding()).push_back(dispatcher);
            }
        }
    }
    else
#endif
    {
        assignVertexAttribArray(_ext->getVertexAlias()._location, array);
    }
}


///////////////////////////////////////////////////////////////////////////////////
//
//  ColorArrayDispatch
//
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
struct ColorArrayDispatch : public VertexArrayState::ArrayDispatch
{
    ColorArrayDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int)
    {
        OSG_INFO<<"ColorArrayDispatch::dispatch() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(array->getDataSize(), array->getDataType(), 0, array->getDataPointer());

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"ColorArrayDispatch::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setColorPointer(array);

        modifiedCount = array->getModifiedCount();
    }
};

struct ColorArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    ColorArrayWithVBODispatch(Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        // OSG_INFO<<"ColorArrayWithVBODispatch::dispatch()"<<std::endl;
#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(array->getDataSize(), array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"ColorArrayDispatchWitVBO::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setColorPointer(array);

        modifiedCount = array->getModifiedCount();
    }

    GLBufferObject* vbo;
};

struct ColorVec3fDispatch : public VertexArrayState::ArrayDispatch
{
    ColorVec3fDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        OSG_INFO<<"ColorVec3fDispatch::dispatch()"<<std::endl;
        glColor3fv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};

struct ColorVec4fDispatch : public VertexArrayState::ArrayDispatch
{
    ColorVec4fDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        OSG_INFO<<"ColorVec4fDispatch::dispatch()"<<std::endl;
        glColor4fv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};

struct ColorVec4ubDispatch : public VertexArrayState::ArrayDispatch
{
    ColorVec4ubDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        // OSG_INFO<<"ColorVec4ubDispatch::dispatch()"<<std::endl;
        glColor4ubv(static_cast<const GLubyte*>(array->getDataPointer(index)));
    }
};
#endif

void VertexArrayState::assignColorArray(osg::Array* array)
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            GLBufferObject* vbo = getGLBufferObject(array);
            osg::ref_ptr<ArrayDispatch> dispatcher;
            if (vbo) dispatcher = new ColorArrayWithVBODispatch(array, vbo);
            else dispatcher = new ColorArrayDispatch(array);
            _dispatchArrays.push_back(dispatcher.get());
        }
        else
        {
            osg::ref_ptr<ArrayDispatch> dispatcher;
            switch(array->getType())
            {
                case(Array::Vec3ArrayType): dispatcher = new ColorVec3fDispatch(array); break;
                case(Array::Vec4ArrayType): dispatcher = new ColorVec4fDispatch(array); break;
                case(Array::Vec4ubArrayType): dispatcher = new ColorVec4ubDispatch(array); break;
                default: break; // unsupported type
            }

            if (dispatcher)
            {
                getArrayDispatchList(array->getBinding()).push_back(dispatcher);
            }
        }
    }
    else
#endif
    {
        assignVertexAttribArray(_ext->getColorAlias()._location, array);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
//  NormalArrayDispatch
//
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
struct NormalArrayDispatch : public VertexArrayState::ArrayDispatch
{
    NormalArrayDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int)
    {
        OSG_INFO<<"NormalArrayDispatch::dispatch() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(array->getDataType(), 0, array->getDataPointer());

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"NormalArrayDispatch::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setNormalPointer(array);

        modifiedCount = array->getModifiedCount();
    }
};

struct NormalArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    NormalArrayWithVBODispatch(Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        OSG_INFO<<"NormalArrayWithVBODispatch::dispatch()"<<std::endl;

#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"NormalArrayDispatchWithVBO::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setNormalPointer(array);

        modifiedCount = array->getModifiedCount();
    }

    GLBufferObject* vbo;
};

struct NormalVec3fDispatch : public VertexArrayState::ArrayDispatch
{
    NormalVec3fDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        glNormal3fv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};
#endif

void VertexArrayState::assignNormalArray(osg::Array* array)
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            GLBufferObject* vbo = getGLBufferObject(array);
            osg::ref_ptr<ArrayDispatch> dispatcher;
            if (vbo) dispatcher = new NormalArrayWithVBODispatch(array, vbo);
            else dispatcher = new NormalArrayDispatch(array);
            _dispatchArrays.push_back(dispatcher.get());
        }
        else
        {
            osg::ref_ptr<ArrayDispatch> dispatcher;
            switch(array->getType())
            {
                case(Array::Vec3ArrayType): dispatcher = new NormalVec3fDispatch(array); break;
                default: break; // unsupported type
            }

            if (dispatcher)
            {
                getArrayDispatchList(array->getBinding()).push_back(dispatcher);
            }
        }
    }
    else
#endif
    {
        assignVertexAttribArray(_ext->getNormalAlias()._location, array);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
//  SecondaryColorArrayDispatch
//
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
struct SecondaryColorArrayDispatch : public VertexArrayState::ArrayDispatch
{
    SecondaryColorArrayDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        OSG_INFO<<"SecondaryColorArrayDispatch::dispatch() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        state.get<GLExtensions>()->glSecondaryColorPointer(array->getDataSize(), array->getDataType(), 0, (const GLvoid *)(array->getDataPointer()));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"SecondaryColorArrayDispatch::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setSecondaryColorPointer(array);

        modifiedCount = array->getModifiedCount();
    }
};

struct SecondaryColorArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    SecondaryColorArrayWithVBODispatch(Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        OSG_INFO<<"SecondaryColorArrayWithVBODispatch::dispatch()"<<std::endl;

#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        state.get<GLExtensions>()->glSecondaryColorPointer(array->getDataSize(), array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"SecondaryColorArrayDispatchWithVBO::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setSecondaryColorPointer(array);

        modifiedCount = array->getModifiedCount();
    }

    GLBufferObject* vbo;
};

struct SecondaryColor3fDispatch : public VertexArrayState::ArrayDispatch
{
    SecondaryColor3fDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glSecondaryColor3fv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};
#endif

void VertexArrayState::assignSecondaryColorArray(osg::Array* array)
{
    if (!_ext->isSecondaryColorSupported)
    {
        OSG_WARN<<"VertexArrayState::assignSecondaryColorArray() glSeconaryColor* not support by OpenGL driver."<<std::endl;
        return;
    }

#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            GLBufferObject* vbo = getGLBufferObject(array);
            osg::ref_ptr<ArrayDispatch> dispatcher;
            if (vbo) dispatcher = new SecondaryColorArrayWithVBODispatch(array, vbo);
            else dispatcher = new SecondaryColorArrayDispatch(array);
            _dispatchArrays.push_back(dispatcher.get());
        }
        else
        {
            osg::ref_ptr<ArrayDispatch> dispatcher;
            switch(array->getType())
            {
                case(Array::Vec3ArrayType): dispatcher = new SecondaryColor3fDispatch(array); break;
                default: break; // unsupported type
            }

            if (dispatcher)
            {
                getArrayDispatchList(array->getBinding()).push_back(dispatcher);
            }
        }
    }
    else
#endif
    {
        assignVertexAttribArray(_ext->getSecondaryColorAlias()._location, array);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
//  FogCoordArrayDispatch
//
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
struct FogCoordArrayDispatch : public VertexArrayState::ArrayDispatch
{
    FogCoordArrayDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        OSG_INFO<<"FogCoordArrayDispatch::dispatch() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        state.get<GLExtensions>()->glFogCoordPointer(array->getDataType(), 0, (const GLvoid *)(array->getDataPointer()));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"FogCoordArrayDispatch::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setFogCoordPointer(array);

        modifiedCount = array->getModifiedCount();
    }
};

struct FogCoordArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    FogCoordArrayWithVBODispatch(Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        OSG_INFO<<"FogCoordArrayWithVBODispatch::dispatch()"<<std::endl;

#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        state.get<GLExtensions>()->glFogCoordPointer(array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"FogCoordArrayDispatchWithVBO::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setFogCoordPointer(array);

        modifiedCount = array->getModifiedCount();
    }

    GLBufferObject* vbo;
};

struct FogCoordfDispatch : public VertexArrayState::ArrayDispatch
{
    FogCoordfDispatch(Array* in_array) : ArrayDispatch(in_array) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glFogCoordfv(static_cast<const GLfloat*>(array->getDataPointer(index)));
    }
};
#endif

void VertexArrayState::assignFogCoordArray(osg::Array* array)
{
    if (!_ext->isFogCoordSupported)
    {
        OSG_WARN<<"VertexArrayState::assignFogCoordArray() glFogCoord* not support by OpenGL driver."<<std::endl;
        return;
    }


#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            GLBufferObject* vbo = getGLBufferObject(array);
            osg::ref_ptr<ArrayDispatch> dispatcher;
            if (vbo) dispatcher = new FogCoordArrayWithVBODispatch(array, vbo);
            else dispatcher = new FogCoordArrayDispatch(array);
            _dispatchArrays.push_back(dispatcher.get());
        }
        else
        {
            osg::ref_ptr<ArrayDispatch> dispatcher;
            switch(array->getType())
            {
                case(Array::FloatArrayType): dispatcher = new FogCoordfDispatch(array); break;
                default: break; // unsupported type
            }

            if (dispatcher)
            {
                getArrayDispatchList(array->getBinding()).push_back(dispatcher);
            }
        }
    }
    else
#endif
    {
        assignVertexAttribArray(_ext->getFogCoordAlias()._location, array);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
//  TexCoordArrayDispatch
//
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
struct TexCoordArrayDispatch : public VertexArrayState::ArrayDispatch
{
    TexCoordArrayDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State&, unsigned int)
    {
        OSG_INFO<<"TexCoordArrayDispatch::dispatch() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(array->getDataSize(), array->getDataType(), 0, array->getDataPointer());

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"TexCoordArrayDispatch::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setTexCoordPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
};

struct TexCoordArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    TexCoordArrayWithVBODispatch(unsigned int in_unit, Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), unit(in_unit), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        OSG_INFO<<"TexCoordArrayWithVBODispatch::dispatch()"<<std::endl;

#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(array->getDataSize(), array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        OSG_INFO<<"TexCoordArrayDispatchWithVBO::dispatchDeprecated() "<<array->getNumElements()<<", "<<array->getDataPointer()<<std::endl;

        state.setTexCoordPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
    GLBufferObject* vbo;
};

struct TexCoordVec2fDispatch : public VertexArrayState::ArrayDispatch
{
    TexCoordVec2fDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        glMultiTexCoord2fv(static_cast<GLenum>(GL_TEXTURE0+unit), static_cast<const GLfloat*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct TexCoordVec3fDispatch : public VertexArrayState::ArrayDispatch
{
    TexCoordVec3fDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        glMultiTexCoord3fv(static_cast<GLenum>(GL_TEXTURE0+unit), static_cast<const GLfloat*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct TexCoordVec4fDispatch : public VertexArrayState::ArrayDispatch
{
    TexCoordVec4fDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State&, unsigned int index)
    {
        glMultiTexCoord4fv(static_cast<GLenum>(GL_TEXTURE0+unit), static_cast<const GLfloat*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};
#endif

void VertexArrayState::assignTexCoordArray(unsigned int unit, osg::Array* array)
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            GLBufferObject* vbo = getGLBufferObject(array);
            osg::ref_ptr<ArrayDispatch> dispatcher;
            if (vbo) dispatcher = new TexCoordArrayWithVBODispatch(unit, array, vbo);
            else dispatcher = new TexCoordArrayDispatch(unit, array);
            _dispatchArrays.push_back(dispatcher.get());
        }
        else
        {
            osg::ref_ptr<ArrayDispatch> dispatcher;
            switch(array->getType())
            {
                case(Array::Vec2ArrayType): dispatcher = new TexCoordVec2fDispatch(unit, array); break;
                case(Array::Vec3ArrayType): dispatcher = new TexCoordVec3fDispatch(unit, array); break;
                case(Array::Vec4ArrayType): dispatcher = new TexCoordVec4fDispatch(unit, array); break;
                default: break; // unsupported type
            }

            if (dispatcher)
            {
                getArrayDispatchList(array->getBinding()).push_back(dispatcher);
            }
        }
    }
    else
#endif
    {
        if (unit<_ext->getTexCoordAliasList().size())
        {
            assignVertexAttribArray(_ext->getTexCoordAliasList()[unit]._location, array);
        }
        // else no slot assigned...
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
//  VertexAttribArrayDispatch
//

struct VertexAttribArrayDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribArrayDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        GLExtensions* ext = state.get<GLExtensions>();

        ext->glEnableVertexAttribArray( unit );
        ext->glVertexAttribPointer(static_cast<GLuint>(unit), array->getDataSize(), array->getDataType(), array->getNormalize(), 0, array->getDataPointer());

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        state.setVertexAttribPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
};

struct VertexAttribArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribArrayWithVBODispatch(unsigned int in_unit, Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), unit(in_unit), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        GLExtensions* ext = state.get<GLExtensions>();
        ext->glEnableVertexAttribArray( unit );
        ext->glVertexAttribPointer(static_cast<GLuint>(unit), array->getDataSize(), array->getDataType(), array->getNormalize(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        state.setVertexAttribPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
    GLBufferObject* vbo;
};

struct VertexAttribLArrayDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribLArrayDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        GLExtensions* ext = state.get<GLExtensions>();

        ext->glEnableVertexAttribArray( unit );
        ext->glVertexAttribLPointer(static_cast<GLuint>(unit), array->getDataSize(), array->getDataType(), 0, array->getDataPointer());

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        state.setVertexAttribLPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
};

struct VertexAttribLArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribLArrayWithVBODispatch(unsigned int in_unit, Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), unit(in_unit), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        GLExtensions* ext = state.get<GLExtensions>();
        ext->glEnableVertexAttribArray( unit );
        ext->glVertexAttribLPointer(static_cast<GLuint>(unit), array->getDataSize(), array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        state.setVertexAttribLPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
    GLBufferObject* vbo;
};

struct VertexAttribIArrayDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribIArrayDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
        GLExtensions* ext = state.get<GLExtensions>();

        ext->glEnableVertexAttribArray( unit );
        ext->glVertexAttribIPointer(static_cast<GLuint>(unit), array->getDataSize(), array->getDataType(), 0, array->getDataPointer());

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        state.setVertexAttribIPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
};

struct VertexAttribIArrayWithVBODispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribIArrayWithVBODispatch(unsigned int in_unit, Array* in_array, GLBufferObject* in_vbo) : ArrayDispatch(in_array), unit(in_unit), vbo(in_vbo) {}

    virtual void dispatch(osg::State& state, unsigned int)
    {
#if 1
        state.getCurrentVertexArrayState()->bindVertexBufferObject(vbo);
#else
        if (vbo->isDirty()) vbo->compileBuffer();
        else vbo->bindBuffer();
#endif

        GLExtensions* ext = state.get<GLExtensions>();
        ext->glEnableVertexAttribArray( unit );
        ext->glVertexAttribIPointer(static_cast<GLuint>(unit), array->getDataSize(), array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(array->getBufferIndex())));

        modifiedCount = array->getModifiedCount();
    }

    virtual void dispatchDeprecated(osg::State& state, unsigned int)
    {
        state.setVertexAttribIPointer(unit, array);

        modifiedCount = array->getModifiedCount();
    }

    unsigned int unit;
    GLBufferObject* vbo;
};

struct VertexAttribFloatDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribFloatDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib1fv(static_cast<GLuint>(unit), static_cast<const GLfloat*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct VertexAttribVec2fDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribVec2fDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib2fv(static_cast<GLuint>(unit), static_cast<const GLfloat*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct VertexAttribVec3fDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribVec3fDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib3fv(static_cast<GLuint>(unit), static_cast<const GLfloat*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct VertexAttribVec4fDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribVec4fDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib4fv(static_cast<GLuint>(unit), static_cast<const GLfloat*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};


struct VertexAttribDoubleDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribDoubleDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib1dv(static_cast<GLuint>(unit), static_cast<const GLdouble*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct VertexAttribVec2dDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribVec2dDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib2dv(static_cast<GLuint>(unit), static_cast<const GLdouble*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct VertexAttribVec3dDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribVec3dDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib3dv(static_cast<GLuint>(unit), static_cast<const GLdouble*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};

struct VertexAttribVec4dDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribVec4dDispatch(unsigned int in_unit, Array* in_array) : ArrayDispatch(in_array), unit(in_unit) {}

    virtual void dispatch(osg::State& state, unsigned int index)
    {
        state.get<GLExtensions>()->glVertexAttrib4dv(static_cast<GLuint>(unit), static_cast<const GLdouble*>(array->getDataPointer(index)));
    }

    unsigned int unit;
};


void VertexArrayState::assignVertexAttribArray(unsigned int unit, osg::Array* array)
{
    if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
    {
        GLBufferObject* vbo = getGLBufferObject(array);
        osg::ref_ptr<ArrayDispatch> dispatcher;
        if (array->getDataType()==GL_FLOAT)
        {
            if (vbo) dispatcher = new VertexAttribArrayWithVBODispatch(unit, array, vbo);
            else dispatcher = new VertexAttribArrayDispatch(unit, array);
        }
        else if (array->getDataType()==GL_DOUBLE)
        {
            if (vbo) dispatcher = new VertexAttribLArrayWithVBODispatch(unit, array, vbo);
            else dispatcher = new VertexAttribLArrayDispatch(unit, array);
        }
        else
        {
            if (vbo) dispatcher = new VertexAttribIArrayWithVBODispatch(unit, array, vbo);
            else dispatcher = new VertexAttribIArrayDispatch(unit, array);
        }

        _dispatchArrays.push_back(dispatcher.get());
    }
    else
    {
        osg::ref_ptr<ArrayDispatch> dispatcher;
        switch(array->getType())
        {
            case(Array::FloatArrayType): dispatcher = new VertexAttribFloatDispatch(unit, array); break;
            case(Array::Vec2ArrayType): dispatcher = new VertexAttribVec2fDispatch(unit, array); break;
            case(Array::Vec3ArrayType): dispatcher = new VertexAttribVec3fDispatch(unit, array); break;
            case(Array::Vec4ArrayType): dispatcher = new VertexAttribVec4fDispatch(unit, array); break;
            case(Array::DoubleArrayType): dispatcher = new VertexAttribFloatDispatch(unit, array); break;
            case(Array::Vec2dArrayType): dispatcher = new VertexAttribVec2dDispatch(unit, array); break;
            case(Array::Vec3dArrayType): dispatcher = new VertexAttribVec3dDispatch(unit, array); break;
            case(Array::Vec4dArrayType): dispatcher = new VertexAttribVec4dDispatch(unit, array); break;
            default: break; // unsupported type
        }

        if (dispatcher)
        {
            getArrayDispatchList(array->getBinding()).push_back(dispatcher);
        }
    }
}

void VertexArrayState::generateVretexArrayObject()
{
    _ext->glGenVertexArrays(1, &_vertexArrayObject);
}

void VertexArrayState::bindVertexArrayObject() const
{
    _ext->glBindVertexArray (_vertexArrayObject);
}

void VertexArrayState::unbindVertexArrayObject() const
{
     _ext->glBindVertexArray (0);
}

void VertexArrayState::releaseGLObjects()
{
    if (_vertexArrayObject)
    {
        _ext->glDeleteVertexArrays(1, &_vertexArrayObject);
    }
}
