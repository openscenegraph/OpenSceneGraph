// GeoSetBuilder.cpp

// Modify DynGeoSet::addToGeometry to generate texture coordinates for texture unit 1 
// that is used to detail texture
// Julian Ortiz, June 18th 2003.

#if defined(_MSC_VER)
#pragma warning( disable : 4786 )
#endif

#include "flt.h"
#include "FltFile.h"
#include "Pool.h"
#include "opcodes.h"
#include "GeoSetBuilder.h"

#include <osg/Object>
#include <osg/LOD>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/CullFace>
#include <osg/PolygonOffset>
#include <osg/Point>
#include <osg/Notify>

#include <osgUtil/Optimizer>

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
    if (_normal_binding==osg::Geometry::BIND_PER_VERTEX || _normal_binding==osg::Geometry::BIND_PER_PRIMITIVE) APPEND_DynGeoSet_List(_normalList)
    if (_color_binding==osg::Geometry::BIND_PER_VERTEX || _color_binding==osg::Geometry::BIND_PER_PRIMITIVE) APPEND_DynGeoSet_List(_colorList)

    for (unsigned int i = 0; i < source->_tcoordLists.size(); i++)
    {
       if ((getTextureBinding(i)==osg::Geometry::BIND_PER_VERTEX) || 
           (getTextureBinding(i)==osg::Geometry::BIND_PER_PRIMITIVE)) 
       {
          if (source->_tcoordLists.size() > 0)
          {
             if (_tcoordLists.size() <= i)
                _tcoordLists.resize(i+1);

             _tcoordLists[i].insert(_tcoordLists[i].end(),
                                    source->_tcoordLists[i].begin(),
                                    source->_tcoordLists[i].end());
          }
       }
    }
}


#define VERIFY_DynGeoSet_Binding(binding,list)          \
        switch (binding)                                \
        {                                               \
        case osg::Geometry::BIND_PER_VERTEX:               \
            if (list.size() < _coordList.size()) {      \
                binding = osg::Geometry::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        case osg::Geometry::BIND_PER_PRIMITIVE:                 \
            if (list.size() < _primLenList.size()) {    \
                binding = osg::Geometry::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        case osg::Geometry::BIND_OVERALL:                 \
            if (list.size() < 1) {                      \
                binding = osg::Geometry::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        default:                                        \
            break;                                      \
        }

const osg::PrimitiveSet::Mode NO_PRIMITIVE_TYPE = (osg::PrimitiveSet::Mode)0xffff;

DynGeoSet::DynGeoSet()
{
    _primtype = NO_PRIMITIVE_TYPE;
    _normal_binding = osg::Geometry::BIND_OFF;
    _color_binding = osg::Geometry::BIND_OFF;

    _detailTextureEnabled = false;

    _geom = new osg::Geometry;
}

void DynGeoSet::setBinding()
{
    unsigned int i;

    VERIFY_DynGeoSet_Binding(_normal_binding, _normalList)
    VERIFY_DynGeoSet_Binding(_color_binding, _colorList)

    for (i = 0; i < _tcoordLists.size(); i++)
       VERIFY_DynGeoSet_Binding(_texture_bindings[i], _tcoordLists[i])

    // Set bindings
    setNormalBinding(_normal_binding);
    setColorBinding(_color_binding);

    for (i = 0; i < _tcoordLists.size(); i++)
       setTextureBinding(i, _texture_bindings[i]);

    osg::StateSet* stateset = getStateSet();
    if (stateset)
    {
        if (_normal_binding == osg::Geometry::BIND_OFF)
            stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    }
}



void DynGeoSet::addToGeometry(osg::Geometry* geom)
{
    int indexBase = 0;
    
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
    if (vertices)
    {
        indexBase = vertices->size();
        vertices->insert(vertices->end(),_coordList.begin(),_coordList.end());
    }
    else
    {
        vertices = new osg::Vec3Array(_coordList.begin(),_coordList.end());
        geom->setVertexArray(vertices);
    }

    if (!_normalList.empty())
    {
        osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geom->getNormalArray());
        if (normals)
        {
            if (_normal_binding==osg::Geometry::BIND_PER_VERTEX || _normal_binding==osg::Geometry::BIND_PER_PRIMITIVE)
                normals->insert(normals->end(),_normalList.begin(),_normalList.end());
        }
        else
        {
            normals = new osg::Vec3Array(_normalList.begin(),_normalList.end());
            geom->setNormalArray(normals);

            switch(_normal_binding)
            {
                case(osg::Geometry::BIND_OVERALL):geom->setNormalBinding(osg::Geometry::BIND_OVERALL);break;
                case(osg::Geometry::BIND_PER_VERTEX):geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);break;        
                case(osg::Geometry::BIND_PER_PRIMITIVE):geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);break;        
                default:geom->setNormalBinding(osg::Geometry::BIND_OFF); break;
            }
        }
    }

    for (unsigned int i = 0; i < _tcoordLists.size(); i++)
    {
       if (!_tcoordLists[i].empty())
       {
           // Grab the current layer's texture coordinate array from the 
           // geometry
           osg::Vec2Array* texcoords = 
              dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(i));

           // See if we need to create the texture coordinate array or add to
           // it
           if (texcoords)
           {
               // Append the new texture coordinates to the end of the existing
               // list
               texcoords->insert(texcoords->end(),_tcoordLists[i].begin(),
                                 _tcoordLists[i].end());
           }
           else
           {
               // Create a new texture coordinate array
               texcoords = new osg::Vec2Array(_tcoordLists[i].begin(),
                                              _tcoordLists[i].end());
               geom->setTexCoordArray(i,texcoords);
           }
       }
    }
   
    // If this geometry uses a detail texture, we apply the detail texture
    // scalars to the texture coordinates on layer 0 to get the detail texture
    // coordinates, which we put on layer 1.  Note that this assumes that
    // layer 1 is not in use for multitexturing.  This means that 
    // multitexturing and detail texturing are not supported at the same time.
    if ((_detailTextureEnabled) && (!_tcoordLists.empty()) &&
        (!_tcoordLists[0].empty()))
    {                
       // Create a new texture coordinate array for the detail texture
       // coordinates
       osg::Vec2Array *texcoords2 = new osg::Vec2Array(_tcoordLists[0].begin(),
                                                       _tcoordLists[0].end());

       // Scale the texture coordinates from layer 0
       for(unsigned int index=0;index<texcoords2->size();index++) 
       {
           (*texcoords2)[index][0] *= _detailTexCoord_m;
           (*texcoords2)[index][1] *= _detailTexCoord_n;
       }                

       // Set the new texcoord array on layer 1 (this wipes out any existing
       // texture coordinates there)
       geom->setTexCoordArray(1,texcoords2);
    }                                                    
    
    if (!_colorList.empty())
    {
        osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
        if (colors)
        {
            if (_color_binding==osg::Geometry::BIND_PER_VERTEX || _color_binding==osg::Geometry::BIND_PER_PRIMITIVE)
                colors->insert(colors->end(),_colorList.begin(),_colorList.end());
        }
        else
        {
            colors = new osg::Vec4Array(_colorList.begin(),_colorList.end());
            geom->setColorArray(colors);

            switch(_color_binding)
            {
                case(osg::Geometry::BIND_OVERALL):geom->setColorBinding(osg::Geometry::BIND_OVERALL);break;
                case(osg::Geometry::BIND_PER_VERTEX):geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);break;        
                case(osg::Geometry::BIND_PER_PRIMITIVE):geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);break;        
                default:geom->setColorBinding(osg::Geometry::BIND_OFF); break;
            }
        }
    }    
    
    if (_primtype!=osg::PrimitiveSet::POLYGON)
    {
        geom->addPrimitiveSet(new osg::DrawArrays(_primtype,indexBase,_coordList.size()));
    }
    else
    {
        for(PrimLenList::iterator itr=_primLenList.begin();
            itr!=_primLenList.end();
            ++itr)
        {
            geom->addPrimitiveSet(new osg::DrawArrays(_primtype,indexBase,*itr));
            indexBase += *itr;
        }
    }    
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

struct SortDynGeoSet
{
    bool operator () (const osg::ref_ptr<DynGeoSet>& lhs,const osg::ref_ptr<DynGeoSet>& rhs)
    {
        return *lhs<*rhs;
    }
};

osg::Geode* GeoSetBuilder::createOsgGeoSets(osg::Geode* geode)
{
    if( geode == NULL) geode = _geode.get();
    if( geode == NULL) return geode;

    for(DynGeoSetList::iterator itr = _dynGeoSetList.begin();
        itr!=_dynGeoSetList.end();
        ++itr)
    {
        DynGeoSet* dgset = itr->get();
        osg::Geometry* geom = dgset->getGeometry();
        geode->addDrawable(geom);
        dgset->addToGeometry(geom);

        osg::StateSet* stateset = dgset->getStateSet();
        assert( stateset == geom->getStateSet() );
    }

    osgUtil::Optimizer optimizer;
    optimizer.optimize(geode, osgUtil::Optimizer::SHARE_DUPLICATE_STATE |
                              osgUtil::Optimizer::MERGE_GEOMETRY |
                              osgUtil::Optimizer::CHECK_GEOMETRY |
                              osgUtil::Optimizer::TESSELATE_GEOMETRY);
    return geode;
}


bool GeoSetBuilder::addPrimitive(bool dontMerge)
{
    DynGeoSet* dgset = getDynGeoSet();  // This is the new geoset we want to add

    if (dgset->getPrimType()==NO_PRIMITIVE_TYPE)
    {
        dgset->setPrimType(findPrimType(dgset->coordListSize()));
    }

    // Still no primitive type?
    if (dgset->getPrimType()==NO_PRIMITIVE_TYPE)
        return false;

    dgset->setBinding();

    dontMerge = true;

    if( dontMerge == true)
    {
        _dynGeoSetList.push_back(dgset);
    }
    else
    {
        DynGeoSet* match = findMatchingGeoSet();
        if (match)
            match->append(dgset);
        else
            _dynGeoSetList.push_back(dgset);
    }
    
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


osg::PrimitiveSet::Mode GeoSetBuilder::findPrimType(const int nVertices)
{
    switch (nVertices)
    {
        case 1: return osg::PrimitiveSet::POINTS;
        case 2: return osg::PrimitiveSet::LINES;
        case 3: return osg::PrimitiveSet::TRIANGLES;
        case 4: return osg::PrimitiveSet::QUADS;
    }

    if (nVertices>=5) return osg::PrimitiveSet::POLYGON;
    
    return NO_PRIMITIVE_TYPE;
}


