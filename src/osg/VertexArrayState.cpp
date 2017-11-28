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
#include <osg/ContextData>

using namespace osg;

#if 1
#define VAS_NOTICE OSG_DEBUG
#else
#define VAS_NOTICE OSG_NOTICE
#endif


VertexArrayState::VertexArrayState(){}
VertexArrayState::~VertexArrayState(){}
VertexArrayState::VertexArrayState(const VertexArrayState& val, const osg::CopyOp& copyop) :
Object(val, copyop){}

class PerContextVertexArrayStateManager : public GraphicsObjectManager
{
public:
    PerContextVertexArrayStateManager(unsigned int contextID):
        GraphicsObjectManager("PerContextVertexArrayStateManager", contextID)
    {
    }

    virtual void flushDeletedGLObjects(double, double& availableTime)
    {
        // if no time available don't try to flush objects.
        if (availableTime<=0.0) return;

        VAS_NOTICE<<"PerContextVertexArrayStateManager::flushDeletedGLObjects()"<<std::endl;

        const osg::Timer& timer = *osg::Timer::instance();
        osg::Timer_t start_tick = timer.tick();
        double elapsedTime = 0.0;

        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_PerContextVertexArrayStateList);

            // trim from front
            PerContextVertexArrayStateList::iterator ditr=_PerContextVertexArrayStateList.begin();
            for(;
                ditr!=_PerContextVertexArrayStateList.end() && elapsedTime<availableTime;
                ++ditr)
            {
                PerContextVertexArrayState* vas = ditr->get();
                vas->deleteVertexArrayObject();

                elapsedTime = timer.delta_s(start_tick,timer.tick());
            }

            if (ditr!=_PerContextVertexArrayStateList.begin()) _PerContextVertexArrayStateList.erase(_PerContextVertexArrayStateList.begin(),ditr);

        }

        elapsedTime = timer.delta_s(start_tick,timer.tick());

        availableTime -= elapsedTime;
    }

    virtual void flushAllDeletedGLObjects()
    {
        VAS_NOTICE<<"PerContextVertexArrayStateManager::flushAllDeletedGLObjects()"<<std::endl;

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_PerContextVertexArrayStateList);
        for(PerContextVertexArrayStateList::iterator itr = _PerContextVertexArrayStateList.begin();
            itr != _PerContextVertexArrayStateList.end();
            ++itr)
        {
            PerContextVertexArrayState* vas = itr->get();
            vas->deleteVertexArrayObject();
        }
        _PerContextVertexArrayStateList.clear();
    }

    virtual void deleteAllGLObjects()
    {
         OSG_INFO<<"PerContextVertexArrayStateManager::deleteAllGLObjects() Not currently implementated"<<std::endl;
    }

    virtual void discardAllGLObjects()
    {
        VAS_NOTICE<<"PerContextVertexArrayStateManager::flushAllDeletedGLObjects()"<<std::endl;

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_PerContextVertexArrayStateList);
        _PerContextVertexArrayStateList.clear();
    }

    void release(PerContextVertexArrayState* vas)
    {
        VAS_NOTICE<<"PerContextVertexArrayStateManager::release("<<this<<")"<<std::endl;

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_PerContextVertexArrayStateList);
        _PerContextVertexArrayStateList.push_back(vas);
    }

protected:

    typedef std::list< osg::ref_ptr<PerContextVertexArrayState> > PerContextVertexArrayStateList;
    OpenThreads::Mutex _mutex_PerContextVertexArrayStateList;
    PerContextVertexArrayStateList _PerContextVertexArrayStateList;
};

#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
///////////////////////////////////////////////////////////////////////////////////
//
//  VertexArrayDispatch
//
struct VertexArrayDispatch : public PerContextVertexArrayState::ArrayDispatch
{
    VertexArrayDispatch(GLint & basevertex):
                        PerContextVertexArrayState::ArrayDispatch(basevertex) {}

    virtual const char* className() const { return "VertexArrayDispatch"; }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        OSG_WARN<<"    VertexArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<std::hex<<vbo<<std::dec<<")"<<vbo->getOffset(new_array->getBufferIndex())<<" "<<basevertex*new_array->getElementSize()<<std::endl;
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void enable_and_dispatch(osg::State& /*state*/, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(size, type, stride, ptr);
    }

    virtual void dispatch(osg::State& /*state*/, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& /*state*/, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo"<<std::hex<<vbo<<std::dec<<")"<<std::endl;
        glVertexPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& /*state*/, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        glVertexPointer(size, type, stride, ptr);
    }

    virtual void disable(osg::State& /*state*/)
    {
        VAS_NOTICE<<"    VertexArrayDispatch::disable()"<<std::endl;
        glDisableClientState(GL_VERTEX_ARRAY);
    }
};


///////////////////////////////////////////////////////////////////////////////////
//
//  ColorArrayDispatch
//
struct ColorArrayDispatch : public PerContextVertexArrayState::ArrayDispatch
{
    ColorArrayDispatch(GLint & basevertex):
        PerContextVertexArrayState::ArrayDispatch(basevertex) {}

    virtual const char* className() const { return "ColorArrayDispatch"; }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& /*state*/, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(size, type, stride, ptr);
    }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<std::hex<<std::hex<<vbo<<std::dec<<std::dec<<")"<<std::endl;

        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& /*state*/, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& /*state*/, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo="<<std::hex<<vbo<<std::dec<<")"<<std::endl;
        glColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& /*state*/, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        glColorPointer(size, type, stride, ptr);
    }

    virtual void disable(osg::State& /*state*/)
    {
        VAS_NOTICE<<"    ColorArrayDispatch::disable()"<<std::endl;
        glDisableClientState(GL_COLOR_ARRAY);
    }

};

///////////////////////////////////////////////////////////////////////////////////
//
//  NormalArrayDispatch
//
struct NormalArrayDispatch : public PerContextVertexArrayState::ArrayDispatch
{
    NormalArrayDispatch(GLint & basevertex):
        PerContextVertexArrayState::ArrayDispatch(basevertex) {}

    virtual const char* className() const { return "NormalArrayDispatch"; }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State&, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<std::hex<<vbo<<std::dec<<")"<<std::endl;
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())));
    }

    virtual void enable_and_dispatch(osg::State& /*state*/, GLint /*size*/, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(type, stride, ptr);
    }

    virtual void dispatch(osg::State& /*state*/, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::dispatch("<<new_array->getNumElements()<<")"<<std::endl;
        glNormalPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& /*state*/, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo="<<std::hex<<vbo<<std::dec<<")"<<std::endl;
        glNormalPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& /*state*/, GLint /*size*/, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        glNormalPointer(type, stride, ptr);
    }

    virtual void disable(osg::State& /*state*/)
    {
        VAS_NOTICE<<"    NormalArrayDispatch::disable()"<<std::endl;
        glDisableClientState(GL_NORMAL_ARRAY);
    }
};


#ifndef GL_SECONDARY_COLOR_ARRAY
    #ifdef GL_SECONDARY_COLOR_ARRAY_EXT
        #define GL_SECONDARY_COLOR_ARRAY GL_SECONDARY_COLOR_ARRAY_EXT
    #else
        #define GL_SECONDARY_COLOR_ARRAY 0x845E
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////////
//
//  SecondaryColorArrayDispatch
//
struct SecondaryColorArrayDispatch : public PerContextVertexArrayState::ArrayDispatch
{
    SecondaryColorArrayDispatch(GLint & basevertex):
        PerContextVertexArrayState::ArrayDispatch(basevertex) {}

    virtual const char* className() const { return "SecondaryColorArrayDispatch"; }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array)
    {
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        state.get<GLExtensions>()->glSecondaryColorPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void disable(osg::State& /*state*/)
    {
        glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    }
};


#ifndef GL_FOG_COORDINATE_ARRAY
    #ifdef GL_FOG_COORDINATE_ARRAY_EXT
        #define GL_FOG_COORDINATE_ARRAY GL_FOG_COORDINATE_ARRAY_EXT
    #else
        #define GL_FOG_COORDINATE_ARRAY 0x8457
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////////
//
//  FogCoordArrayDispatch
//
struct FogCoordArrayDispatch : public PerContextVertexArrayState::ArrayDispatch
{
    FogCoordArrayDispatch(GLint & basevertex):
        PerContextVertexArrayState::ArrayDispatch(basevertex) {}

    virtual const char* className() const { return "FogCoordArrayDispatch"; }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array)
    {
        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        state.get<GLExtensions>()->glFogCoordPointer(new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void disable(osg::State& /*state*/)
    {
        glDisableClientState(GL_FOG_COORDINATE_ARRAY);
    }
};

///////////////////////////////////////////////////////////////////////////////////
//
//  TexCoordArrayDispatch
//
struct TexCoordArrayDispatch : public PerContextVertexArrayState::ArrayDispatch
{
    TexCoordArrayDispatch(unsigned int in_unit,GLint & basevertex):
                          PerContextVertexArrayState::ArrayDispatch(basevertex) , unit(in_unit) {}

    virtual const char* className() const { return "TexCoordArrayDispatch"; }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<") unit="<<unit<<std::endl;

        state.setClientActiveTextureUnit(unit);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void enable_and_dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::enable_and_dispatch("<<new_array->getNumElements()<<", vbo="<<std::hex<<vbo<<std::dec<<") unit="<<unit<<std::endl;

        state.setClientActiveTextureUnit(unit);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void enable_and_dispatch(osg::State& state, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        state.setClientActiveTextureUnit(unit);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(size, type, stride, ptr);
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::dispatch("<<new_array->getNumElements()<<") unit="<<unit<<std::endl;

        state.setClientActiveTextureUnit(unit);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        VAS_NOTICE<<"    TexCoordArrayDispatch::dispatch("<<new_array->getNumElements()<<", vbo="<<std::hex<<vbo<<std::dec<<") unit="<<unit<<std::endl;

        state.setClientActiveTextureUnit(unit);
        glTexCoordPointer(new_array->getDataSize(), new_array->getDataType(), 0, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& state, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean /*normalized*/)
    {
        state.setClientActiveTextureUnit(unit);
        glTexCoordPointer(size, type, stride, ptr);
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

struct VertexAttribArrayDispatch : public PerContextVertexArrayState::ArrayDispatch
{
    VertexAttribArrayDispatch(unsigned int in_unit,GLint & basevertex):
                              PerContextVertexArrayState::ArrayDispatch(basevertex), unit(in_unit) {}

    virtual const char* className() const { return "VertexAttribArrayDispatch"; }

    inline void callVertexAttribPointer(GLExtensions* ext, const osg::Array* new_array, const GLvoid * ptr)
    {
        if (new_array->getPreserveDataType())
        {
            if (new_array->getDataType()==GL_FLOAT)
                ext->glVertexAttribPointer(static_cast<GLuint>(unit), new_array->getDataSize(), new_array->getDataType(), new_array->getNormalize(), 0, ptr);
            else if (new_array->getDataType()==GL_DOUBLE)
                ext->glVertexAttribLPointer(static_cast<GLuint>(unit), new_array->getDataSize(), new_array->getDataType(), 0, ptr);
            else
                ext->glVertexAttribIPointer(static_cast<GLuint>(unit), new_array->getDataSize(), new_array->getDataType(), 0, ptr);
        }
        else
        {
            ext->glVertexAttribPointer(static_cast<GLuint>(unit), new_array->getDataSize(), new_array->getDataType(), new_array->getNormalize(), 0, ptr);
        }
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
        callVertexAttribPointer(ext, new_array, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array)
    {
        GLExtensions* ext = state.get<GLExtensions>();
        callVertexAttribPointer(ext, new_array, new_array->getDataPointer());
    }

    virtual void dispatch(osg::State& state, const osg::Array* new_array, const osg::GLBufferObject* vbo)
    {
        GLExtensions* ext = state.get<GLExtensions>();
        callVertexAttribPointer(ext, new_array, (const GLvoid *)(vbo->getOffset(new_array->getBufferIndex())-basevertex*new_array->getElementSize()));
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
// PerContextVertexArrayState
//
PerContextVertexArrayState::PerContextVertexArrayState(osg::State* state, GLint basevertex):
    _state(state),
    _vertexArrayObject(0),
    _currentVBO(0),
    _currentEBO(0),
    _requiresSetArrays(true),
    _basevertex(basevertex)
{
    _ext = _state->get<GLExtensions>();
    _isVertexBufferObjectSupported =  _ext->isBufferObjectSupported;
}

void PerContextVertexArrayState::generateVertexArrayObject()
{
    _ext->glGenVertexArrays(1, &_vertexArrayObject);
}

void PerContextVertexArrayState::deleteVertexArrayObject()
{
    if (_vertexArrayObject)
    {
        VAS_NOTICE<<"  PerContextVertexArrayState::deleteVertexArrayObject() "<<_vertexArrayObject<<std::endl;

        _ext->glDeleteVertexArrays(1, &_vertexArrayObject);
        _vertexArrayObject = 0;
    }
}

void PerContextVertexArrayState::assignVertexArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_state->getUseVertexAttributeAliasing())
    {
        _vertexArray = new VertexArrayDispatch(_basevertex);
    }
    else
#endif
    {
        VAS_NOTICE<<"PerContextVertexArrayState::assignNormalArrayDispatcher() _state->getVertexAlias()._location="<<_state->getVertexAlias()._location<<std::endl;
        _vertexArray = new VertexAttribArrayDispatch(_state->getVertexAlias()._location,_basevertex);
    }
}

void PerContextVertexArrayState::assignNormalArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_state->getUseVertexAttributeAliasing())
    {
        _normalArray = new NormalArrayDispatch(_basevertex);
    }
    else
#endif
    {
        VAS_NOTICE<<"PerContextVertexArrayState::assignNormalArrayDispatcher() _state->getNormalAlias()._location="<<_state->getNormalAlias()._location<<std::endl;
        _normalArray = new VertexAttribArrayDispatch(_state->getNormalAlias()._location,_basevertex);
    }
}

void PerContextVertexArrayState::assignColorArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_state->getUseVertexAttributeAliasing())
    {
        _colorArray = new ColorArrayDispatch(_basevertex);
    }
    else
#endif
    {
        VAS_NOTICE<<"PerContextVertexArrayState::assignColorArrayDispatcher() _state->getColorAlias()._location="<<_state->getColorAlias()._location<<std::endl;
        _colorArray = new VertexAttribArrayDispatch(_state->getColorAlias()._location,_basevertex);
    }
}

void PerContextVertexArrayState::assignSecondaryColorArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_state->getUseVertexAttributeAliasing())
    {
        _secondaryColorArray = new SecondaryColorArrayDispatch(_basevertex);
    }
    else
#endif
    {
        _secondaryColorArray = new VertexAttribArrayDispatch(_state->getSecondaryColorAlias()._location,_basevertex);
    }
}

void PerContextVertexArrayState::assignFogCoordArrayDispatcher()
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_state->getUseVertexAttributeAliasing())
    {
        _fogCoordArray = new FogCoordArrayDispatch(_basevertex);
    }
    else
#endif
    {
        _fogCoordArray = new VertexAttribArrayDispatch(_state->getFogCoordAlias()._location,_basevertex);
    }
}

void PerContextVertexArrayState::assignTexCoordArrayDispatcher(unsigned int numUnits)
{
#ifdef OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
    if (!_state->getUseVertexAttributeAliasing())
    {
        _texCoordArrays.clear();
        for(unsigned int i=0; i<numUnits; ++i)
        {
            _texCoordArrays.push_back( new TexCoordArrayDispatch(i,_basevertex) );
        }
    }
    else
#endif
    {
        _texCoordArrays.clear();
        for(unsigned int i=0; i<numUnits; ++i)
        {
            VAS_NOTICE<<"PerContextVertexArrayState::PerContextVertexArrayState::assignTexCoordArrayDispatcher() _state->getTexCoordAliasList()[i]._location="<<_state->getTexCoordAliasList()[i]._location<<std::endl;
            _texCoordArrays.push_back( new VertexAttribArrayDispatch(_state->getTexCoordAliasList()[i]._location,_basevertex) );
        }
    }
}

void PerContextVertexArrayState::assignVertexAttribArrayDispatcher(unsigned int numUnits)
{
    _vertexAttribArrays.clear();
    for(unsigned int i=0; i<numUnits; ++i)
    {
        _vertexAttribArrays.push_back( new VertexAttribArrayDispatch(i,_basevertex) );
    }
}

void PerContextVertexArrayState::assignAllDispatchers()
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

void PerContextVertexArrayState::release()
{
    VAS_NOTICE<<"PerContextVertexArrayState::release() "<<this<<std::endl;

    osg::get<PerContextVertexArrayStateManager>(_ext->contextID)->release(this);
}

void PerContextVertexArrayState::setArray(ArrayDispatch* vad, osg::State& state, const osg::Array* new_array)
{
    if (new_array)
    {
        if (!vad->active)
        {
            vad->active = true;
            _activeDispatchers.push_back(vad);
        }

        if (vad->array==0)
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
        else if (new_array!=vad->array || new_array->getModifiedCount()!=vad->modifiedCount)
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

        vad->array = new_array;
        vad->modifiedCount = new_array->getModifiedCount();

    }
    else if (vad->array)
    {
        disable(vad, state);
    }
}

void PerContextVertexArrayState::setArray(ArrayDispatch* vad, osg::State& state, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr, GLboolean normalized)
{
    if (ptr)
    {
        if (!vad->active)
        {
            vad->active = true;
            _activeDispatchers.push_back(vad);
        }

        if (vad->array==0)
        {
            unbindVertexBufferObject();
            vad->enable_and_dispatch(state, size, type, stride, ptr, normalized);
        }
        else
        {
            unbindVertexBufferObject();
            vad->dispatch(state, size, type, stride, ptr, normalized);
        }

        vad->array = 0;
        vad->modifiedCount = 0xffffffff;

    }
    else if (vad->array)
    {
        disable(vad, state);
    }
}

void PerContextVertexArrayState::setInterleavedArrays( osg::State& state, GLenum format, GLsizei stride, const GLvoid* pointer)
{
#if defined(OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE) && !defined(OSG_GLES1_AVAILABLE)
    unbindVertexBufferObject();

    //lazyDisablingOfVertexAttributes();
    //applyDisablingOfVertexAttributes(state);

    glInterleavedArrays( format, stride, pointer);
#else
    OSG_NOTICE<<"Warning: State::setInterleavedArrays(..) not implemented."<<std::endl;
#endif
}

void PerContextVertexArrayState::dirty()
{
    setRequiresSetArrays(true);
}

