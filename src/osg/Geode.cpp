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
    
    _occluder = dynamic_cast<ConvexPlanerOccluder*>(copyop(geode.getOccluder()));
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

const bool Geode::addDrawable( Drawable *drawable )
{
    if (drawable && !containsDrawable(drawable))
    {
        // note ref_ptr<> automatically handles incrementing drawable's reference count.
        _drawables.push_back(drawable);
        
        // register as parent of drawable.
        drawable->addParent(this);
        
        dirtyBound();        
        
        return true;
    }
    else return false;
}


const bool Geode::removeDrawable( Drawable *drawable )
{
    DrawableList::iterator itr = findDrawable(drawable);
    if (itr!=_drawables.end())
    {
        // remove this Geode from the child parent list.
        drawable->removeParent(this);

        // note ref_ptr<> automatically handles decrementing drawable's reference count.
        _drawables.erase(itr);        
        
        dirtyBound();
        
        return true;
    }
    else return false;
}


const bool Geode::replaceDrawable( Drawable *origDrawable, Drawable *newDrawable )
{
    if (newDrawable==NULL || origDrawable==newDrawable) return false;

    DrawableList::iterator itr = findDrawable(origDrawable);
    if (itr!=_drawables.end())
    {
        
        // remove from origDrawable's parent list.
        origDrawable->removeParent(this);
        
        // note ref_ptr<> automatically handles decrementing origGset's reference count,
        // and inccrementing newGset's reference count.
        *itr = newDrawable;

        // register as parent of child.
        newDrawable->addParent(this);

        dirtyBound();
        
        return true;
    }
    else return false;

}


const bool Geode::computeBound() const
{
    BoundingBox bb;

//     if (_occluder.valid()) _occluder->computeBound(bb);

    DrawableList::const_iterator itr;
    for(itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        bb.expandBy((*itr)->getBound());
    }

    if (bb.isValid())
    {

        _bsphere._center = bb.center();
        _bsphere._radius = 0.0f;

        for(itr=_drawables.begin();
            itr!=_drawables.end();
            ++itr)
        {
            const BoundingBox& bbox = (*itr)->getBound();
            for(unsigned int c=0;c<8;++c)
            {
                _bsphere.expandRadiusBy(bbox.corner(c));
            }
        }

//         if (_occluder.valid()) _occluder->computeBound(_bsphere);

        _bsphere_computed=true;
        return true;
    }
    else
    {
        _bsphere.init();
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
