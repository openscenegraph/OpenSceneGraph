// GeoSetBuilder.cpp

#if defined(WIN32) && !defined(__CYGWIN__)
#pragma warning( disable : 4786 )
#endif

#include "flt.h"
#include "FltFile.h"
#include "Pool.h"
#include "opcodes.h"
#include "GeoSetBuilder.h"

#include <osg/Object>
#include <osg/LOD>
#include <osg/Transparency>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/CullFace>
#include <osg/PolygonOffset>
#include <osg/Point>
#include <osg/Notify>

#include <map>
#include <algorithm>

using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                       DynGeoSet
//
////////////////////////////////////////////////////////////////////


#define APPEND_DynGeoSet_List(list)                     \
    if (source->list.size() > 0)                        \
        list.insert(list.end(),                         \
            source->list.begin(), source->list.end());


void DynGeoSet::append(DynGeoSet* source)
{
    APPEND_DynGeoSet_List(_primLenList)
    APPEND_DynGeoSet_List(_coordList)
    APPEND_DynGeoSet_List(_normalList)
    APPEND_DynGeoSet_List(_colorList)
    APPEND_DynGeoSet_List(_tcoordList)
}


#define VERIFY_DynGeoSet_Binding(binding,list)          \
        switch (binding)                                \
        {                                               \
        case osg::GeoSet::BIND_PERVERTEX:               \
            if (list.size() < _coordList.size()) {      \
                binding = osg::GeoSet::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        case osg::GeoSet::BIND_PERPRIM:                 \
            if (list.size() < _primLenList.size()) {    \
                binding = osg::GeoSet::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        case osg::GeoSet::BIND_OVERALL:                 \
            if (list.size() < 1) {                      \
                binding = osg::GeoSet::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        }

DynGeoSet::DynGeoSet():osg::GeoSet()
{
	// disable the attribute delete functor since the vectors contained in DynGeoSet
	// will delete the memory for us.
	_adf = NULL;
}

void DynGeoSet::setBinding()
{
    VERIFY_DynGeoSet_Binding(_normal_binding, _normalList)
    VERIFY_DynGeoSet_Binding(_color_binding, _colorList)
    VERIFY_DynGeoSet_Binding(_texture_binding, _tcoordList)

    // Set bindings
    setNormalBinding(_normal_binding);
    setColorBinding(_color_binding);
    setTextureBinding(_texture_binding);

    osg::StateSet* stateset = getStateSet();
    if (stateset)
    {
        if (_normal_binding == osg::GeoSet::BIND_OFF)
            stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    }
}


bool DynGeoSet::setLists()
{
    if ((_primLenList.size() > 0) && (_coordList.size() > 0))
    {
        setPrimLengths(&_primLenList.front());
        setCoords(&_coordList.front());

        if ((_normalList.size() > 0)
        &&  (getNormalBinding() != osg::GeoSet::BIND_OFF))
            setNormals(&_normalList.front());

        if ((_colorList.size() > 0)
        &&  (getColorBinding() != osg::GeoSet::BIND_OFF))
            setColors(&_colorList.front());

        if ((_tcoordList.size() > 0)
        &&  (getTextureBinding() != osg::GeoSet::BIND_OFF))
            setTextureCoords(&_tcoordList.front());

        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////////////
//
//                       GeoSetBuilder
//
////////////////////////////////////////////////////////////////////

// OpenFlight don't save data in GeoSets.  This class tries to find
// existing GeoSets with matching state before creating a new GeoSet.

GeoSetBuilder::GeoSetBuilder(osg::Geode* geode)
{
    _geode = geode;
    initPrimData();
}


void GeoSetBuilder::initPrimData()
{
    _dynGeoSet = new DynGeoSet;
    _dynGeoSet->setStateSet(new osg::StateSet);
}


osg::Geode* GeoSetBuilder::createOsgGeoSets()
{
    for(DynGeoSetList::iterator itr=_dynGeoSetList.begin();
        itr!=_dynGeoSetList.end();
        ++itr)
    {
        DynGeoSet* dgset = itr->get();
        if (dgset)
        {
            int prims = dgset->primLenListSize();
            if (prims > 0)
            {
                dgset->setLists();
                dgset->setNumPrims(prims);
                _geode.get()->addDrawable(dgset);
            }
        }
    }

    return _geode.get();
}


bool GeoSetBuilder::addPrimitive()
{
    DynGeoSet* dgset = getDynGeoSet();  // This is the new geoset we want to add

    if (dgset->getPrimType() == osg::GeoSet::NO_TYPE)
        dgset->setPrimType(findPrimType(dgset->coordListSize()));

    // Still no primitive type?
    if (dgset->getPrimType() == osg::GeoSet::NO_TYPE)
        return false;

    dgset->setBinding();

    DynGeoSet* match = findMatchingGeoSet();
    if (match)
        match->append(dgset);
    else
        _dynGeoSetList.push_back(dgset);

    initPrimData();     // initialize _dynGeoSet
    return true;
}


DynGeoSet* GeoSetBuilder::findMatchingGeoSet()
{
    DynGeoSet* new_dgset = getDynGeoSet();
    for(DynGeoSetList::iterator itr=_dynGeoSetList.begin();
        itr!=_dynGeoSetList.end();
        ++itr)
    {
        DynGeoSet* dgset = itr->get();
        if (*new_dgset == *dgset)
            return dgset;
    }

    return NULL;
}


osg::GeoSet::PrimitiveType GeoSetBuilder::findPrimType(const int nVertices)
{
    switch (nVertices)
    {
        case 1: return osg::GeoSet::POINTS;
        case 2: return osg::GeoSet::LINES;
        case 3: return osg::GeoSet::TRIANGLES;
        case 4: return osg::GeoSet::QUADS;
    }

    if (nVertices >= 5) return osg::GeoSet::POLYGON;
    return osg::GeoSet::NO_TYPE;
}


