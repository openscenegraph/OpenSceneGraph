#include <stdio.h>
#include <math.h>
#include <osg/Geode>

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
    if (drawable && !containsDrawable(drawable))
    {
        // note ref_ptr<> automatically handles incrementing drawable's reference count.
        _drawables.push_back(drawable);
        
        // register as parent of drawable.
        drawable->addParent(this);
        
        if (drawable->getAppCallback())
        {
            setNumChildrenRequiringAppTraversal(getNumChildrenRequiringAppTraversal()+1);
        }
        
        dirtyBound();        
        
        return true;
    }
    else return false;
}


bool Geode::removeDrawable( Drawable *drawable )
{
    DrawableList::iterator itr = findDrawable(drawable);
    if (itr!=_drawables.end())
    {
        // remove this Geode from the child parent list.
        drawable->removeParent(this);

        if (drawable->getAppCallback())
        {
            setNumChildrenRequiringAppTraversal(getNumChildrenRequiringAppTraversal()-1);
        }

        // note ref_ptr<> automatically handles decrementing drawable's reference count.
        _drawables.erase(itr);        
        
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

        int delta = 0;
        if (origDrawable->getAppCallback()) --delta;
        if (newDrawable->getAppCallback()) ++delta;
        if (delta!=0)
        {
            setNumChildrenRequiringAppTraversal(getNumChildrenRequiringAppTraversal()+delta);
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


bool Geode::computeBound() const
{
    _bsphere.init();

    BoundingBox bb;

    DrawableList::const_iterator itr;
    for(itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        bb.expandBy((*itr)->getBound());
    }

    if (bb.valid())
    {
        _bsphere.expandBy(bb);
        _bsphere_computed=true;
        return true;
    }
    else
    {
        _bsphere_computed=true;
        return false;
    }
}

void Geode::compileDrawables(State& state)
{
    for(DrawableList::iterator itr = _drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        (*itr)->compile(state);
    }
}
