#include <osg/Types>
#include <osg/Notify>
#include <osg/TriangleFunctor>

#include <osgUtil/TriStripVisitor>

#include <stdio.h>

#include "TriStrip_tri_stripper.h"

using namespace osg;
using namespace osgUtil;

// triangle functor.
struct TriangleAcumulatorFunctor
{

    triangle_stripper::tri_stripper::indices in_indices;
    const Vec3* _vbase;

    TriangleAcumulatorFunctor() : _vbase(0) {}
    
    void setCoords( const Vec3* vbase ) { _vbase = vbase; }

    inline void operator() ( const Vec3 &v1, const Vec3 &v2, const Vec3 &v3 )
    {
        int p1 = (int)(&v1-_vbase);
        int p2 = (int)(&v2-_vbase);
        int p3 = (int)(&v3-_vbase);
        if (p1==p2 || p1==p3 || p2==p3) return;
        in_indices.push_back(p1);
        in_indices.push_back(p2);
        in_indices.push_back(p3);
    }
};

void TriStripVisitor::stripify(Geometry& geom)
{

    unsigned int numSurfacePrimitives = 0;
    unsigned int numNonSurfacePrimitives = 0;

    Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    Geometry::PrimitiveSetList::iterator itr;
    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        switch((*itr)->getMode())
        {
            case(PrimitiveSet::TRIANGLES):
            case(PrimitiveSet::TRIANGLE_STRIP):
            case(PrimitiveSet::TRIANGLE_FAN):
            case(PrimitiveSet::QUADS):
            case(PrimitiveSet::QUAD_STRIP):
            case(PrimitiveSet::POLYGON):
                ++numSurfacePrimitives;
                break;
            default:
                ++numNonSurfacePrimitives;
                break;
                
        }
    }
    
    if (!numSurfacePrimitives) return;
    
    TriangleFunctor<TriangleAcumulatorFunctor> taf;

    Geometry::PrimitiveSetList new_primitives;
    new_primitives.reserve(primitives.size());

    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        switch((*itr)->getMode())
        {
            case(PrimitiveSet::TRIANGLES):
            case(PrimitiveSet::TRIANGLE_STRIP):
            case(PrimitiveSet::TRIANGLE_FAN):
            case(PrimitiveSet::QUADS):
            case(PrimitiveSet::QUAD_STRIP):
            case(PrimitiveSet::POLYGON):
                (*itr)->accept(taf);
                break;
            default:
                new_primitives.push_back(*itr);
                break;

        }
    }
    
    if (!taf.in_indices.empty())
    {
        int in_numVertices = -1;
        for(triangle_stripper::tri_stripper::indices::iterator itr=taf.in_indices.begin();
            itr!=taf.in_indices.end();
            ++itr)
        {
            if ((int)*itr>in_numVertices) in_numVertices=*itr;
        }
        // the largest indice is in_numVertices, but indices start at 0
        // so increment to give to the corrent number of verticies.
        ++in_numVertices;            

        int in_cacheSize = 16;
        int in_minStripLength = 2;

        triangle_stripper::tri_stripper stripifier(taf.in_indices);
        stripifier.SetCacheSize(in_cacheSize);
        stripifier.SetMinStripSize(in_minStripLength);

        triangle_stripper::tri_stripper::primitives_vector outPrimitives;
        stripifier.Strip(&outPrimitives);

        for(triangle_stripper::tri_stripper::primitives_vector::iterator itr=outPrimitives.begin();
            itr!=outPrimitives.end();
            ++itr)
        {
            osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(itr->m_Type,itr->m_Indices.begin(),itr->m_Indices.end());
            new_primitives.push_back(elements);
        }

        geom.setPrimitiveSetList(new_primitives);
    }
}


void TriStripVisitor::apply(Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); ++i )
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) stripify(*geom);
    }
}
