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
#include <osg/Geode>
#include <osg/Notify>

#include <stdio.h>
#include <math.h>

#define square(x)   ((x)*(x))

using namespace osg;

Geode::Geode()
{
}

Geode::Geode(const Geode& geode,const CopyOp& copyop):
    Node(geode,copyop)
{
    for(DrawableList::const_iterator itr=geode._drawables.begin();
        itr!=geode._drawables.end();
        ++itr)
    {
        Drawable* drawable = copyop(itr->get());
        if (drawable) addDrawable(drawable);
    }
}

Geode::~Geode()
{
    // remove reference to this from children's parent lists.
    for(DrawableList::iterator itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
       (*itr)->removeParent(this);
    }
}

bool Geode::addDrawable( Drawable *drawable )
{
    if (drawable /* && !containsDrawable(drawable)*/)
    {
        // note ref_ptr<> automatically handles incrementing drawable's reference count.
        _drawables.push_back(drawable);
        
        // register as parent of drawable.
        drawable->addParent(this);
        
        if (drawable->requiresUpdateTraversal())
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
        }
        
        if (drawable->requiresEventTraversal())
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()+1);
        }

        dirtyBound();        
        
        return true;
    }
    else return false;
}


bool Geode::removeDrawable( Drawable *drawable )
{
    return removeDrawables(getDrawableIndex(drawable),1);
}

bool Geode::removeDrawables(unsigned int pos,unsigned int numDrawablesToRemove)
{
    if (pos<_drawables.size() && numDrawablesToRemove>0)
    {
        unsigned int endOfRemoveRange = pos+numDrawablesToRemove;
        if (endOfRemoveRange>_drawables.size())
        {
            notify(DEBUG_INFO)<<"Warning: Geode::removeDrawable(i,numDrawablesToRemove) has been passed an excessive number"<<std::endl;
            notify(DEBUG_INFO)<<"         of drawables to remove, trimming just to end of drawable list."<<std::endl;
            endOfRemoveRange=_drawables.size();
        }

        unsigned int updateCallbackRemoved = 0;
        unsigned int eventCallbackRemoved = 0;
        for(unsigned i=pos;i<endOfRemoveRange;++i)
        {
            // remove this Geode from the child parent list.
            _drawables[i]->removeParent(this);
            // update the number of app calbacks removed
            if (_drawables[i]->requiresUpdateTraversal()) ++updateCallbackRemoved;
            if (_drawables[i]->requiresEventTraversal()) ++eventCallbackRemoved;
        }

        _drawables.erase(_drawables.begin()+pos,_drawables.begin()+endOfRemoveRange);

        if (updateCallbackRemoved)
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-updateCallbackRemoved);
        }
        
        if (eventCallbackRemoved)
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-eventCallbackRemoved);
        }

        dirtyBound();
        
        return true;
    }
    else return false;
}

bool Geode::replaceDrawable( Drawable *origDrawable, Drawable *newDrawable )
{
    if (newDrawable==NULL || origDrawable==newDrawable) return false;

    unsigned int pos = getDrawableIndex(origDrawable);
    if (pos<_drawables.size())
    {
        return setDrawable(pos,newDrawable);
    }
    return false;
}

bool Geode::setDrawable( unsigned  int i, Drawable* newDrawable )
{
    if (i<_drawables.size() && newDrawable)
    {
    
        Drawable* origDrawable = _drawables[i].get();

        int deltaUpdate = 0;
        if (origDrawable->requiresUpdateTraversal()) --deltaUpdate;
        if (newDrawable->requiresUpdateTraversal()) ++deltaUpdate;
        if (deltaUpdate!=0)
        {
            setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+deltaUpdate);
        }

        int deltaEvent = 0;
        if (origDrawable->requiresEventTraversal()) --deltaEvent;
        if (newDrawable->requiresEventTraversal()) ++deltaEvent;
        if (deltaEvent!=0)
        {
            setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()+deltaEvent);
        }


        // remove from origDrawable's parent list.
        origDrawable->removeParent(this);
        
        // note ref_ptr<> automatically handles decrementing origGset's reference count,
        // and inccrementing newGset's reference count.
        _drawables[i] = newDrawable;

        // register as parent of child.
        newDrawable->addParent(this);


        dirtyBound();
        
        return true;
    }
    else return false;

}


BoundingSphere Geode::computeBound() const
{
    BoundingSphere bsphere;

    _bbox.init();

    DrawableList::const_iterator itr;
    for(itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        _bbox.expandBy((*itr)->getBound());
    }

    if (_bbox.valid())
    {
        bsphere.expandBy(_bbox);
    }
    return bsphere;
}

void Geode::compileDrawables(State& state)
{
    for(DrawableList::iterator itr = _drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        (*itr)->compileGLObjects(state);
    }
}

void Geode::releaseGLObjects(osg::State* state) const
{
    Node::releaseGLObjects(state);

    for(DrawableList::const_iterator itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }
}
