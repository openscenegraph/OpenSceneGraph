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

#define VAS_NOTICE OSG_INFO
//#define VAS_NOTICE OSG_NOTICE

#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
///////////////////////////////////////////////////////////////////////////////////
//
//  VertexArrayDispatch
//
struct VertexArrayDispatch : public VertexArrayState::ArrayDispatch
{
    VertexArrayDispatch() {}

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<vbo<<")"<<std::endl;
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo"<<vbo<<")"<<std::endl;
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void disable(osg::State& state)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::disable()"<<std::endl;
        glDisableClientState(GL_VERTEX_ARRAY);
    }
};


///////////////////////////////////////////////////////////////////////////////////
//
//  ColorArrayDispatch
//
struct ColorArrayDispatch : public VertexArrayState::ArrayDispatch
{
    ColorArrayDispatch() {}

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<vbo<<")"<<std::endl;

        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo="<<vbo<<")"<<std::endl;
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void disable(osg::State& state)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::disable()"<<std::endl;
        glDisableClientState(GL_COLOR_ARRAY);
    }

};
///////////////////////////////////////////////////////////////////////////////////
//
//  NormalArrayDispatch
//
struct NormalArrayDispatch : public VertexArrayState::ArrayDispatch
{
    NormalArrayDispatch() {}

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<vbo<<")"<<std::endl;
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glNormalPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo="<<vbo<<")"<<std::endl;
        glNormalPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void disable(osg::State& state)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::disable()"<<std::endl;
        glDisableClientState(GL_NORMAL_ARRAY);
    }
};
///////////////////////////////////////////////////////////////////////////////////
//
//  SecondaryColorArrayDispatch
//
struct SecondaryColorArrayDispatch : public VertexArrayState::ArrayDispatch
{
    SecondaryColorArrayDispatch() {}

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array)
    {
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void disable(osg::State& state)
    {
        glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    }
};


///////////////////////////////////////////////////////////////////////////////////
//
//  FogCoordArrayDispatch
//
struct FogCoordArrayDispatch : public VertexArrayState::ArrayDispatch
{
    FogCoordArrayDispatch() {}

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array)
    {
        glEnableClientState(GL_FOG_COORD_ARRAY);
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        glEnableClientState(GL_FOG_COORD_ARRAY);
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void disable(osg::State& state)
    {
        glDisableClientState(GL_FOG_COORD_ARRAY);
    }
};

///////////////////////////////////////////////////////////////////////////////////
//
//  TexCoordArrayDispatch
//
struct TexCoordArrayDispatch : public VertexArrayState::ArrayDispatch
{
    TexCoordArrayDispatch(unsigned int in_unit) : unit(in_unit) {}

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<") unit="<<unit<<std::endl;

        glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
        //state.setClientActiveTextureUnit(unit);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<vbo<<") unit="<<unit<<std::endl;

        //glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
        state.setClientActiveTextureUnit(unit);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::dispatch("<<new_array->getNumElements()<<") unit="<<unit<<std::endl;

        //glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
        state.setClientActiveTextureUnit(unit);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo="<<vbo<<") unit="<<unit<<std::endl;

        //glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
        state.setClientActiveTextureUnit(unit);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void disable(osg::State& state)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::disable() unit="<<unit<<std::endl;

        //state.glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
        state.setClientActiveTextureUnit(unit);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    unsigned int unit;
};
#endif

///////////////////////////////////////////////////////////////////////////////////
//
//  VertexAttribArrayDispatch
//

struct VertexAttribArrayDispatch : public VertexArrayState::ArrayDispatch
{
    VertexAttribArrayDispatch(unsigned int in_unit) : unit(in_unit) {}

    inline void callVertexAttribPointer(GLExtensions* ext, const osg::Array* new_array, const GLvoid * ptr)
    {
        if (new_array->getDataType()==GL_FLOAT)
            ext->glVertexAttribPointer(static_cast<GLuint>(unit), new_array->getDataSize(), new_array->getDataType(), new_array->getNormalize(), 0, ptr);
        else if (array->getDataType()==GL_DOUBLE)
            ext->glVertexAttribLPointer(static_cast<GLuint>(unit), new_array->getDataSize(), new_array->getDataType(), 0, ptr);
        else
            ext->glVertexAttribIPointer(static_cast<GLuint>(unit), new_array->getDataSize(), new_array->getDataType(), 0, ptr);
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array)
    {
        GLExtensions* ext = state.get<GLExtensions>();

        ext->glEnableVertexAttribArray( unit );
        callVertexAttribPointer(ext, new_array, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        GLExtensions* ext = state.get<GLExtensions>();

        ext->glEnableVertexAttribArray( unit );
        callVertexAttribPointer(ext, new_array, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        GLExtensions* ext = state.get<GLExtensions>();
        callVertexAttribPointer(ext, new_array, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        GLExtensions* ext = state.get<GLExtensions>();
        callVertexAttribPointer(ext, new_array, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void disable(osg::State& state)
    {
        GLExtensions* ext = state.get<GLExtensions>();
        ext->glDisableVertexAttribArray( unit );
    }

    unsigned int unit;
};


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

void VertexArrayState::generateVretexArrayObject()
{
    _ext->glGenVertexArrays(1, &_vertexArrayObject);
}

void VertexArrayState::bindVertexArrayObject() const
{
    VAS_NOTICE<<"glBindVertexArray() _vertexArrayObject="<<_vertexArrayObject<<std::endl;

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


void VertexArrayState::assignVertexArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        _vertexArray = new VertexArrayDispatch();
    }
    else
#endif
    {
        _vertexArray = new VertexAttribArrayDispatch(_ext->getVertexAlias()._location);
    }
}

void VertexArrayState::assignNormalArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        _normalArray = new NormalArrayDispatch();
    }
    else
#endif
    {
        _normalArray = new VertexAttribArrayDispatch(_ext->getNormalAlias()._location);
    }
}

void VertexArrayState::assignColorArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        _colorArray = new ColorArrayDispatch();
    }
    else
#endif
    {
        _colorArray = new VertexAttribArrayDispatch(_ext->getColorAlias()._location);
    }
}

void VertexArrayState::assignSecondaryColorArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        _secondaryColorArray = new SecondaryColorArrayDispatch();
    }
    else
#endif
    {
        _secondaryColorArray = new VertexAttribArrayDispatch(_ext->getSecondaryColorAlias()._location);
    }
}

void VertexArrayState::assignFogCoordArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        _fogCoordArray = new FogCoordArrayDispatch();
    }
    else
#endif
    {
        _fogCoordArray = new VertexAttribArrayDispatch(_ext->getFogCoordAlias()._location);
    }
}

void VertexArrayState::assignTexCoordArrayDispatcher(unsigned int numUnits)
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_ext->getUseVertexAttributeAliasing())
    {
        _texCoordArrays.clear();
        for(unsigned int i=0; i<numUnits; ++i)
        {
            _texCoordArrays.push_back( new TexCoordArrayDispatch(i) );
        }
    }
    else
#endif
    {
        _texCoordArrays.clear();
        for(unsigned int i=0; i<numUnits; ++i)
        {
            _texCoordArrays.push_back( new VertexAttribArrayDispatch(_ext->getTexCoordAliasList()[i]._location) );
        }
    }
}

void VertexArrayState::assignVertexAttribArrayDispatcher(unsigned int numUnits)
{
    _vertexAttribArrays.clear();
    for(unsigned int i=0; i<numUnits; ++i)
    {
        _vertexAttribArrays.push_back( new VertexAttribArrayDispatch(i) );
    }
}

void VertexArrayState::assignAllDispatchers()
{
    unsigned int numUnits = 8;
    unsigned int numVertexAttrib = 16;

    assignVertexArrayDispatcher();
    assignNormalArrayDispatcher();
    assignColorArrayDispatcher();
    assignSecondaryColorArrayDispatcher();
    assignFogCoordArrayDispatcher();
    assignTexCoordArrayDispatcher(numUnits);
    assignVertexAttribArrayDispatcher(numVertexAttrib);
}

void VertexArrayState::lazyDisablingOfVertexAttributes()
{
    VAS_NOTICE<<"  VertexArrayState<"<<this<<">::lazyDisablingOfVertexAttributes() _activeDispatchers.size()="<<_activeDispatchers.size()<<", _previous_activeDispatchers.size()="<<_previous_activeDispatchers.size()<<std::endl;

    _activeDispatchers.swap(_previous_activeDispatchers);
    _activeDispatchers.clear();

    for(ActiveDispatchers::iterator itr = _previous_activeDispatchers.begin();
        itr != _previous_activeDispatchers.end();
        ++itr)
    {
        ArrayDispatch* ad = (*itr);
        // ad->array = 0;
        ad->active = false;
    }
}

void VertexArrayState::applyDisablingOfVertexAttributes(osg::State& state)
{
    VAS_NOTICE<<"  VertexArrayState<"<<this<<">::applyDisablingOfVertexAttributes() _activeDispatchers.size()="<<_activeDispatchers.size()<<", _previous_activeDispatchers.size()="<<_previous_activeDispatchers.size()<<std::endl;

    for(ActiveDispatchers::iterator itr = _previous_activeDispatchers.begin();
        itr != _previous_activeDispatchers.end();
        ++itr)
    {
        ArrayDispatch* ad = (*itr);
        if (!ad->active)
        {
            ad->disable(state);
            ad->array = 0;
            ad->modifiedCount = 0xffffffff;
        }
    }
    _previous_activeDispatchers.clear();
}

void VertexArrayState::setArray(ArrayDispatch* vad, osg::State& state, const osg::Array* new_array)
{
    if (new_array)
    {
        VAS_NOTICE<<"  VertexArrayState<"<<this<<">::setArray() "<<typeid(*vad).name()<<" new_array="<<new_array<<", size()="<<new_array->getNumElements()<<std::endl;

        if (!vad->active)
        {
            vad->active = true;
            _activeDispatchers.push_back(vad);
        }

#define LAZY_UPDATE
#define CHECK_IF_UPDATE

#ifdef LAZY_UPDATE
        if (vad->array==0)
#endif
        {
            GLBufferObject* vbo = isVertexBufferObjectSupported() ? new_array->getOrCreateGLBufferObject(state.getContextID()) : 0;
            if (vbo)
            {
                bindVertexBufferObject(vbo);
                vad->enable_and_dispatch(state, new_array, vbo);
            }
            else
            {
                unbindVertexBufferObject();
                vad->enable_and_dispatch(state, new_array);
            }
        }
#ifdef LAZY_UPDATE
        else
#ifdef CHECK_IF_UPDATE
        if (new_array!=vad->array || new_array->getModifiedCount()!=vad->modifiedCount)
#endif
        {
            GLBufferObject* vbo = isVertexBufferObjectSupported() ? new_array->getOrCreateGLBufferObject(state.getContextID()) : 0;
            if (vbo)
            {
                bindVertexBufferObject(vbo);
                vad->dispatch(state, new_array, vbo);
            }
            else
            {
                unbindVertexBufferObject();
                vad->dispatch(state, new_array);
            }
        }
#ifdef CHECK_IF_UPDATE
        else
        {
            VAS_NOTICE<<"**************** No need to update *************************"<<std::endl;
            return;
        }
#endif
#endif

        vad->array = new_array;
        vad->modifiedCount = new_array->getModifiedCount();

    }
    else if (vad->array)
    {
        VAS_NOTICE<<"  VertexArrayState::setArray() need to disable "<<typeid(*vad).name()<<std::endl;

        disable(vad, state);
    }
}

void VertexArrayState::disableTexCoordArrayAboveAndIncluding(osg::State& state, unsigned int index)
{
    for(unsigned int i=index; i<_texCoordArrays.size(); ++i)
    {
        disable(_texCoordArrays[i].get(), state);
    }
}

void VertexArrayState::disableVertexAttribArrayAboveAndIncluding(osg::State& state, unsigned int index)
{
    for(unsigned int i=index; i<_vertexAttribArrays.size(); ++i)
    {
        disable(_vertexAttribArrays[i].get(), state);
    }
}
