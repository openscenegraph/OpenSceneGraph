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

Drawable::Drawable()
    :Object(true)
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

    _numChildrenRequiringUpdateTraversal = 0;
    _numChildrenRequiringEventTraversal = 0;
}

Drawable::Drawable(const Drawable& drawable,const CopyOp& copyop):
    Object(drawable,copyop),
    _parents(), // leave empty as parentList is managed by Geode
    _initialBound(drawable._initialBound),
    _computeBoundCallback(drawable._computeBoundCallback),
    _boundingBox(drawable._boundingBox),
    _boundingBoxComputed(drawable._boundingBoxComputed),
    _shape(copyop(drawable._shape.get())),
    _supportsDisplayList(drawable._supportsDisplayList),
    _useDisplayList(drawable._useDisplayList),
    _supportsVertexBufferObjects(drawable._supportsVertexBufferObjects),
    _useVertexBufferObjects(drawable._useVertexBufferObjects),
    _updateCallback(drawable._updateCallback),
    _numChildrenRequiringUpdateTraversal(drawable._numChildrenRequiringUpdateTraversal),
    _eventCallback(drawable._eventCallback),
    _numChildrenRequiringEventTraversal(drawable._numChildrenRequiringEventTraversal),
    _cullCallback(drawable._cullCallback),
    _drawCallback(drawable._drawCallback)
{
    setStateSet(copyop(drawable._stateset.get()));
}

Drawable::~Drawable()
{
    // cleanly detatch any associated stateset (include remove parent links)
    setStateSet(0);

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

void Drawable::addParent(osg::Node* node)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    _parents.push_back(node);
}

void Drawable::removeParent(osg::Node* node)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),node);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}


void Drawable::setStateSet(osg::StateSet* stateset)
{
    // do nothing if nothing changed.
    if (_stateset==stateset) return;

    // track whether we need to account for the need to do a update or event traversal.
    int delta_update = 0;
    int delta_event = 0;

    // remove this node from the current statesets parent list
    if (_stateset.valid())
    {
        _stateset->removeParent(this);
        if (_stateset->requiresUpdateTraversal()) --delta_update;
        if (_stateset->requiresEventTraversal()) --delta_event;
    }

    // set the stateset.
    _stateset = stateset;

    // add this node to the new stateset to the parent list.
    if (_stateset.valid())
    {
        _stateset->addParent(this);
        if (_stateset->requiresUpdateTraversal()) ++delta_update;
        if (_stateset->requiresEventTraversal()) ++delta_event;
    }


    // only inform parents if change occurs and drawable doesn't already have an update callback
    if (delta_update!=0 && !_updateCallback)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringUpdateTraversal( (*itr)->getNumChildrenRequiringUpdateTraversal()+delta_update );
        }
    }

    // only inform parents if change occurs and drawable doesn't already have an event callback
    if (delta_event!=0 && !_eventCallback)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringEventTraversal( (*itr)->getNumChildrenRequiringEventTraversal()+delta_event );
        }
    }


}

void Drawable::setNumChildrenRequiringUpdateTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringUpdateTraversal==num) return;

    // note, if _updateCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringUpdateTraversal so no need to inform them.
    if (!_updateCallback && !_parents.empty())
    {
        // need to pass on changes to parents.
        int delta = 0;
        if (_numChildrenRequiringUpdateTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                (*itr)->setNumChildrenRequiringUpdateTraversal( (*itr)->getNumChildrenRequiringUpdateTraversal()+delta );
            }

        }
    }

    // finally update this objects value.
    _numChildrenRequiringUpdateTraversal=num;

}


void Drawable::setNumChildrenRequiringEventTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringEventTraversal==num) return;

    // note, if _eventCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringEventTraversal so no need to inform them.
    if (!_eventCallback && !_parents.empty())
    {
        // need to pass on changes to parents.
        int delta = 0;
        if (_numChildrenRequiringEventTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                (*itr)->setNumChildrenRequiringEventTraversal( (*itr)->getNumChildrenRequiringEventTraversal()+delta );
            }

        }
    }

    // finally Event this objects value.
    _numChildrenRequiringEventTraversal=num;

}

osg::StateSet* Drawable::getOrCreateStateSet()
{
    if (!_stateset) setStateSet(new StateSet);
    return _stateset.get();
}

void Drawable::dirtyBound()
{
    if (_boundingBoxComputed)
    {
        _boundingBoxComputed = false;

        // dirty parent bounding sphere's to ensure that all are valid.
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->dirtyBound();
        }

    }
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
    if (_updateCallback.valid()) _updateCallback->setThreadSafeRefUnref(threadSafe);
    if (_eventCallback.valid()) _eventCallback->setThreadSafeRefUnref(threadSafe);
    if (_cullCallback.valid()) _cullCallback->setThreadSafeRefUnref(threadSafe);
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


void Drawable::setUpdateCallback(UpdateCallback* ac)
{
    if (_updateCallback==ac) return;

    int delta = 0;
    if (_updateCallback.valid()) --delta;
    if (ac) ++delta;

    _updateCallback = ac;

    if (delta!=0 && !(_stateset.valid() && _stateset->requiresUpdateTraversal()))
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringUpdateTraversal((*itr)->getNumChildrenRequiringUpdateTraversal()+delta);
        }
    }
}

void Drawable::setEventCallback(EventCallback* ac)
{
    if (_eventCallback==ac) return;

    int delta = 0;
    if (_eventCallback.valid()) --delta;
    if (ac) ++delta;

    _eventCallback = ac;

    if (delta!=0 && !(_stateset.valid() && _stateset->requiresEventTraversal()))
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringEventTraversal( (*itr)->getNumChildrenRequiringEventTraversal()+delta );
        }
    }
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

BoundingBox Drawable::computeBound() const
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


//////////////////////////////////////////////////////////////////////////////
//
//  Extension support
//

typedef buffered_value< ref_ptr<Drawable::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Drawable::Extensions* Drawable::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Drawable::Extensions(contextID);
    return s_extensions[contextID].get();
}

void Drawable::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Drawable::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

Drawable::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isVertexProgramSupported = rhs._isVertexProgramSupported;
    _isSecondaryColorSupported = rhs._isSecondaryColorSupported;
    _isFogCoordSupported = rhs._isFogCoordSupported;
    _isMultiTexSupported = rhs._isMultiTexSupported;
    _isOcclusionQuerySupported = rhs._isOcclusionQuerySupported;
    _isARBOcclusionQuerySupported = rhs._isARBOcclusionQuerySupported;
    _isTimerQuerySupported = rhs._isTimerQuerySupported;
    _isARBTimerQuerySupported = rhs._isARBTimerQuerySupported;

    _glFogCoordfv = rhs._glFogCoordfv;
    _glFogCoorddv = rhs._glFogCoorddv;
    _glSecondaryColor3bv = rhs._glSecondaryColor3bv;
    _glSecondaryColor3ubv = rhs._glSecondaryColor3ubv;
    _glSecondaryColor3sv = rhs._glSecondaryColor3sv;
    _glSecondaryColor3fv = rhs._glSecondaryColor3fv;
    _glSecondaryColor3dv = rhs._glSecondaryColor3dv;
    _glMultiTexCoord1sv = rhs._glMultiTexCoord1sv;
    _glMultiTexCoord1iv = rhs._glMultiTexCoord1iv;
    _glMultiTexCoord1fv = rhs._glMultiTexCoord1fv;
    _glMultiTexCoord1dv = rhs._glMultiTexCoord1dv;
    _glMultiTexCoord2sv = rhs._glMultiTexCoord2sv;
    _glMultiTexCoord2fv = rhs._glMultiTexCoord2fv;
    _glMultiTexCoord2dv = rhs._glMultiTexCoord2dv;
    _glMultiTexCoord3sv = rhs._glMultiTexCoord3sv;
    _glMultiTexCoord3fv = rhs._glMultiTexCoord3fv;
    _glMultiTexCoord3dv = rhs._glMultiTexCoord3dv;
    _glMultiTexCoord4sv = rhs._glMultiTexCoord4sv;
    _glMultiTexCoord4fv = rhs._glMultiTexCoord4fv;
    _glMultiTexCoord4dv = rhs._glMultiTexCoord4dv;
    _glMultiTexCoord1f = rhs._glMultiTexCoord1f;
    _glMultiTexCoord1d = rhs._glMultiTexCoord1d;
    _glVertexAttrib1s = rhs._glVertexAttrib1s;
    _glVertexAttrib1f = rhs._glVertexAttrib1f;
    _glVertexAttrib1d = rhs._glVertexAttrib1d;
    _glVertexAttrib1sv = rhs._glVertexAttrib1sv;
    _glVertexAttribI1iv = rhs._glVertexAttribI1iv;
    _glVertexAttribI1uiv = rhs._glVertexAttribI1uiv;
    _glVertexAttrib1fv = rhs._glVertexAttrib1fv;
    _glVertexAttrib1dv = rhs._glVertexAttrib1dv;
    _glVertexAttrib2sv = rhs._glVertexAttrib2sv;
    _glVertexAttrib2fv = rhs._glVertexAttrib2fv;
    _glVertexAttrib2dv = rhs._glVertexAttrib2dv;
    _glVertexAttrib3sv = rhs._glVertexAttrib3sv;
    _glVertexAttrib3fv = rhs._glVertexAttrib3fv;
    _glVertexAttrib3dv = rhs._glVertexAttrib3dv;
    _glVertexAttrib4bv = rhs._glVertexAttrib4bv;
    _glVertexAttrib4Nbv = rhs._glVertexAttrib4Nbv;
    _glVertexAttrib4ubv = rhs._glVertexAttrib4ubv;
    _glVertexAttrib4Nubv = rhs._glVertexAttrib4Nubv;
    _glVertexAttrib4sv = rhs._glVertexAttrib4sv;
    _glVertexAttrib4Nsv = rhs._glVertexAttrib4Nsv;
    _glVertexAttrib4fv = rhs._glVertexAttrib4fv;
    _glVertexAttrib4dv = rhs._glVertexAttrib4dv;
    _glGenBuffers = rhs._glGenBuffers;
    _glBindBuffer = rhs._glBindBuffer;
    _glBufferData = rhs._glBufferData;
    _glBufferSubData = rhs._glBufferSubData;
    _glDeleteBuffers = rhs._glDeleteBuffers;
    _glGenOcclusionQueries = rhs._glGenOcclusionQueries;
    _glDeleteOcclusionQueries = rhs._glDeleteOcclusionQueries;
    _glIsOcclusionQuery = rhs._glIsOcclusionQuery;
    _glBeginOcclusionQuery = rhs._glBeginOcclusionQuery;
    _glEndOcclusionQuery = rhs._glEndOcclusionQuery;
    _glGetOcclusionQueryiv = rhs._glGetOcclusionQueryiv;
    _glGetOcclusionQueryuiv = rhs._glGetOcclusionQueryuiv;
    _gl_gen_queries_arb = rhs._gl_gen_queries_arb;
    _gl_delete_queries_arb = rhs._gl_delete_queries_arb;
    _gl_is_query_arb = rhs._gl_is_query_arb;
    _gl_begin_query_arb = rhs._gl_begin_query_arb;
    _gl_end_query_arb = rhs._gl_end_query_arb;
    _gl_get_queryiv_arb = rhs._gl_get_queryiv_arb;
    _gl_get_query_objectiv_arb = rhs._gl_get_query_objectiv_arb;
    _gl_get_query_objectuiv_arb = rhs._gl_get_query_objectuiv_arb;
    _gl_get_query_objectui64v = rhs._gl_get_query_objectui64v;
    _glGetInteger64v = rhs._glGetInteger64v;
}


void Drawable::Extensions::lowestCommonDenominator(const Extensions& rhs)    // Necessary method ? Never seen called
{
    if (!rhs._isVertexProgramSupported) _isVertexProgramSupported = false;
    if (!rhs._isSecondaryColorSupported) _isSecondaryColorSupported = false;
    if (!rhs._isFogCoordSupported) _isFogCoordSupported = false;
    if (!rhs._isMultiTexSupported) _isMultiTexSupported = false;
    if (!rhs._isOcclusionQuerySupported) _isOcclusionQuerySupported = false;
    if (!rhs._isARBOcclusionQuerySupported) _isARBOcclusionQuerySupported = false;

    if (!rhs._isTimerQuerySupported) _isTimerQuerySupported = false;
    if (!rhs._isARBTimerQuerySupported) _isARBTimerQuerySupported = false;

    if (!rhs._glFogCoordfv) _glFogCoordfv = 0;
    if (!rhs._glFogCoorddv) _glFogCoorddv = 0;
    if (!rhs._glSecondaryColor3bv) _glSecondaryColor3bv = 0;
    if (!rhs._glSecondaryColor3ubv) _glSecondaryColor3ubv = 0;
    if (!rhs._glSecondaryColor3sv) _glSecondaryColor3sv = 0;
    if (!rhs._glSecondaryColor3fv) _glSecondaryColor3fv = 0;
    if (!rhs._glSecondaryColor3dv) _glSecondaryColor3dv = 0;

    if (!rhs._glMultiTexCoord1sv) _glMultiTexCoord1sv = 0;
    if (!rhs._glMultiTexCoord1iv) _glMultiTexCoord1iv = 0;
    if (!rhs._glMultiTexCoord1fv) _glMultiTexCoord1fv = 0;
    if (!rhs._glMultiTexCoord1dv) _glMultiTexCoord1dv = 0;
    if (!rhs._glMultiTexCoord2sv) _glMultiTexCoord2sv = 0;
    if (!rhs._glMultiTexCoord2fv) _glMultiTexCoord2fv = 0;
    if (!rhs._glMultiTexCoord2dv) _glMultiTexCoord2dv = 0;
    if (!rhs._glMultiTexCoord3sv) _glMultiTexCoord3sv = 0;
    if (!rhs._glMultiTexCoord3fv) _glMultiTexCoord3fv = 0;
    if (!rhs._glMultiTexCoord3dv) _glMultiTexCoord3dv = 0;
    if (!rhs._glMultiTexCoord4sv) _glMultiTexCoord4sv = 0;
    if (!rhs._glMultiTexCoord4fv) _glMultiTexCoord4fv = 0;
    if (!rhs._glMultiTexCoord4dv) _glMultiTexCoord4dv = 0;
    if (!rhs._glMultiTexCoord1f) _glMultiTexCoord1f = 0;
    if (!rhs._glMultiTexCoord1d) _glMultiTexCoord1d = 0;
    if (!rhs._glVertexAttrib1s) _glVertexAttrib1s = 0;
    if (!rhs._glVertexAttrib1f) _glVertexAttrib1f = 0;
    if (!rhs._glVertexAttrib1d) _glVertexAttrib1d = 0;
    if (!rhs._glVertexAttrib1sv) _glVertexAttrib1sv = 0;
    if (!rhs._glVertexAttribI1iv) _glVertexAttribI1iv = 0;
    if (!rhs._glVertexAttribI1uiv) _glVertexAttribI1uiv = 0;
    if (!rhs._glVertexAttrib1fv) _glVertexAttrib1fv = 0;
    if (!rhs._glVertexAttrib1dv) _glVertexAttrib1dv = 0;
    if (!rhs._glVertexAttrib2sv) _glVertexAttrib2sv = 0;
    if (!rhs._glVertexAttrib2fv) _glVertexAttrib2fv = 0;
    if (!rhs._glVertexAttrib2dv) _glVertexAttrib2dv = 0;
    if (!rhs._glVertexAttrib3sv) _glVertexAttrib3sv = 0;
    if (!rhs._glVertexAttrib3fv) _glVertexAttrib3fv = 0;
    if (!rhs._glVertexAttrib3dv) _glVertexAttrib3dv = 0;
    if (!rhs._glVertexAttrib4bv) _glVertexAttrib4bv = 0;
    if (!rhs._glVertexAttrib4Nbv) _glVertexAttrib4Nbv = 0;
    if (!rhs._glVertexAttrib4ubv) _glVertexAttrib4ubv = 0;
    if (!rhs._glVertexAttrib4Nubv) _glVertexAttrib4Nubv = 0;
    if (!rhs._glVertexAttrib4sv) _glVertexAttrib4sv = 0;
    if (!rhs._glVertexAttrib4Nsv) _glVertexAttrib4Nsv = 0;
    if (!rhs._glVertexAttrib4fv) _glVertexAttrib4fv = 0;
    if (!rhs._glVertexAttrib4dv) _glVertexAttrib4dv = 0;
    if (!rhs._glGenBuffers) _glGenBuffers = 0;
    if (!rhs._glBindBuffer) _glBindBuffer = 0;
    if (!rhs._glBufferData) _glBufferData = 0;
    if (!rhs._glBufferSubData) _glBufferSubData = 0;
    if (!rhs._glDeleteBuffers) _glDeleteBuffers = 0;
    if (!rhs._glIsBuffer) _glIsBuffer = 0;
    if (!rhs._glGetBufferSubData) _glGetBufferSubData = 0;
    if (!rhs._glMapBuffer) _glMapBuffer = 0;
    if (!rhs._glUnmapBuffer) _glUnmapBuffer = 0;
    if (!rhs._glGetBufferParameteriv) _glGetBufferParameteriv = 0;
    if (!rhs._glGetBufferPointerv) _glGetBufferPointerv = 0;

    if (!rhs._glGenOcclusionQueries) _glGenOcclusionQueries = 0;
    if (!rhs._glDeleteOcclusionQueries) _glDeleteOcclusionQueries = 0;
    if (!rhs._glIsOcclusionQuery) _glIsOcclusionQuery = 0;
    if (!rhs._glBeginOcclusionQuery) _glBeginOcclusionQuery = 0;
    if (!rhs._glEndOcclusionQuery) _glEndOcclusionQuery = 0;
    if (!rhs._glGetOcclusionQueryiv) _glGetOcclusionQueryiv = 0;
    if (!rhs._glGetOcclusionQueryuiv) _glGetOcclusionQueryuiv = 0;

    if (!rhs._gl_gen_queries_arb) _gl_gen_queries_arb = 0;
    if (!rhs._gl_delete_queries_arb) _gl_delete_queries_arb = 0;
    if (!rhs._gl_is_query_arb) _gl_is_query_arb = 0;
    if (!rhs._gl_begin_query_arb) _gl_begin_query_arb = 0;
    if (!rhs._gl_end_query_arb) _gl_end_query_arb = 0;
    if (!rhs._gl_get_queryiv_arb) _gl_get_queryiv_arb = 0;
    if (!rhs._gl_get_query_objectiv_arb) _gl_get_query_objectiv_arb = 0;
    if (!rhs._gl_get_query_objectuiv_arb) _gl_get_query_objectuiv_arb = 0;
    if (!rhs._gl_get_query_objectui64v) _gl_get_query_objectui64v = 0;
    if (!rhs._glGetInteger64v) _glGetInteger64v = 0;
}

void Drawable::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isVertexProgramSupported = isGLExtensionSupported(contextID,"GL_ARB_vertex_program");
    _isSecondaryColorSupported = isGLExtensionSupported(contextID,"GL_EXT_secondary_color");
    _isFogCoordSupported = isGLExtensionSupported(contextID,"GL_EXT_fog_coord");
    _isMultiTexSupported = isGLExtensionSupported(contextID,"GL_ARB_multitexture");
    _isOcclusionQuerySupported = osg::isGLExtensionSupported(contextID, "GL_NV_occlusion_query" );
    _isARBOcclusionQuerySupported = OSG_GL3_FEATURES || osg::isGLExtensionSupported(contextID, "GL_ARB_occlusion_query" );

    _isTimerQuerySupported = osg::isGLExtensionSupported(contextID, "GL_EXT_timer_query" );
    _isARBTimerQuerySupported = osg::isGLExtensionSupported(contextID, "GL_ARB_timer_query");


    setGLExtensionFuncPtr(_glFogCoordfv, "glFogCoordfv","glFogCoordfvEXT");
    setGLExtensionFuncPtr(_glFogCoorddv, "glFogCoorddv","glFogCoorddvEXT");
    setGLExtensionFuncPtr(_glSecondaryColor3bv, "glSecondaryColor3bv","glSecondaryColor3bvEXT");
    setGLExtensionFuncPtr(_glSecondaryColor3ubv, "glSecondaryColor3ubv","glSecondaryColor3ubvEXT");
    setGLExtensionFuncPtr(_glSecondaryColor3sv, "glSecondaryColor3sv","glSecondaryColor3svEXT");
    setGLExtensionFuncPtr(_glSecondaryColor3fv, "glSecondaryColor3fv","glSecondaryColor3fvEXT");
    setGLExtensionFuncPtr(_glSecondaryColor3dv, "glSecondaryColor3dv","glSecondaryColor3dvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord1sv, "glMultiTexCoord1sv","glMultiTexCoord1svEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord1iv, "glMultiTexCoord1iv","glMultiTexCoord1ivEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord1fv, "glMultiTexCoord1fv","glMultiTexCoord1fvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord1dv, "glMultiTexCoord1dv","glMultiTexCoord1dvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord2sv, "glMultiTexCoord2sv","glMultiTexCoord2svEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord2fv, "glMultiTexCoord2fv","glMultiTexCoord2fvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord2dv, "glMultiTexCoord2dv","glMultiTexCoord2dvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord3sv, "glMultiTexCoord3sv","glMultiTexCoord3svEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord3fv, "glMultiTexCoord3fv","glMultiTexCoord3fvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord3dv, "glMultiTexCoord3dv","glMultiTexCoord3dvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord4sv, "glMultiTexCoord4sv","glMultiTexCoord4svEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord4fv, "glMultiTexCoord4fv","glMultiTexCoord4fvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord4dv, "glMultiTexCoord4dv","glMultiTexCoord4dvEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord1f, "glMultiTexCoord1f","glMultiTexCoord1fEXT");
    setGLExtensionFuncPtr(_glMultiTexCoord1d, "glMultiTexCoord1d","glMultiTexCoord1dEXT");

    setGLExtensionFuncPtr(_glVertexAttrib1s, "glVertexAttrib1s","glVertexAttrib1sARB");
    setGLExtensionFuncPtr(_glVertexAttrib1f, "glVertexAttrib1f","glVertexAttrib1fARB");
    setGLExtensionFuncPtr(_glVertexAttrib1d, "glVertexAttrib1d","glVertexAttrib1dARB");
    setGLExtensionFuncPtr(_glVertexAttrib1sv, "glVertexAttrib1sv","glVertexAttrib1svARB");
    setGLExtensionFuncPtr(_glVertexAttribI1iv, "glVertexAttribI1iv","glVertexAttribI1ivARB");
    setGLExtensionFuncPtr(_glVertexAttribI1uiv, "glVertexAttribI1uiv","glVertexAttribI1uivARB");
    setGLExtensionFuncPtr(_glVertexAttrib1fv, "glVertexAttrib1fv","glVertexAttrib1fvARB");
    setGLExtensionFuncPtr(_glVertexAttrib1dv, "glVertexAttrib1dv","glVertexAttrib1dvARB");
    setGLExtensionFuncPtr(_glVertexAttrib2sv, "glVertexAttrib2sv","glVertexAttrib2svARB");
    setGLExtensionFuncPtr(_glVertexAttrib2fv, "glVertexAttrib2fv","glVertexAttrib2fvARB");
    setGLExtensionFuncPtr(_glVertexAttrib2dv, "glVertexAttrib2dv","glVertexAttrib2dvARB");
    setGLExtensionFuncPtr(_glVertexAttrib3sv, "glVertexAttrib3sv","glVertexAttrib3svARB");
    setGLExtensionFuncPtr(_glVertexAttrib3fv, "glVertexAttrib3fv","glVertexAttrib3fvARB");
    setGLExtensionFuncPtr(_glVertexAttrib3dv, "glVertexAttrib3dv","glVertexAttrib3dvARB");
    setGLExtensionFuncPtr(_glVertexAttrib4bv, "glVertexAttrib4bv","glVertexAttrib4bvARB");
    setGLExtensionFuncPtr(_glVertexAttrib4Nbv, "glVertexAttrib4Nbv","glVertexAttrib4NbvARB");
    setGLExtensionFuncPtr(_glVertexAttrib4ubv, "glVertexAttrib4ubv","glVertexAttrib4ubvARB");
    setGLExtensionFuncPtr(_glVertexAttrib4Nubv, "glVertexAttrib4Nubv","glVertexAttrib4NubvARB");
    setGLExtensionFuncPtr(_glVertexAttrib4sv, "glVertexAttrib4sv","glVertexAttrib4svARB");
    setGLExtensionFuncPtr(_glVertexAttrib4Nsv, "glVertexAttrib4Nsv","glVertexAttrib4NsvARB");
    setGLExtensionFuncPtr(_glVertexAttrib4fv, "glVertexAttrib4fv","glVertexAttrib4fvARB");
    setGLExtensionFuncPtr(_glVertexAttrib4dv, "glVertexAttrib4dv","glVertexAttrib4dvARB");

    setGLExtensionFuncPtr(_glGenBuffers, "glGenBuffers","glGenBuffersARB");
    setGLExtensionFuncPtr(_glBindBuffer, "glBindBuffer","glBindBufferARB");
    setGLExtensionFuncPtr(_glBufferData, "glBufferData","glBufferDataARB");
    setGLExtensionFuncPtr(_glBufferSubData, "glBufferSubData","glBufferSubDataARB");
    setGLExtensionFuncPtr(_glDeleteBuffers, "glDeleteBuffers","glDeleteBuffersARB");
    setGLExtensionFuncPtr(_glIsBuffer, "glIsBuffer","glIsBufferARB");
    setGLExtensionFuncPtr(_glGetBufferSubData, "glGetBufferSubData","glGetBufferSubDataARB");
    setGLExtensionFuncPtr(_glMapBuffer, "glMapBuffer","glMapBufferARB");
    setGLExtensionFuncPtr(_glUnmapBuffer, "glUnmapBuffer","glUnmapBufferARB");
    setGLExtensionFuncPtr(_glGetBufferParameteriv, "glGetBufferParameteriv","glGetBufferParameterivARB");
    setGLExtensionFuncPtr(_glGetBufferPointerv, "glGetBufferPointerv","glGetBufferPointervARB");

    setGLExtensionFuncPtr(_glGenOcclusionQueries, "glGenOcclusionQueries","glGenOcclusionQueriesNV");
    setGLExtensionFuncPtr(_glDeleteOcclusionQueries, "glDeleteOcclusionQueries","glDeleteOcclusionQueriesNV");
    setGLExtensionFuncPtr(_glIsOcclusionQuery, "glIsOcclusionQuery","_glIsOcclusionQueryNV");
    setGLExtensionFuncPtr(_glBeginOcclusionQuery, "glBeginOcclusionQuery","glBeginOcclusionQueryNV");
    setGLExtensionFuncPtr(_glEndOcclusionQuery, "glEndOcclusionQuery","glEndOcclusionQueryNV");
    setGLExtensionFuncPtr(_glGetOcclusionQueryiv, "glGetOcclusionQueryiv","glGetOcclusionQueryivNV");
    setGLExtensionFuncPtr(_glGetOcclusionQueryuiv, "glGetOcclusionQueryuiv","glGetOcclusionQueryuivNV");

    setGLExtensionFuncPtr(_gl_gen_queries_arb, "glGenQueries", "glGenQueriesARB");
    setGLExtensionFuncPtr(_gl_delete_queries_arb, "glDeleteQueries", "glDeleteQueriesARB");
    setGLExtensionFuncPtr(_gl_is_query_arb, "glIsQuery", "glIsQueryARB");
    setGLExtensionFuncPtr(_gl_begin_query_arb, "glBeginQuery", "glBeginQueryARB");
    setGLExtensionFuncPtr(_gl_end_query_arb, "glEndQuery", "glEndQueryARB");
    setGLExtensionFuncPtr(_gl_get_queryiv_arb, "glGetQueryiv", "glGetQueryivARB");
    setGLExtensionFuncPtr(_gl_get_query_objectiv_arb, "glGetQueryObjectiv","glGetQueryObjectivARB");
    setGLExtensionFuncPtr(_gl_get_query_objectuiv_arb, "glGetQueryObjectuiv","glGetQueryObjectuivARB");
    setGLExtensionFuncPtr(_gl_get_query_objectui64v, "glGetQueryObjectui64v","glGetQueryObjectui64vEXT");
    setGLExtensionFuncPtr(_glQueryCounter, "glQueryCounter");
    setGLExtensionFuncPtr(_glGetInteger64v, "glGetInteger64v");
}

void Drawable::Extensions::glFogCoordfv(const GLfloat* coord) const
{
    if (_glFogCoordfv)
    {
        _glFogCoordfv(coord);
    }
    else
    {
        OSG_WARN<<"Error: glFogCoordfv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glFogCoorddv(const GLdouble* coord) const
{
    if (_glFogCoorddv)
    {
        _glFogCoorddv(coord);
    }
    else
    {
        OSG_WARN<<"Error: glFogCoorddv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glSecondaryColor3bv(const GLbyte* coord) const
{
    if (_glSecondaryColor3bv)
    {
        _glSecondaryColor3bv(coord);
    }
    else
    {
        OSG_WARN<<"Error: glSecondaryColor3bv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glSecondaryColor3ubv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glSecondaryColor3sv(const GLshort* coord) const
{
    if (_glSecondaryColor3sv)
    {
        _glSecondaryColor3sv(coord);
    }
    else
    {
        OSG_WARN<<"Error: glSecondaryColor3sv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glSecondaryColor3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glSecondaryColor3dv(const GLdouble* coord) const
{
    if (_glSecondaryColor3dv)
    {
        _glSecondaryColor3dv(coord);
    }
    else
    {
        OSG_WARN<<"Error: glSecondaryColor3dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord1sv(GLenum target,const GLshort* coord) const
{
    if (_glMultiTexCoord1sv)
    {
        _glMultiTexCoord1sv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord1sv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord1iv(GLenum target,const GLint* coord) const
{
    if (_glMultiTexCoord1iv)
    {
        _glMultiTexCoord1iv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord1iv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord1fv(GLenum target,const GLfloat* coord) const
{
    if (_glMultiTexCoord1fv)
    {
        _glMultiTexCoord1fv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord1fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord1dv(GLenum target,const GLdouble* coord) const
{
    if (_glMultiTexCoord1dv)
    {
        _glMultiTexCoord1dv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord1dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord2sv(GLenum target,const GLshort* coord) const
{
    if (_glMultiTexCoord2sv)
    {
        _glMultiTexCoord2sv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord2sv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glMultiTexCoord2fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord2dv(GLenum target,const GLdouble* coord) const
{
    if (_glMultiTexCoord2dv)
    {
        _glMultiTexCoord2dv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord2dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord3sv(GLenum target,const GLshort* coord) const
{
    if (_glMultiTexCoord3sv)
    {
        _glMultiTexCoord3sv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord3sv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glMultiTexCoord3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord3dv(GLenum target,const GLdouble* coord) const
{
    if (_glMultiTexCoord3dv)
    {
        _glMultiTexCoord3dv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord3dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord4sv(GLenum target,const GLshort* coord) const
{
    if (_glMultiTexCoord4sv)
    {
        _glMultiTexCoord4sv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord4sv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glMultiTexCoord4fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord4dv(GLenum target,const GLdouble* coord) const
{
    if (_glMultiTexCoord4dv)
    {
        _glMultiTexCoord4dv(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord4dv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glMultiTexCoord1f not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glMultiTexCoord1d(GLenum target,GLdouble coord) const
{
    if (_glMultiTexCoord1d)
    {
        _glMultiTexCoord1d(target,coord);
    }
    else
    {
        OSG_WARN<<"Error: glMultiTexCoord1d not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glVertexAttrib1s not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glVertexAttrib1f not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib1d(unsigned int index, GLdouble f) const
{
    if (_glVertexAttrib1d)
    {
        _glVertexAttrib1d(index,f);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib1d not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib1sv(unsigned int index, const GLshort * v) const
{
    if (_glVertexAttrib1sv)
    {
        _glVertexAttrib1sv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib1sv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttribI1iv(unsigned int index, const GLint * v) const
{
    if (_glVertexAttribI1iv)
    {
        _glVertexAttribI1iv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttribI1iv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttribI1uiv(unsigned int index, const GLuint * v) const
{
    if (_glVertexAttribI1uiv)
    {
        _glVertexAttribI1uiv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttribI1uiv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib1fv(unsigned int index, const GLfloat * v) const
{
    if (_glVertexAttrib1fv)
    {
        _glVertexAttrib1fv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib1fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib1dv(unsigned int index, const GLdouble * v) const
{
    if (_glVertexAttrib1dv)
    {
        _glVertexAttrib1dv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib1dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib2sv(unsigned int index, const GLshort * v) const
{
    if (_glVertexAttrib2sv)
    {
        _glVertexAttrib2sv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib2sv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glVertexAttrib2fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib2dv(unsigned int index, const GLdouble * v) const
{
    if (_glVertexAttrib2dv)
    {
        _glVertexAttrib2dv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib2dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib3sv(unsigned int index, const GLshort * v) const
{
    if (_glVertexAttrib3sv)
    {
        _glVertexAttrib3sv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib3sv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glVertexAttrib3fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib3dv(unsigned int index, const GLdouble * v) const
{
    if (_glVertexAttrib3dv)
    {
        _glVertexAttrib3dv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib3dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4bv(unsigned int index, const GLbyte * v) const
{
    if (_glVertexAttrib4bv)
    {
        _glVertexAttrib4bv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib4bv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4Nbv(unsigned int index, const GLbyte * v) const
{
    if (_glVertexAttrib4Nbv)
    {
        _glVertexAttrib4Nbv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib4Nbv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glVertexAttrib4ubv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glVertexAttrib4Nubv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4sv(unsigned int index, const GLshort * v) const
{
    if (_glVertexAttrib4sv)
    {
        _glVertexAttrib4sv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib4sv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4Nsv(unsigned int index, const GLshort * v) const
{
    if (_glVertexAttrib4Nsv)
    {
        _glVertexAttrib4Nsv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib4Nsv not supported by OpenGL driver"<<std::endl;
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
        OSG_WARN<<"Error: glVertexAttrib4fv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glVertexAttrib4dv(unsigned int index, const GLdouble * v) const
{
    if (_glVertexAttrib4dv)
    {
        _glVertexAttrib4dv(index,v);
    }
    else
    {
        OSG_WARN<<"Error: glVertexAttrib4dv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glGenBuffers(GLsizei n, GLuint *buffers) const
{
    if (_glGenBuffers) _glGenBuffers(n, buffers);
    else OSG_WARN<<"Error: glGenBuffers not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glBindBuffer(GLenum target, GLuint buffer) const
{
    if (_glBindBuffer) _glBindBuffer(target, buffer);
    else OSG_WARN<<"Error: glBindBuffer not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glBufferData(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage) const
{
    if (_glBufferData) _glBufferData(target, size, data, usage);
    else OSG_WARN<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glBufferSubData(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data) const
{
    if (_glBufferSubData) _glBufferSubData(target, offset, size, data);
    else OSG_WARN<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glDeleteBuffers(GLsizei n, const GLuint *buffers) const
{
    if (_glDeleteBuffers) _glDeleteBuffers(n, buffers);
    else OSG_WARN<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

GLboolean Drawable::Extensions::glIsBuffer (GLuint buffer) const
{
    if (_glIsBuffer) return _glIsBuffer(buffer);
    else
    {
        OSG_WARN<<"Error: glIsBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void Drawable::Extensions::glGetBufferSubData (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data) const
{
    if (_glGetBufferSubData) _glGetBufferSubData(target,offset,size,data);
    else OSG_WARN<<"Error: glGetBufferSubData not supported by OpenGL driver"<<std::endl;
}

GLvoid* Drawable::Extensions::glMapBuffer (GLenum target, GLenum access) const
{
    if (_glMapBuffer) return _glMapBuffer(target,access);
    else
    {
        OSG_WARN<<"Error: glMapBuffer not supported by OpenGL driver"<<std::endl;
        return 0;
    }
}

GLboolean Drawable::Extensions::glUnmapBuffer (GLenum target) const
{
    if (_glUnmapBuffer) return _glUnmapBuffer(target);
    else
    {
        OSG_WARN<<"Error: glUnmapBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void Drawable::Extensions::glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) const
{
    if (_glGetBufferParameteriv) _glGetBufferParameteriv(target,pname,params);
    else OSG_WARN<<"Error: glGetBufferParameteriv not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params) const
{
    if (_glGetBufferPointerv) _glGetBufferPointerv(target,pname,params);
    else OSG_WARN<<"Error: glGetBufferPointerv not supported by OpenGL driver"<<std::endl;
}


void Drawable::Extensions::glGenOcclusionQueries( GLsizei n, GLuint *ids ) const
{
    if (_glGenOcclusionQueries)
    {
        _glGenOcclusionQueries( n, ids );
    }
    else
    {
        OSG_WARN<<"Error: glGenOcclusionQueries not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glDeleteOcclusionQueries( GLsizei n, const GLuint *ids ) const
{
    if (_glDeleteOcclusionQueries)
    {
        _glDeleteOcclusionQueries( n, ids );
    }
    else
    {
        OSG_WARN<<"Error: glDeleteOcclusionQueries not supported by OpenGL driver"<<std::endl;
    }
}

GLboolean Drawable::Extensions::glIsOcclusionQuery( GLuint id ) const
{
    if (_glIsOcclusionQuery)
    {
        return _glIsOcclusionQuery( id );
    }
    else
    {
        OSG_WARN<<"Error: glIsOcclusionQuery not supported by OpenGL driver"<<std::endl;
    }

    return GLboolean( 0 );
}

void Drawable::Extensions::glBeginOcclusionQuery( GLuint id ) const
{
    if (_glBeginOcclusionQuery)
    {
        _glBeginOcclusionQuery( id );
    }
    else
    {
        OSG_WARN<<"Error: glBeginOcclusionQuery not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glEndOcclusionQuery() const
{
    if (_glEndOcclusionQuery)
    {
        _glEndOcclusionQuery();
    }
    else
    {
        OSG_WARN<<"Error: glEndOcclusionQuery not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glGetOcclusionQueryiv( GLuint id, GLenum pname, GLint *params ) const
{
    if (_glGetOcclusionQueryiv)
    {
        _glGetOcclusionQueryiv( id, pname, params );
    }
    else
    {
        OSG_WARN<<"Error: glGetOcclusionQueryiv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glGetOcclusionQueryuiv( GLuint id, GLenum pname, GLuint *params ) const
{
    if (_glGetOcclusionQueryuiv)
    {
        _glGetOcclusionQueryuiv( id, pname, params );
    }
    else
    {
        OSG_WARN<<"Error: glGetOcclusionQueryuiv not supported by OpenGL driver"<<std::endl;
    }
}

void Drawable::Extensions::glGetQueryiv(GLenum target, GLenum pname, GLint *params) const
{
  if (_gl_get_queryiv_arb)
    _gl_get_queryiv_arb(target, pname, params);
  else
    OSG_WARN << "Error: glGetQueryiv not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGenQueries(GLsizei n, GLuint *ids) const
{
  if (_gl_gen_queries_arb)
    _gl_gen_queries_arb(n, ids);
  else
    OSG_WARN << "Error: glGenQueries not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glBeginQuery(GLenum target, GLuint id) const
{
  if (_gl_begin_query_arb)
    _gl_begin_query_arb(target, id);
  else
    OSG_WARN << "Error: glBeginQuery not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glEndQuery(GLenum target) const
{
  if (_gl_end_query_arb)
    _gl_end_query_arb(target);
  else
    OSG_WARN << "Error: glEndQuery not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glQueryCounter(GLuint id, GLenum target) const
{
    if (_glQueryCounter)
        _glQueryCounter(id, target);
    else
        OSG_WARN << "Error: glQueryCounter not supported by OpenGL driver\n";
}

GLboolean Drawable::Extensions::glIsQuery(GLuint id) const
{
  if (_gl_is_query_arb) return _gl_is_query_arb(id);

  OSG_WARN << "Error: glIsQuery not supported by OpenGL driver" << std::endl;
  return false;
}

void Drawable::Extensions::glDeleteQueries(GLsizei n, const GLuint *ids) const
{
    if (_gl_delete_queries_arb)
        _gl_delete_queries_arb(n, ids);
    else
        OSG_WARN << "Error: glIsQuery not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params) const
{
  if (_gl_get_query_objectiv_arb)
    _gl_get_query_objectiv_arb(id, pname, params);
  else
    OSG_WARN << "Error: glGetQueryObjectiv not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) const
{
  if (_gl_get_query_objectuiv_arb)
    _gl_get_query_objectuiv_arb(id, pname, params);
  else
    OSG_WARN << "Error: glGetQueryObjectuiv not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64EXT *params) const
{
  if (_gl_get_query_objectui64v)
    _gl_get_query_objectui64v(id, pname, params);
  else
    OSG_WARN << "Error: glGetQueryObjectui64v not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGetInteger64v(GLenum pname, GLint64EXT *params)
    const
{
    if (_glGetInteger64v)
        _glGetInteger64v(pname, params);
    else
        OSG_WARN << "Error: glGetInteger64v not supported by OpenGL driver\n";
}
