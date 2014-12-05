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
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/Drawable>
#include <osg/State>
#include <osg/Notify>
#include <osg/Node>
#include <osg/GLExtensions>
#include <osg/Timer>
#include <osg/TriangleFunctor>
#include <osg/io_utils>

#include <algorithm>
#include <map>
#include <list>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

using namespace osg;

unsigned int Drawable::s_numberDrawablesReusedLastInLastFrame = 0;
unsigned int Drawable::s_numberNewDrawablesInLastFrame = 0;
unsigned int Drawable::s_numberDeletedDrawablesInLastFrame = 0;

// static cache of deleted display lists which can only
// by completely deleted once the appropriate OpenGL context
// is set.  Used osg::Drawable::deleteDisplayList(..) and flushDeletedDisplayLists(..) below.
typedef std::multimap<unsigned int,GLuint> DisplayListMap;
typedef osg::buffered_object<DisplayListMap> DeletedDisplayListCache;

static OpenThreads::Mutex s_mutex_deletedDisplayListCache;
static DeletedDisplayListCache s_deletedDisplayListCache;

GLuint Drawable::generateDisplayList(unsigned int contextID, unsigned int sizeHint)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedDisplayListCache);

    DisplayListMap& dll = s_deletedDisplayListCache[contextID];
    if (dll.empty())
    {
        ++s_numberNewDrawablesInLastFrame;
        return  glGenLists( 1 );
    }
    else
    {
        DisplayListMap::iterator itr = dll.lower_bound(sizeHint);
        if (itr!=dll.end())
        {
            // OSG_NOTICE<<"Reusing a display list of size = "<<itr->first<<" for requested size = "<<sizeHint<<std::endl;

            ++s_numberDrawablesReusedLastInLastFrame;

            GLuint globj = itr->second;
            dll.erase(itr);

            return globj;
        }
        else
        {
            // OSG_NOTICE<<"Creating a new display list of size = "<<sizeHint<<" although "<<dll.size()<<" are available"<<std::endl;
            ++s_numberNewDrawablesInLastFrame;
            return  glGenLists( 1 );
        }
    }
#else
    OSG_NOTICE<<"Warning: Drawable::generateDisplayList(..) - not supported."<<std::endl;
    return 0;
#endif
}

unsigned int s_minimumNumberOfDisplayListsToRetainInCache = 0;
void Drawable::setMinimumNumberOfDisplayListsToRetainInCache(unsigned int minimum)
{
    s_minimumNumberOfDisplayListsToRetainInCache = minimum;
}

unsigned int Drawable::getMinimumNumberOfDisplayListsToRetainInCache()
{
    return s_minimumNumberOfDisplayListsToRetainInCache;
}

void Drawable::deleteDisplayList(unsigned int contextID,GLuint globj, unsigned int sizeHint)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    if (globj!=0)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedDisplayListCache);

        // insert the globj into the cache for the appropriate context.
        s_deletedDisplayListCache[contextID].insert(DisplayListMap::value_type(sizeHint,globj));
    }
#else
    OSG_NOTICE<<"Warning: Drawable::deleteDisplayList(..) - not supported."<<std::endl;
#endif
}

void Drawable::flushAllDeletedDisplayLists(unsigned int contextID)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedDisplayListCache);

    DisplayListMap& dll = s_deletedDisplayListCache[contextID];

    for(DisplayListMap::iterator ditr=dll.begin();
        ditr!=dll.end();
        ++ditr)
    {
        glDeleteLists(ditr->second,1);
    }

    dll.clear();
#else
    OSG_NOTICE<<"Warning: Drawable::deleteDisplayList(..) - not supported."<<std::endl;
#endif
}

void Drawable::discardAllDeletedDisplayLists(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedDisplayListCache);

    DisplayListMap& dll = s_deletedDisplayListCache[contextID];
    dll.clear();
}

void Drawable::flushDeletedDisplayLists(unsigned int contextID, double& availableTime)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    unsigned int noDeleted = 0;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedDisplayListCache);

        DisplayListMap& dll = s_deletedDisplayListCache[contextID];

        bool trimFromFront = true;
        if (trimFromFront)
        {
            unsigned int prev_size = dll.size();

            DisplayListMap::iterator ditr=dll.begin();
            unsigned int maxNumToDelete = (dll.size() > s_minimumNumberOfDisplayListsToRetainInCache) ? dll.size()-s_minimumNumberOfDisplayListsToRetainInCache : 0;
            for(;
                ditr!=dll.end() && elapsedTime<availableTime && noDeleted<maxNumToDelete;
                ++ditr)
            {
                glDeleteLists(ditr->second,1);

                elapsedTime = timer.delta_s(start_tick,timer.tick());
                ++noDeleted;

                ++Drawable::s_numberDeletedDrawablesInLastFrame;
             }

             if (ditr!=dll.begin()) dll.erase(dll.begin(),ditr);

             if (noDeleted+dll.size() != prev_size)
             {
                OSG_WARN<<"Error in delete"<<std::endl;
             }
        }
        else
        {
            unsigned int prev_size = dll.size();

            DisplayListMap::reverse_iterator ditr=dll.rbegin();
            unsigned int maxNumToDelete = (dll.size() > s_minimumNumberOfDisplayListsToRetainInCache) ? dll.size()-s_minimumNumberOfDisplayListsToRetainInCache : 0;
            for(;
                ditr!=dll.rend() && elapsedTime<availableTime && noDeleted<maxNumToDelete;
                ++ditr)
            {
                glDeleteLists(ditr->second,1);

                elapsedTime = timer.delta_s(start_tick,timer.tick());
                ++noDeleted;

                ++Drawable::s_numberDeletedDrawablesInLastFrame;
             }

             if (ditr!=dll.rbegin()) dll.erase(ditr.base(),dll.end());

             if (noDeleted+dll.size() != prev_size)
             {
                OSG_WARN<<"Error in delete"<<std::endl;
             }
        }
    }
    elapsedTime = timer.delta_s(start_tick,timer.tick());

    if (noDeleted!=0) OSG_INFO<<"Number display lists deleted = "<<noDeleted<<" elapsed time"<<elapsedTime<<std::endl;

    availableTime -= elapsedTime;
#else
    OSG_NOTICE<<"Warning: Drawable::flushDeletedDisplayLists(..) - not supported."<<std::endl;
#endif
}

bool Drawable::UpdateCallback::run(osg::Object* object, osg::Object* data)
{
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(object);
    osg::NodeVisitor* nv = dynamic_cast<osg::NodeVisitor*>(data);
    if (drawable && nv)
    {
        update(nv, drawable);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}

bool Drawable::EventCallback::run(osg::Object* object, osg::Object* data)
{
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(object);
    osg::NodeVisitor* nv = dynamic_cast<osg::NodeVisitor*>(data);
    if (drawable && nv)
    {
        event(nv, drawable);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}

Drawable::Drawable()
{
    _boundingBoxComputed = false;

    // Note, if your are defining a subclass from drawable which is
    // dynamically updated then you should set both the following to
    // to false in your constructor.  This will prevent any display
    // lists from being automatically created and safeguard the
    // dynamic updating of data.
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    _supportsDisplayList = true;
    _useDisplayList = true;
#else
    _supportsDisplayList = false;
    _useDisplayList = false;
#endif

    _supportsVertexBufferObjects = false;
    _useVertexBufferObjects = false;
    // _useVertexBufferObjects = true;
}

Drawable::Drawable(const Drawable& drawable,const CopyOp& copyop):
    Node(drawable,copyop),
    _initialBound(drawable._initialBound),
    _computeBoundCallback(drawable._computeBoundCallback),
    _boundingBox(drawable._boundingBox),
    _boundingBoxComputed(drawable._boundingBoxComputed),
    _shape(copyop(drawable._shape.get())),
    _supportsDisplayList(drawable._supportsDisplayList),
    _useDisplayList(drawable._useDisplayList),
    _supportsVertexBufferObjects(drawable._supportsVertexBufferObjects),
    _useVertexBufferObjects(drawable._useVertexBufferObjects),
    _drawableUpdateCallback(drawable._drawableUpdateCallback),
    _drawableEventCallback(drawable._drawableEventCallback),
    _drawableCullCallback(drawable._drawableCullCallback),
    _drawCallback(drawable._drawCallback)
{
    setStateSet(copyop(drawable._stateset.get()));
}

Drawable::~Drawable()
{
    dirtyDisplayList();
}

osg::MatrixList Drawable::getWorldMatrices(const osg::Node* haltTraversalAtNode) const
{
    osg::MatrixList matrices;
    for(ParentList::const_iterator itr = _parents.begin();
        itr != _parents.end();
        ++itr)
    {
        osg::MatrixList localMatrices = (*itr)->getWorldMatrices(haltTraversalAtNode);
        matrices.insert(matrices.end(), localMatrices.begin(), localMatrices.end());
    }
    return matrices;
}

void Drawable::computeDataVariance()
{
    if (getDataVariance() != UNSPECIFIED) return;

    bool dynamic = false;

    if (getUpdateCallback() ||
        getEventCallback() ||
        getCullCallback())
    {
        dynamic = true;
    }

    setDataVariance(dynamic ? DYNAMIC : STATIC);
}

void Drawable::compileGLObjects(RenderInfo& renderInfo) const
{
    if (!_useDisplayList) return;

#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    unsigned int contextID = renderInfo.getContextID();

    // get the globj for the current contextID.
    GLuint& globj = _globjList[contextID];

    // call the globj if already set otherwise compile and execute.
    if( globj != 0 )
    {
        glDeleteLists( globj, 1 );
    }

    globj = generateDisplayList(contextID, getGLObjectSizeHint());
    glNewList( globj, GL_COMPILE );

    if (_drawCallback.valid())
        _drawCallback->drawImplementation(renderInfo,this);
    else
        drawImplementation(renderInfo);

    glEndList();
#else
    OSG_NOTICE<<"Warning: Drawable::compileGLObjects(RenderInfo&) - not supported."<<std::endl;
#endif
}

void Drawable::setThreadSafeRefUnref(bool threadSafe)
{
    Object::setThreadSafeRefUnref(threadSafe);

    if (_stateset.valid()) _stateset->setThreadSafeRefUnref(threadSafe);
    if (_drawableUpdateCallback.valid()) _drawableUpdateCallback->setThreadSafeRefUnref(threadSafe);
    if (_drawableEventCallback.valid()) _drawableEventCallback->setThreadSafeRefUnref(threadSafe);
    if (_drawableCullCallback.valid()) _drawableCullCallback->setThreadSafeRefUnref(threadSafe);
    if (_drawCallback.valid()) _drawCallback->setThreadSafeRefUnref(threadSafe);
}

void Drawable::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_stateset.valid()) _stateset->resizeGLObjectBuffers(maxSize);
    if (_drawCallback.valid()) _drawCallback->resizeGLObjectBuffers(maxSize);

    _globjList.resize(maxSize);
}

void Drawable::releaseGLObjects(State* state) const
{
    if (_stateset.valid()) _stateset->releaseGLObjects(state);

    if (_drawCallback.valid()) _drawCallback->releaseGLObjects(state);

    if (!_useDisplayList) return;

    if (state)
    {
        // get the contextID (user defined ID of 0 upwards) for the
        // current OpenGL context.
        unsigned int contextID = state->getContextID();

        // get the globj for the current contextID.
        GLuint& globj = _globjList[contextID];

        // call the globj if already set otherwise compile and execute.
        if( globj != 0 )
        {
            Drawable::deleteDisplayList(contextID,globj, getGLObjectSizeHint());
            globj = 0;
        }
    }
    else
    {
        const_cast<Drawable*>(this)->dirtyDisplayList();
    }
}

void Drawable::setSupportsDisplayList(bool flag)
{
    // if value unchanged simply return.
    if (_supportsDisplayList==flag) return;

#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
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
#else
    _supportsDisplayList=false;
#endif
}

void Drawable::setUseDisplayList(bool flag)
{
    // if value unchanged simply return.
    if (_useDisplayList==flag) return;

#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
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
            OSG_WARN<<"Warning: attempt to setUseDisplayList(true) on a drawable with does not support display lists."<<std::endl;
        }
        else
        {
            // set with new value.
            _useDisplayList = false;
        }
    }
#else
   _useDisplayList = false;
#endif
}


void Drawable::setUseVertexBufferObjects(bool flag)
{
    // _useVertexBufferObjects = true;

    // OSG_NOTICE<<"Drawable::setUseVertexBufferObjects("<<flag<<")"<<std::endl;

    // if value unchanged simply return.
    if (_useVertexBufferObjects==flag) return;

    // if was previously set to true, remove display list.
    if (_useVertexBufferObjects)
    {
        dirtyDisplayList();
    }

    _useVertexBufferObjects = flag;
}

void Drawable::dirtyDisplayList()
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    unsigned int i;
    for(i=0;i<_globjList.size();++i)
    {
        if (_globjList[i] != 0)
        {
            Drawable::deleteDisplayList(i,_globjList[i], getGLObjectSizeHint());
            _globjList[i] = 0;
        }
    }
#endif
}


struct ComputeBound : public PrimitiveFunctor
{
        ComputeBound()
        {
            _vertices2f = 0;
            _vertices3f = 0;
            _vertices4f = 0;
            _vertices2d = 0;
            _vertices3d = 0;
            _vertices4d = 0;
        }

        virtual void setVertexArray(unsigned int,const Vec2* vertices) { _vertices2f = vertices; }
        virtual void setVertexArray(unsigned int,const Vec3* vertices) { _vertices3f = vertices; }
        virtual void setVertexArray(unsigned int,const Vec4* vertices) { _vertices4f = vertices; }

        virtual void setVertexArray(unsigned int,const Vec2d* vertices) { _vertices2d  = vertices; }
        virtual void setVertexArray(unsigned int,const Vec3d* vertices) { _vertices3d  = vertices; }
        virtual void setVertexArray(unsigned int,const Vec4d* vertices) { _vertices4d = vertices; }

        template<typename T>
        void _drawArrays(T* vert, T* end)
        {
            for(;vert<end;++vert)
            {
                vertex(*vert);
            }
        }


        template<typename T, typename I>
        void _drawElements(T* vert, I* indices, I* end)
        {
            for(;indices<end;++indices)
            {
                vertex(vert[*indices]);
            }
        }

        virtual void drawArrays(GLenum,GLint first,GLsizei count)
        {
            if      (_vertices3f) _drawArrays(_vertices3f+first, _vertices3f+(first+count));
            else if (_vertices2f) _drawArrays(_vertices2f+first, _vertices2f+(first+count));
            else if (_vertices4f) _drawArrays(_vertices4f+first, _vertices4f+(first+count));
            else if (_vertices2d) _drawArrays(_vertices2d+first, _vertices2d+(first+count));
            else if (_vertices3d) _drawArrays(_vertices3d+first, _vertices3d+(first+count));
            else if (_vertices4d) _drawArrays(_vertices4d+first, _vertices4d+(first+count));
        }

        virtual void drawElements(GLenum,GLsizei count,const GLubyte* indices)
        {
            if (_vertices3f) _drawElements(_vertices3f, indices, indices + count);
            else if (_vertices2f) _drawElements(_vertices2f, indices, indices + count);
            else if (_vertices4f) _drawElements(_vertices4f, indices, indices + count);
            else if (_vertices2d) _drawElements(_vertices2d, indices, indices + count);
            else if (_vertices3d) _drawElements(_vertices3d, indices, indices + count);
            else if (_vertices4d) _drawElements(_vertices4d, indices, indices + count);
        }

        virtual void drawElements(GLenum,GLsizei count,const GLushort* indices)
        {
            if      (_vertices3f) _drawElements(_vertices3f, indices, indices + count);
            else if (_vertices2f) _drawElements(_vertices2f, indices, indices + count);
            else if (_vertices4f) _drawElements(_vertices4f, indices, indices + count);
            else if (_vertices2d) _drawElements(_vertices2d, indices, indices + count);
            else if (_vertices3d) _drawElements(_vertices3d, indices, indices + count);
            else if (_vertices4d) _drawElements(_vertices4d, indices, indices + count);
        }

        virtual void drawElements(GLenum,GLsizei count,const GLuint* indices)
        {
            if      (_vertices3f) _drawElements(_vertices3f, indices, indices + count);
            else if (_vertices2f) _drawElements(_vertices2f, indices, indices + count);
            else if (_vertices4f) _drawElements(_vertices4f, indices, indices + count);
            else if (_vertices2d) _drawElements(_vertices2d, indices, indices + count);
            else if (_vertices3d) _drawElements(_vertices3d, indices, indices + count);
            else if (_vertices4d) _drawElements(_vertices4d, indices, indices + count);
        }

        virtual void begin(GLenum) {}
        virtual void vertex(const Vec2& vert) { _bb.expandBy(osg::Vec3(vert[0],vert[1],0.0f)); }
        virtual void vertex(const Vec3& vert) { _bb.expandBy(vert); }
        virtual void vertex(const Vec4& vert) { if (vert[3]!=0.0f) _bb.expandBy(osg::Vec3(vert[0],vert[1],vert[2])/vert[3]); }
        virtual void vertex(const Vec2d& vert) { _bb.expandBy(osg::Vec3(vert[0],vert[1],0.0f)); }
        virtual void vertex(const Vec3d& vert) { _bb.expandBy(vert); }
        virtual void vertex(const Vec4d& vert) { if (vert[3]!=0.0f) _bb.expandBy(osg::Vec3(vert[0],vert[1],vert[2])/vert[3]); }
        virtual void vertex(float x,float y)  { _bb.expandBy(x,y,1.0f); }
        virtual void vertex(float x,float y,float z) { _bb.expandBy(x,y,z); }
        virtual void vertex(float x,float y,float z,float w) { if (w!=0.0f) _bb.expandBy(x/w,y/w,z/w); }
        virtual void vertex(double x,double y)  { _bb.expandBy(x,y,1.0f); }
        virtual void vertex(double x,double y,double z) { _bb.expandBy(x,y,z); }
        virtual void vertex(double x,double y,double z,double w) { if (w!=0.0f) _bb.expandBy(x/w,y/w,z/w); }
        virtual void end() {}

        const Vec2*     _vertices2f;
        const Vec3*     _vertices3f;
        const Vec4*     _vertices4f;
        const Vec2d*    _vertices2d;
        const Vec3d*    _vertices3d;
        const Vec4d*    _vertices4d;
        BoundingBox     _bb;
};

BoundingSphere Drawable::computeBound() const
{
    return BoundingSphere(getBoundingBox());
}

BoundingBox Drawable::computeBoundingBox() const
{
    ComputeBound cb;

    Drawable* non_const_this = const_cast<Drawable*>(this);
    non_const_this->accept(cb);

#if 0
    OSG_NOTICE<<"computeBound() "<<cb._bb.xMin()<<", "<<cb._bb.xMax()<<", "<<std::endl;
    OSG_NOTICE<<"               "<<cb._bb.yMin()<<", "<<cb._bb.yMax()<<", "<<std::endl;
    OSG_NOTICE<<"               "<<cb._bb.zMin()<<", "<<cb._bb.zMax()<<", "<<std::endl;
#endif

    return cb._bb;
}

void Drawable::setBound(const BoundingBox& bb) const
{
     _boundingBox = bb;
     _boundingBoxComputed = true;
}
