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
        
        virtual void setVertexArray(unsigned int count,const Vec2* vertices) 
        {
            notify(WARN)<<"ComputeBound does not support Vec2* vertex arrays"<<std::endl;
        }

        virtual void setVertexArray(unsigned int,const Vec3* vertices) { _vertices = vertices; }

        virtual void setVertexArray(unsigned int count,const Vec4* vertices) 
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
