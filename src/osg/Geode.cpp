#include <stdio.h>
#include <math.h>
#include "osg/Geode"
#include "osg/Input"
#include "osg/Output"

#include <algorithm>

#ifdef __sgi
using std::find;
using std::for_each;
#endif

#define square(x)   ((x)*(x))

#include "osg/Registry"

using namespace osg;

RegisterObjectProxy<Geode> g_GeodeProxy;

Geode::Geode()
{
    _bsphere_computed = false;
}


Geode::~Geode()
{
    // ref_ptr<> automactially decrements the reference count of all geosets.
}


bool Geode::readLocalData(Input& fr)
{

    bool iteratorAdvanced = false;

    if (Node::readLocalData(fr)) iteratorAdvanced = true;

    int num_geosets;
    if (fr[0].matchWord("num_geosets") &&
        fr[1].getInt(num_geosets))
    {
        // could allocate space for children here...
        fr+=2;
        iteratorAdvanced = true;
    }

    GeoSet* gset_read = NULL;
    do
    {
        if ((gset_read=static_cast<GeoSet*>(GeoSet::instance()->readClone(fr))))
        {
            addGeoSet(gset_read);
            iteratorAdvanced = true;
        }

    } while(gset_read != NULL);

    return iteratorAdvanced;
}


bool Geode::writeLocalData(Output& fw)
{
    Node::writeLocalData(fw);

    fw.indent() << "num_geosets " << getNumGeosets() << endl;
    for(GeoSetList::iterator itr = _geosets.begin();
        itr!=_geosets.end();
        ++itr)
    {
        (*itr)->write(fw);
    }
    return true;
}


bool Geode::addGeoSet( GeoSet *gset )
{
    if (gset && !containsGeoSet(gset))
    {
        // note ref_ptr<> automatically handles incrementing gset's reference count.
        _geosets.push_back(gset);
        dirtyBound();
        return true;
    }
    else return false;
}

bool Geode::removeGeoSet( GeoSet *gset )
{
    GeoSetList::iterator itr = findGeoSet(gset);
    if (itr!=_geosets.end())
    {
        // note ref_ptr<> automatically handles decrementing gset's reference count.
        _geosets.erase(itr);
        dirtyBound();
        return true;
    }
    else return false;
}

bool Geode::replaceGeoSet( GeoSet *origGset, GeoSet *newGset )
{
    if (newGset==NULL || origGset==newGset) return false;

    GeoSetList::iterator itr = findGeoSet(origGset);
    if (itr!=_geosets.end())
    {
        // note ref_ptr<> automatically handles decrementing origGset's reference count,
        // and inccrementing newGset's reference count.
        *itr = newGset;
        dirtyBound();
        return true;
    }
    else return false;
    
}


bool Geode::computeBound( void )
{    
    BoundingBox bb;
    GeoSetList::iterator itr;
    for(itr=_geosets.begin();
        itr!=_geosets.end();
        ++itr)
    {
        bb.expandBy((*itr)->getBound());
    }
    
    _bsphere._center = bb.center();
    _bsphere._radius = 0.0f;
    
    for(itr=_geosets.begin();
        itr!=_geosets.end();
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


void Geode::compileGeoSets( void )
{
    for(GeoSetList::iterator itr = _geosets.begin();
        itr!=_geosets.end();
        ++itr)
    {
        (*itr)->compile();
    }
}
