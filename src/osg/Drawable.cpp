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
            // osg::notify(osg::NOTICE)<<"Reusing a display list of size = "<<itr->first<<" for requested size = "<<sizeHint<<std::endl;

            ++s_numberDrawablesReusedLastInLastFrame;
            
            GLuint globj = itr->second;
            dll.erase(itr);
            
            return globj;
        } 
        else
        {
            // osg::notify(osg::NOTICE)<<"Creating a new display list of size = "<<sizeHint<<" although "<<dll.size()<<" are available"<<std::endl;
            ++s_numberNewDrawablesInLastFrame;
            return  glGenLists( 1 );
        }
    }
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
    if (globj!=0)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedDisplayListCache);

        // insert the globj into the cache for the appropriate context.
        s_deletedDisplayListCache[contextID].insert(DisplayListMap::value_type(sizeHint,globj));
    }
}

void Drawable::flushAllDeletedDisplayLists(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedDisplayListCache);

    DisplayListMap& dll = s_deletedDisplayListCache[contextID];

    for(DisplayListMap::iterator ditr=dll.begin();
        ditr!=dll.end();
        ++ditr)
    {
        glDeleteLists(ditr->second,1);
    }

    dll.clear();         
}

void Drawable::flushDeletedDisplayLists(unsigned int contextID, double& availableTime)
{
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
                osg::notify(osg::WARN)<<"Error in delete"<<std::endl;
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
                osg::notify(osg::WARN)<<"Error in delete"<<std::endl;
             }    
        }
    }
    elapsedTime = timer.delta_s(start_tick,timer.tick());
    
    // if (noDeleted) notify(NOTICE)<<"Number display lists deleted = "<<noDeleted<<" elapsed time"<<elapsedTime<<std::endl;

    availableTime -= elapsedTime;
}


static OpenThreads::Mutex s_mutex_deletedVertexBufferObjectCache;
static DeletedDisplayListCache s_deletedVertexBufferObjectCache;

void Drawable::deleteVertexBufferObject(unsigned int contextID,GLuint globj)
{
    if (globj!=0)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedVertexBufferObjectCache);
        
        // insert the globj into the cache for the appropriate context.
        s_deletedVertexBufferObjectCache[contextID].insert(DisplayListMap::value_type(0,globj));
    }
}

/** flush all the cached display list which need to be deleted
  * in the OpenGL context related to contextID.*/
void Drawable::flushDeletedVertexBufferObjects(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;


    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedVertexBufferObjectCache);

        const Extensions* extensions = getExtensions(contextID,true);

        unsigned int noDeleted = 0;

        DisplayListMap& dll = s_deletedVertexBufferObjectCache[contextID];

        DisplayListMap::iterator ditr=dll.begin();
        for(;
            ditr!=dll.end() && elapsedTime<availableTime;
            ++ditr)
        {
            extensions->glDeleteBuffers(1,&(ditr->second));
            elapsedTime = timer.delta_s(start_tick,timer.tick());
            ++noDeleted;
        }
        if (ditr!=dll.begin()) dll.erase(dll.begin(),ditr);

        if (noDeleted!=0) notify(osg::INFO)<<"Number VBOs deleted = "<<noDeleted<<std::endl;
    }    
    
    availableTime -= elapsedTime;
}


Drawable::Drawable()
{
    _boundingBoxComputed = false;

    // Note, if your are defining a subclass from drawable which is
    // dynamically updated then you should set both the following to
    // to false in your constructor.  This will prevent any display
    // lists from being automatically created and safeguard the
    // dynamic updating of data.
    _supportsDisplayList = true;
    _useDisplayList = true;

    _supportsVertexBufferObjects = false;
    _useVertexBufferObjects = false;

    _numChildrenRequiringUpdateTraversal = 0;
    _numChildrenRequiringEventTraversal = 0;
}

Drawable::Drawable(const Drawable& drawable,const CopyOp& copyop):
    Object(drawable,copyop),
    _parents(), // leave empty as parentList is managed by Geode
    _stateset(copyop(drawable._stateset.get())),
    _initialBound(drawable._initialBound),
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
}

Drawable::~Drawable()
{
    // cleanly detatch any associated stateset (include remove parent links)
    setStateSet(0);

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
            // reqired on this subgraph.
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
            // reqired on this subgraph.
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

void Drawable::compileGLObjects(State& state) const
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

    globj = generateDisplayList(contextID, getGLObjectSizeHint());
    glNewList( globj, GL_COMPILE );

    if (_drawCallback.valid())
        _drawCallback->drawImplementation(state,this);
    else 
        drawImplementation(state);

    glEndList();

}

void Drawable::releaseGLObjects(State* state) const
{
    if (_stateset.valid()) _stateset->releaseGLObjects(state);

    if (!_useDisplayList) return;
    
    if (state)
    {
        // get the contextID (user defined ID of 0 upwards) for the 
        // current OpenGL context.
        unsigned int contextID = state->getContextID();

        // get the globj for the current contextID.
        GLuint& globj = _globjList[contextID];

        // call the globj if already set otherwise comple and execute.
        if( globj != 0 )
        {
            glDeleteLists( globj, 1 );
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


void Drawable::setUseVertexBufferObjects(bool flag)
{
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
    unsigned int i;
    for(i=0;i<_globjList.size();++i)
    {
        if (_globjList[i] != 0)
        {
            Drawable::deleteDisplayList(i,_globjList[i], getGLObjectSizeHint());
            _globjList[i] = 0;
        }
    }

    for(i=0;i<_vboList.size();++i)
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
   ComputeBound()  {   _vertices = 0;  _vertices4 = 0;}

        virtual void setVertexArray(unsigned int,const Vec2*)
        {
            notify(WARN)<<"ComputeBound does not support Vec2* vertex arrays"<<std::endl;
        }

        virtual void setVertexArray(unsigned int,const Vec3* vertices) { _vertices  = vertices; }
        virtual void setVertexArray(unsigned int,const Vec4* vertices) { _vertices4 = vertices; }

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

            if (_vertices4)
            {
                const osg::Vec4* vert = _vertices4+first;
                for(;count>0;--count,++vert)
                {
                    _bb.expandBy(*((Vec3*) vert));
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

            if (_vertices4)
            {
                for(;count>0;--count,++indices)
                {
                    _bb.expandBy(*((Vec3*)&_vertices4[*indices]));
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

            if (_vertices4)
            {
                for(;count>0;--count,++indices)
                {
                    _bb.expandBy(*((Vec3*)&_vertices4[*indices]));
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

            if (_vertices4)
            {
                for(;count>0;--count,++indices)
                {
                    _bb.expandBy(*((Vec3*)&_vertices4[*indices]));
                }
            }
        }

        virtual void begin(GLenum) {}
        virtual void vertex(const Vec2& vert) { _bb.expandBy(osg::Vec3(vert[0],vert[1],0.0f)); }
        virtual void vertex(const Vec3& vert) { _bb.expandBy(vert); }
        virtual void vertex(const Vec4& vert) { if (vert[3]!=0.0f) _bb.expandBy(osg::Vec3(vert[0],vert[1],vert[2])/vert[3]); }
        virtual void vertex(float x,float y)  { _bb.expandBy(x,y,1.0f); }
        virtual void vertex(float x,float y,float z) { _bb.expandBy(x,y,z); }
        virtual void vertex(float x,float y,float z,float w) { if (w!=0.0f) _bb.expandBy(x/w,y/w,z/w); }
        virtual void end() {}
       
        const Vec3*     _vertices;
        const Vec4*     _vertices4;
        BoundingBox     _bb; 
};

BoundingBox Drawable::computeBound() const
{
    ComputeBound cb;

    Drawable* non_const_this = const_cast<Drawable*>(this);
    non_const_this->accept(cb);
    
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
    setupGLExtenions(contextID);
}

Drawable::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isVertexProgramSupported = rhs._isVertexProgramSupported;
    _isSecondaryColorSupported = rhs._isSecondaryColorSupported;
    _isFogCoordSupported = rhs._isFogCoordSupported;
    _isMultiTexSupported = rhs._isMultiTexSupported;
    _isOcclusionQuerySupported = rhs._isOcclusionQuerySupported;
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
}


void Drawable::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isVertexProgramSupported) _isVertexProgramSupported = false;
    if (!rhs._isSecondaryColorSupported) _isSecondaryColorSupported = false;
    if (!rhs._isFogCoordSupported) _isFogCoordSupported = false;
    if (!rhs._isMultiTexSupported) _isMultiTexSupported = false;
    if (!rhs._isOcclusionQuerySupported) _isOcclusionQuerySupported = false;
    if (!rhs._isARBOcclusionQuerySupported) _isARBOcclusionQuerySupported = false;

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
}

void Drawable::Extensions::setupGLExtenions(unsigned int contextID)
{
    _isVertexProgramSupported = isGLExtensionSupported(contextID,"GL_ARB_vertex_program");
    _isSecondaryColorSupported = isGLExtensionSupported(contextID,"GL_EXT_secondary_color");
    _isFogCoordSupported = isGLExtensionSupported(contextID,"GL_EXT_fog_coord");
    _isMultiTexSupported = isGLExtensionSupported(contextID,"GL_ARB_multitexture");
    _isOcclusionQuerySupported = osg::isGLExtensionSupported(contextID, "GL_NV_occlusion_query" );
    _isARBOcclusionQuerySupported = osg::isGLExtensionSupported(contextID, "GL_ARB_occlusion_query" );

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
    _glIsBuffer = ((IsBufferProc)osg::getGLExtensionFuncPtr("glIsBuffer","glIsBufferARB"));
    _glGetBufferSubData = ((GetBufferSubDataProc)osg::getGLExtensionFuncPtr("glGetBufferSubData","glGetBufferSubDataARB"));
    _glMapBuffer = ((MapBufferProc)osg::getGLExtensionFuncPtr("glMapBuffer","glMapBufferARB"));
    _glUnmapBuffer = ((UnmapBufferProc)osg::getGLExtensionFuncPtr("glUnmapBuffer","glUnmapBufferARB"));
    _glGetBufferParameteriv = ((GetBufferParameterivProc)osg::getGLExtensionFuncPtr("glGetBufferParameteriv","glGetBufferParameterivARB"));
    _glGetBufferPointerv = ((GetBufferPointervProc)osg::getGLExtensionFuncPtr("glGetBufferPointerv","glGetBufferPointervARB"));

    _glGenOcclusionQueries = ((GenOcclusionQueriesProc)osg::getGLExtensionFuncPtr("glGenOcclusionQueries","glGenOcclusionQueriesNV"));
    _glDeleteOcclusionQueries = ((DeleteOcclusionQueriesProc)osg::getGLExtensionFuncPtr("glDeleteOcclusionQueries","glDeleteOcclusionQueriesNV"));
    _glIsOcclusionQuery = ((IsOcclusionQueryProc)osg::getGLExtensionFuncPtr("glIsOcclusionQuery","_glIsOcclusionQueryNV"));
    _glBeginOcclusionQuery = ((BeginOcclusionQueryProc)osg::getGLExtensionFuncPtr("glBeginOcclusionQuery","glBeginOcclusionQueryNV"));
    _glEndOcclusionQuery = ((EndOcclusionQueryProc)osg::getGLExtensionFuncPtr("glEndOcclusionQuery","glEndOcclusionQueryNV"));
    _glGetOcclusionQueryiv = ((GetOcclusionQueryivProc)osg::getGLExtensionFuncPtr("glGetOcclusionQueryiv","glGetOcclusionQueryivNV"));
    _glGetOcclusionQueryuiv = ((GetOcclusionQueryuivProc)osg::getGLExtensionFuncPtr("glGetOcclusionQueryuiv","glGetOcclusionQueryuivNV"));

    _gl_gen_queries_arb = (GenQueriesProc)osg::getGLExtensionFuncPtr("glGenQueries", "glGenQueriesARB");
    _gl_delete_queries_arb = (DeleteQueriesProc)osg::getGLExtensionFuncPtr("glDeleteQueries", "glDeleteQueriesARB");
    _gl_is_query_arb = (IsQueryProc)osg::getGLExtensionFuncPtr("glIsQuery", "glIsQueryARB");
    _gl_begin_query_arb = (BeginQueryProc)osg::getGLExtensionFuncPtr("glBeginQuery", "glBeginQueryARB");
    _gl_end_query_arb = (EndQueryProc)osg::getGLExtensionFuncPtr("glEndQuery", "glEndQueryARB");
    _gl_get_queryiv_arb = (GetQueryivProc)osg::getGLExtensionFuncPtr("glGetQueryiv", "glGetQueryivARB");
    _gl_get_query_objectiv_arb = (GetQueryObjectivProc)osg::getGLExtensionFuncPtr("glGetQueryObjectiv","glGetQueryObjectivARB");
    _gl_get_query_objectuiv_arb = (GetQueryObjectuivProc)osg::getGLExtensionFuncPtr("glGetQueryObjectuiv","glGetQueryObjectuivARB");
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
    if (_glGenBuffers) _glGenBuffers(n, buffers); 
    else notify(WARN)<<"Error: glGenBuffers not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glBindBuffer(GLenum target, GLuint buffer) const
{
    if (_glBindBuffer) _glBindBuffer(target, buffer); 
    else notify(WARN)<<"Error: glBindBuffer not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glBufferData(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage) const
{
    if (_glBufferData) _glBufferData(target, size, data, usage); 
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glBufferSubData(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data) const
{
    if (_glBufferSubData) _glBufferSubData(target, offset, size, data); 
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glDeleteBuffers(GLsizei n, const GLuint *buffers) const
{
    if (_glDeleteBuffers) _glDeleteBuffers(n, buffers); 
    else notify(WARN)<<"Error: glBufferData not supported by OpenGL driver"<<std::endl;
}

GLboolean Drawable::Extensions::glIsBuffer (GLuint buffer) const
{
    if (_glIsBuffer) return _glIsBuffer(buffer);
    else 
    {
        notify(WARN)<<"Error: glIsBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void Drawable::Extensions::glGetBufferSubData (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data) const
{
    if (_glGetBufferSubData) _glGetBufferSubData(target,offset,size,data);
    else notify(WARN)<<"Error: glGetBufferSubData not supported by OpenGL driver"<<std::endl;
}

GLvoid* Drawable::Extensions::glMapBuffer (GLenum target, GLenum access) const
{
    if (_glMapBuffer) return _glMapBuffer(target,access);
    else 
    {
        notify(WARN)<<"Error: glMapBuffer not supported by OpenGL driver"<<std::endl;
        return 0;
    }
}

GLboolean Drawable::Extensions::glUnmapBuffer (GLenum target) const
{
    if (_glUnmapBuffer) return _glUnmapBuffer(target);
    else 
    {
        notify(WARN)<<"Error: glUnmapBuffer not supported by OpenGL driver"<<std::endl;
        return GL_FALSE;
    }
}

void Drawable::Extensions::glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) const
{
    if (_glGetBufferParameteriv) _glGetBufferParameteriv(target,pname,params);
    else notify(WARN)<<"Error: glGetBufferParameteriv not supported by OpenGL driver"<<std::endl;
}

void Drawable::Extensions::glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params) const
{
    if (_glGetBufferPointerv) _glGetBufferPointerv(target,pname,params);
    else notify(WARN)<<"Error: glGetBufferPointerv not supported by OpenGL driver"<<std::endl;
}


void Drawable::Extensions::glGenOcclusionQueries( GLsizei n, GLuint *ids ) const
{
    if (_glGenOcclusionQueries)
    {
        _glGenOcclusionQueries( n, ids );
    }
    else
    {
        osg::notify(osg::WARN)<<"Error: glGenOcclusionQueries not supported by OpenGL driver"<<std::endl;
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
        osg::notify(osg::WARN)<<"Error: glDeleteOcclusionQueries not supported by OpenGL driver"<<std::endl;
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
        osg::notify(osg::WARN)<<"Error: glIsOcclusionQuery not supported by OpenGL driver"<<std::endl;
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
        osg::notify(osg::WARN)<<"Error: glBeginOcclusionQuery not supported by OpenGL driver"<<std::endl;
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
        osg::notify(osg::WARN)<<"Error: glEndOcclusionQuery not supported by OpenGL driver"<<std::endl;
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
        osg::notify(osg::WARN)<<"Error: glGetOcclusionQueryiv not supported by OpenGL driver"<<std::endl;
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
        osg::notify(osg::WARN)<<"Error: glGetOcclusionQueryuiv not supported by OpenGL driver"<<std::endl;
    }    
}

void Drawable::Extensions::glGetQueryiv(GLenum target, GLenum pname, GLint *params) const
{
  if (_gl_get_queryiv_arb)
    _gl_get_queryiv_arb(target, pname, params);
  else
    osg::notify(osg::WARN) << "Error: glGetQueryiv not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGenQueries(GLsizei n, GLuint *ids) const
{
  if (_gl_gen_queries_arb)
    _gl_gen_queries_arb(n, ids);
  else
    osg::notify(osg::WARN) << "Error: glGenQueries not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glBeginQuery(GLenum target, GLuint id) const
{
  if (_gl_begin_query_arb)
    _gl_begin_query_arb(target, id);
  else
    osg::notify(osg::WARN) << "Error: glBeginQuery not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glEndQuery(GLenum target) const
{
  if (_gl_end_query_arb)
    _gl_end_query_arb(target);
  else
    osg::notify(osg::WARN) << "Error: glEndQuery not supported by OpenGL driver" << std::endl;
}

GLboolean Drawable::Extensions::glIsQuery(GLuint id) const
{
  if (_gl_is_query_arb) return _gl_is_query_arb(id);

  osg::notify(osg::WARN) << "Error: glIsQuery not supported by OpenGL driver" << std::endl;
  return false;
}

void Drawable::Extensions::glDeleteQueries(GLsizei n, const GLuint *ids) const
{
    if (_gl_delete_queries_arb) 
        _gl_delete_queries_arb(n, ids);
    else
        osg::notify(osg::WARN) << "Error: glIsQuery not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params) const
{
  if (_gl_get_query_objectiv_arb)
    _gl_get_query_objectiv_arb(id, pname, params);
  else
    osg::notify(osg::WARN) << "Error: glGetQueryObjectiv not supported by OpenGL driver" << std::endl;
}

void Drawable::Extensions::glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) const
{
  if (_gl_get_query_objectuiv_arb)
    _gl_get_query_objectuiv_arb(id, pname, params);
  else
    osg::notify(osg::WARN) << "Error: glGetQueryObjectuiv not supported by OpenGL driver" << std::endl;
}
