/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/Drawable>
#include <osg/State>
#include <osg/Notify>
#include <osg/Node>
#include <osg/GLExtensions>

#include <algorithm>
#include <map>
#include <set>

using namespace osg;

// static cache of deleted display lists which can only 
// by completely deleted once the appropriate OpenGL context
// is set.  Used osg::Drawable::deleteDisplayList(..) and flushDeletedDisplayLists(..) below.
typedef std::vector<GLuint> DisplayListVector;
typedef std::map<GLuint,DisplayListVector> DeletedDisplayListCache;
static DeletedDisplayListCache s_deletedDisplayListCache;
static DeletedDisplayListCache s_deletedVertexBufferObjectCache;

void Drawable::deleteDisplayList(unsigned int contextID,GLuint globj)
{
    if (globj!=0)
    {
        // insert the globj into the cache for the appropriate context.
        s_deletedDisplayListCache[contextID].push_back(globj);
    }
}

/** flush all the cached display list which need to be deleted
  * in the OpenGL context related to contextID.*/
void Drawable::flushDeletedDisplayLists(unsigned int contextID)
{
    DeletedDisplayListCache::iterator citr = s_deletedDisplayListCache.find(contextID);
    if (citr!=s_deletedDisplayListCache.end())
    {
        DisplayListVector displayListSet;
        displayListSet.reserve(1000);
        
        // this swap will transfer the content of and empty citr->second
        // in one quick pointer change.
        displayListSet.swap(citr->second);
        
        for(DisplayListVector::iterator gitr=displayListSet.begin();
                                     gitr!=displayListSet.end();
                                     ++gitr)
        {
            glDeleteLists(*gitr,1);
        }
    }
}

void Drawable::deleteVertexBufferObject(unsigned int contextID,GLuint globj)
{
    if (globj!=0)
    {
        // insert the globj into the cache for the appropriate context.
        s_deletedVertexBufferObjectCache[contextID].push_back(globj);
    }
}

/** flush all the cached display list which need to be deleted
  * in the OpenGL context related to contextID.*/
void Drawable::flushDeletedVertexBufferObjects(unsigned int contextID)
{
    DeletedDisplayListCache::iterator citr = s_deletedVertexBufferObjectCache.find(contextID);
    if (citr!=s_deletedVertexBufferObjectCache.end())
    {
        const Extensions* extensions = getExtensions(contextID,true);

        DisplayListVector displayListSet;
        displayListSet.reserve(1000);
        
        // this swap will transfer the content of and empty citr->second
        // in one quick pointer change.
        displayListSet.swap(citr->second);
        
        for(DisplayListVector::iterator gitr=displayListSet.begin();
                                     gitr!=displayListSet.end();
                                     ++gitr)
        {
            extensions->glDeleteBuffers(1,&(*gitr));
        }
    }
}


Drawable::Drawable()
{
    _bbox_computed = false;

    // Note, if your are defining a subclass from drawable which is
    // dynamically updated then you should set both the following to
    // to false in your constructor.  This will prevent any display
    // lists from being automatically created and safeguard the
    // dynamic updating of data.
    _supportsDisplayList = true;
    _useDisplayList = true;

    _supportsVertexBufferObjects = false;
    _useVertexBufferObjects = true;
}

Drawable::Drawable(const Drawable& drawable,const CopyOp& copyop):
    Object(drawable,copyop),
    _parents(), // leave empty as parentList is managed by Geode
    _stateset(copyop(drawable._stateset.get())),
    _bbox(drawable._bbox),
    _bbox_computed(drawable._bbox_computed),
    _shape(copyop(drawable._shape.get())),
    _supportsDisplayList(drawable._supportsDisplayList),
    _useDisplayList(drawable._useDisplayList),
    _supportsVertexBufferObjects(drawable._supportsVertexBufferObjects),
    _useVertexBufferObjects(drawable._useVertexBufferObjects),
    _drawCallback(drawable._drawCallback),
    _cullCallback(drawable._cullCallback)
{
}

Drawable::~Drawable()
{
    dirtyDisplayList();
}

void Drawable::addParent(osg::Node* node)
{
    _parents.push_back(node);    
}

void Drawable::removeParent(osg::Node* node)
{
    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),node);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}

osg::StateSet* Drawable::getOrCreateStateSet()
{
    if (!_stateset) _stateset = new StateSet;
    return _stateset.get();
}

void Drawable::dirtyBound()
{
    if (_bbox_computed)
    {
        _bbox_computed = false;

        // dirty parent bounding sphere's to ensure that all are valid.
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->dirtyBound();
        }

    }
}

void Drawable::compile(State& state) const
{
    if (!_useDisplayList) return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    unsigned int contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& globj = _globjList[contextID];

    // call the globj if already set otherwise comple and execute.
    if( globj != 0 )
    {
        glDeleteLists( globj, 1 );
    }

    
    if (_stateset.valid())
    {
        _stateset->compile(state);
    }

    globj = glGenLists( 1 );
    glNewList( globj, GL_COMPILE );

    if (_drawCallback.valid())
        _drawCallback->drawImplementation(state,this);
    else 
        drawImplementation(state);

    glEndList();

}

void Drawable::setSupportsDisplayList(bool flag)
{
    // if value unchanged simply return.
    if (_supportsDisplayList==flag) return;
    
    // if previously set to true then need to check about display lists.
    if (_supportsDisplayList)
    {
        if (_useDisplayList)
        {
            // used to support display lists and display lists switched
            // on so now delete them and turn useDisplayList off.
            dirtyDisplayList();
            _useDisplayList = false;
        }
    }
    
    // set with new value.
    _supportsDisplayList=flag;
}

void Drawable::setUseDisplayList(bool flag)
{
    // if value unchanged simply return.
    if (_useDisplayList==flag) return;

    // if was previously set to true, remove display list.
    if (_useDisplayList)
    {
        dirtyDisplayList();
    }
    
    if (_supportsDisplayList)
    {
    
        // set with new value.
        _useDisplayList = flag;
        
    }
    else // does not support display lists.
    {
        if (flag)
        {
            notify(WARN)<<"Warning: attempt to setUseDisplayList(true) on a drawable with does not support display lists."<<std::endl;
        }
        else 
        {
            // set with new value.
            _useDisplayList = false;
        }
    }
}


void Drawable::dirtyDisplayList()
{
    for(unsigned int i=0;i<_globjList.size();++i)
    {
        if (_globjList[i] != 0)
        {
            Drawable::deleteDisplayList(i,_globjList[i]);
            _globjList[i] = 0;
        }
    }

    for(unsigned int i=0;i<_vboList.size();++i)
    {
        if (_vboList[i] != 0)
        {
            Drawable::deleteVertexBufferObject(i,_vboList[i]);
            _vboList[i] = 0;
        }
    }
}


void Drawable::setUpdateCallback(UpdateCallback* ac)
{
    if (_updateCallback==ac) return;
    
    int delta = 0;
    if (_updateCallback.valid()) --delta;
    if (ac) ++delta;

    _updateCallback = ac;
    
    if (delta!=0)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringUpdateTraversal((*itr)->getNumChildrenRequiringUpdateTraversal()+delta);
        }
    }
}

struct ComputeBound : public Drawable::PrimitiveFunctor
{
        ComputeBound():_vertices(0) {}
        
        virtual void setVertexArray(unsigned int,const Vec2*) 
        {
            notify(WARN)<<"ComputeBound does not support Vec2* vertex arrays"<<std::endl;
        }

        virtual void setVertexArray(unsigned int,const Vec3* vertices) { _vertices = vertices; }

        virtual void setVertexArray(unsigned int,const Vec4*) 
        {
            notify(WARN)<<"ComputeBound does not support Vec4* vertex arrays"<<std::endl;
        }

        virtual void drawArrays(GLenum,GLint first,GLsizei count)
        {
            if (_vertices)
            {
                const osg::Vec3* vert = _vertices+first;
                for(;count>0;--count,++vert)
                {
                    _bb.expandBy(*vert);
                }
            }
        }

        virtual void drawElements(GLenum,GLsizei count,const GLubyte* indices)
        {
            if (_vertices)
            {
                for(;count>0;--count,++indices)
                {
                    _bb.expandBy(_vertices[*indices]);
                }
            }
        }

        virtual void drawElements(GLenum,GLsizei count,const GLushort* indices)
        {
            if (_vertices)
            {
                for(;count>0;--count,++indices)
                {
                    _bb.expandBy(_vertices[*indices]);
                }
            }
        }

        virtual void drawElements(GLenum,GLsizei count,const GLuint* indices)
        {
            if (_vertices)
            {
                for(;count>0;--count,++indices)
                {
                    _bb.expandBy(_vertices[*indices]);
                }
            }
        }

        virtual void begin(GLenum) {}
        virtual void vertex(const Vec2& vert) { _bb.expandBy(osg::Vec3(vert[0],vert[1],0.0f)); }
        virtual void vertex(const Vec3& vert) { _bb.expandBy(vert); }
        virtual void vertex(const Vec4& vert) { if (vert[3]!=0.0f) _bb.expandBy(osg::Vec3(vert[0],vert[1],vert[2])/vert[3]); }
        virtual void vertex(float x,float y) { _bb.expandBy(x,y,1.0f); }
        virtual void vertex(float x,float y,float z) { _bb.expandBy(x,y,z); }
        virtual void vertex(float x,float y,float z,float w) { if (w!=0.0f) _bb.expandBy(x/w,y/w,z/w); }
        virtual void end() {}
        
        const Vec3*     _vertices;
        BoundingBox     _bb;
};

bool Drawable::computeBound() const
{
    ComputeBound cb;

    Drawable* non_const_this = const_cast<Drawable*>(this);
    non_const_this->accept(cb);
    
    _bbox = cb._bb;
    _bbox_computed = true;

    return true;
}

void Drawable::setBound(const BoundingBox& bb) const
{
     _bbox = bb;
     _bbox_computed = true;
}


//////////////////////////////////////////////////////////////////////////////
//
//  Extension support
//

typedef buffered_value< ref_ptr<Drawable::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Drawable::Extensions* Drawable::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Drawable::Extensions;
    return s_extensions[contextID].get();
}

void Drawable::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Drawable::Extensions::Extensions()
{
    setupGLExtenions();
}

Drawable::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isVertexProgramSupported = rhs._isVertexProgramSupported;
    _isSecondaryColorSupported = rhs._isSecondaryColorSupported;
    _isFogCoordSupported = rhs._isFogCoordSupported;
    _isMultiTexSupported = rhs._isMultiTexSupported;
    _glFogCoordfv = rhs._glFogCoordfv;
    _glSecondaryColor3ubv = rhs._glSecondaryColor3ubv;
    _glSecondaryColor3fv = rhs._glSecondaryColor3fv;
    _glMultiTexCoord1f = rhs._glMultiTexCoord1f;
    _glMultiTexCoord2fv = rhs._glMultiTexCoord2fv;
    _glMultiTexCoord3fv = rhs._glMultiTexCoord3fv;
    _glMultiTexCoord4fv = rhs._glMultiTexCoord4fv;
    _glVertexAttrib1s = rhs._glVertexAttrib1s;
    _glVertexAttrib1f = rhs._glVertexAttrib1f;
    _glVertexAttrib2fv = rhs._glVertexAttrib2fv;
    _glVertexAttrib3fv = rhs._glVertexAttrib3fv;
    _glVertexAttrib4fv = rhs._glVertexAttrib4fv;
    _glVertexAttrib4ubv = rhs._glVertexAttrib4ubv;
    _glVertexAttrib4Nubv = rhs._glVertexAttrib4Nubv;
    _glGenBuffers = rhs._glGenBuffers;
    _glBindBuffer = rhs._glBindBuffer;
    _glBufferData = rhs._glBufferData;
    _glBufferSubData = rhs._glBufferSubData;
    _glDeleteBuffers = rhs._glDeleteBuffers;
}


void Drawable::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isVertexProgramSupported) _isVertexProgramSupported = false;
    if (!rhs._isSecondaryColorSupported) _isSecondaryColorSupported = false;
    if (!rhs._isFogCoordSupported) _isFogCoordSupported = false;
    if (!rhs._isMultiTexSupported) _isMultiTexSupported = false;

    if (!rhs._glFogCoordfv) _glFogCoordfv = 0;
    if (!rhs._glSecondaryColor3ubv) _glSecondaryColor3ubv = 0;
    if (!rhs._glSecondaryColor3fv) _glSecondaryColor3fv = 0;
    if (!rhs._glMultiTexCoord1f) _glMultiTexCoord1f = 0;
    if (!rhs._glMultiTexCoord2fv) _glMultiTexCoord2fv = 0;
    if (!rhs._glMultiTexCoord3fv) _glMultiTexCoord3fv = 0;
    if (!rhs._glMultiTexCoord4fv) _glMultiTexCoord4fv = 0;

    if (!rhs._glVertexAttrib1s) _glVertexAttrib1s = 0;
    if (!rhs._glVertexAttrib1f) _glVertexAttrib1f = 0;
    if (!rhs._glVertexAttrib2fv) _glVertexAttrib2fv = 0;
    if (!rhs._glVertexAttrib3fv) _glVertexAttrib3fv = 0;
    if (!rhs._glVertexAttrib4fv) _glVertexAttrib4fv = 0;
    if (!rhs._glVertexAttrib4ubv) _glVertexAttrib4ubv = 0;
    if (!rhs._glVertexAttrib4Nubv) _glVertexAttrib4Nubv = 0;

    if (!rhs._glGenBuffers) _glGenBuffers = 0;
    if (!rhs._glBindBuffer) _glBindBuffer = 0;
    if (!rhs._glBufferData) _glBufferData = 0;
    if (!rhs._glBufferSubData) _glBufferSubData = 0;
    if (!rhs._glDeleteBuffers) _glDeleteBuffers = 0;
}

void Drawable::Extensions::setupGLExtenions()
{
    _isVertexProgramSupported = isGLExtensionSupported("GL_ARB_vertex_program");
    _isSecondaryColorSupported = isGLExtensionSupported("GL_EXT_secondary_color");
    _isFogCoordSupported = isGLExtensionSupported("GL_EXT_fog_coord");
    _isMultiTexSupported = isGLExtensionSupported("GL_ARB_multitexture");

    _glFogCoordfv = ((FogCoordProc)osg::getGLExtensionFuncPtr("glFogCoordfv","glFogCoordfvEXT"));
    _glSecondaryColor3ubv = ((SecondaryColor3ubvProc)osg::getGLExtensionFuncPtr("glSecondaryColor3ubv","glSecondaryColor3ubvEXT"));
    _glSecondaryColor3fv = ((SecondaryColor3fvProc)osg::getGLExtensionFuncPtr("glSecondaryColor3fv","glSecondaryColor3fvEXT"));
    _glMultiTexCoord1f = ((MultiTexCoord1fProc)osg::getGLExtensionFuncPtr("glMultiTexCoord1f","glMultiTexCoord1fARB"));
    _glMultiTexCoord2fv = ((MultiTexCoordfvProc)osg::getGLExtensionFuncPtr("glMultiTexCoord2fv","glMultiTexCoord2fvARB"));
    _glMultiTexCoord3fv = ((MultiTexCoordfvProc)osg::getGLExtensionFuncPtr("glMultiTexCoord3fv","glMultiTexCoord3fvARB"));
    _glMultiTexCoord4fv = ((MultiTexCoordfvProc)osg::getGLExtensionFuncPtr("glMultiTexCoord4fv","glMultiTexCoord4fvARB"));

    _glVertexAttrib1s = ((VertexAttrib1sProc)osg::getGLExtensionFuncPtr("glVertexAttrib1s","glVertexAttrib1sARB"));
    _glVertexAttrib1f = ((VertexAttrib1fProc)osg::getGLExtensionFuncPtr("glVertexAttrib1f","glVertexAttrib1fARB"));
    _glVertexAttrib2fv = ((VertexAttribfvProc)osg::getGLExtensionFuncPtr("glVertexAttrib2fv","glVertexAttrib2fvARB"));
    _glVertexAttrib3fv = ((VertexAttribfvProc)osg::getGLExtensionFuncPtr("glVertexAttrib3fv","glVertexAttrib3fvARB"));
    _glVertexAttrib4fv = ((VertexAttribfvProc)osg::getGLExtensionFuncPtr("glVertexAttrib4fv","glVertexAttrib4fvARB"));
    _glVertexAttrib4ubv = ((VertexAttribubvProc)osg::getGLExtensionFuncPtr("glVertexAttrib4ubv","glVertexAttrib4ubvARB"));
    _glVertexAttrib4Nubv = ((VertexAttribubvProc)osg::getGLExtensionFuncPtr("glVertexAttrib4Nubv","glVertexAttrib4NubvARB"));

    _glGenBuffers = ((GenBuffersProc)osg::getGLExtensionFuncPtr("glGenBuffers","glGenBuffersARB"));
    _glBindBuffer = ((BindBufferProc)osg::getGLExtensionFuncPtr("glBindBuffer","glBindBufferARB"));
    _glBufferData = ((BufferDataProc)osg::getGLExtensionFuncPtr("glBufferData","glBufferDataARB"));
    _glBufferSubData = ((BufferSubDataProc)osg::getGLExtensionFuncPtr("glBufferSubData","glBufferSubDataARB"));
    _glDeleteBuffers = ((DeleteBuffersProc)osg::getGLExtensionFuncPtr("glDeleteBuffers","glDeleteBuffersARB"));

}
void Drawable::Extensions::glFogCoordfv(const GLfloat* coord) const
{
    if (_glFogCoordfv)
    {
        _glFogCoordfv(coord);
    }
    else
    {
        notify(WARN)<<"Error: glFogCoordfv not supported by OpenGL driver"<<std::endl;
    }    
}

void Drawable::Extensions::glSecondaryColor3ubv(const GLubyte* coord) const
{
    if (_glSecondaryColor3ubv)
    {
        _glSecondaryColor3ubv(coord);
    }
    else
    {
        notify(WARN)<<"Error: glSecondaryColor3ubv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glSecondaryColor3fv(const GLfloat* coord) const
{
    if (_glSecondaryColor3fv)
    {
        _glSecondaryColor3fv(coord);
    }
    else
    {
        notify(WARN)<<"Error: glSecondaryColor3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord1f(GLenum target,GLfloat coord) const
{
    if (_glMultiTexCoord1f)
    {
        _glMultiTexCoord1f(target,coord); 
    }
    else
    {
        notify(WARN)<<"Error: glMultiTexCoord1f not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord2fv(GLenum target,const GLfloat* coord) const
{
    if (_glMultiTexCoord2fv)
    {
        _glMultiTexCoord2fv(target,coord); 
    }
    else
    {
        notify(WARN)<<"Error: glMultiTexCoord2fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord3fv(GLenum target,const GLfloat* coord) const
{
    if (_glMultiTexCoord3fv)
    {
        _glMultiTexCoord3fv(target,coord); 
    }
    else
    {
        notify(WARN)<<"Error: _glMultiTexCoord3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord4fv(GLenum target,const GLfloat* coord) const
{
    if (_glMultiTexCoord4fv)
    {
        _glMultiTexCoord4fv(target,coord); 
    }
    else
    {
        notify(WARN)<<"Error: glMultiTexCoord4fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib1s(unsigned int index, GLshort s) const
{
    if (_glVertexAttrib1s)
    {
        _glVertexAttrib1s(index,s); 
    }
    else
    {
        notify(WARN)<<"Error: glVertexAttrib1s not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib1f(unsigned int index, GLfloat f) const
{
    if (_glVertexAttrib1f)
    {
        _glVertexAttrib1f(index,f); 
    }
    else
    {
        notify(WARN)<<"Error: glVertexAttrib1f not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib2fv(unsigned int index, const GLfloat * v) const
{
    if (_glVertexAttrib2fv)
    {
        _glVertexAttrib2fv(index,v); 
    }
    else
    {
        notify(WARN)<<"Error: glVertexAttrib2fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib3fv(unsigned int index, const GLfloat * v) const
{
    if (_glVertexAttrib3fv)
    {
        _glVertexAttrib3fv(index,v); 
    }
    else
    {
        notify(WARN)<<"Error: glVertexAttrib3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4fv(unsigned int index, const GLfloat * v) const
{
    if (_glVertexAttrib4fv)
    {
        _glVertexAttrib4fv(index,v); 
    }
    else
    {
        notify(WARN)<<"Error: glVertexAttrib4fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4ubv(unsigned int index, const GLubyte * v) const
{
    if (_glVertexAttrib4ubv)
    {
        _glVertexAttrib4ubv(index,v); 
    }
    else
    {
        notify(WARN)<<"Error: glVertexAttrib4ubv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4Nubv(unsigned int index, const GLubyte * v) const
{
    if (_glVertexAttrib4Nubv)
    {
        _glVertexAttrib4Nubv(index,v); 
    }
    else
    {
        notify(WARN)<<"Error: glVertexAttrib4Nubv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glGenBuffers(GLsizei n, GLuint *buffers) const
{
    if (_glGenBuffers)
    {
        _glGenBuffers(n, buffers); 
    }
    else
    {
        notify(WARN)<<"Error: glGenBuffers not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glBindBuffer(GLenum target, GLuint buffer) const
{
    if (_glBindBuffer)
    {
        _glBindBuffer(target, buffer); 
    }
    else
    {
        notify(WARN)<<"Error: glBindBuffer not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glBufferData(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage) const
{
    if (_glBufferData)
    {
        _glBufferData(target, size, data, usage); 
    }
    else
    {
        notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glBufferSubData(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data) const
{
    if (_glBufferSubData)
    {
        _glBufferSubData(target, offset, size, data); 
    }
    else
    {
        notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glDeleteBuffers(GLsizei n, const GLuint *buffers) const
{
    if (_glDeleteBuffers)
    {
        _glDeleteBuffers(n, buffers); 
    }
    else
    {
        notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
    }
}
