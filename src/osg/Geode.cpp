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
    // ref_ptr<> automactially decrements the reference count of all drawables.
}

const bool Geode::addDrawable( Drawable *gset )
{
    if (gset && !containsDrawable(gset))
    {
        // note ref_ptr<> automatically handles incrementing gset's reference count.
        _drawables.push_back(gset);
        dirtyBound();
        return true;
    }
    else return false;
}


const bool Geode::removeDrawable( Drawable *gset )
{
    DrawableList::iterator itr = findDrawable(gset);
    if (itr!=_drawables.end())
    {
        // note ref_ptr<> automatically handles decrementing gset's reference count.
        _drawables.erase(itr);
        dirtyBound();
        return true;
    }
    else return false;
}


const bool Geode::replaceDrawable( Drawable *origGset, Drawable *newGset )
{
    if (newGset==NULL || origGset==newGset) return false;

    DrawableList::iterator itr = findDrawable(origGset);
    if (itr!=_drawables.end())
    {
        // note ref_ptr<> automatically handles decrementing origGset's reference count,
        // and inccrementing newGset's reference count.
        *itr = newGset;
        dirtyBound();
        return true;
    }
    else return false;

}


const bool Geode::computeBound() const
{
    BoundingBox bb;

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
