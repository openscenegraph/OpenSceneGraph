
#include <osg/Types>
#include <osg/Notify>

#include <osgUtil/TriStripVisitor>

#include <stdio.h>

#include "NvTriStripObjects.h"

using namespace osg;
using namespace osgUtil;

// triangle functor.
struct TriangleAcumulatorFunctor
{

    WordVec in_indices;
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
        for(WordVec::iterator itr=taf.in_indices.begin();
            itr!=taf.in_indices.end();
            ++itr)
        {
            if (*itr>in_numVertices) in_numVertices=*itr;
        }
        // the largest indice is in_numVertices, but indices start at 0
        // so increment to give to the corrent number of verticies.
        ++in_numVertices;            

        int in_cacheSize = 16;
        int in_minStripLength = 2;
        NvStripInfoVec strips;
        NvFaceInfoVec leftoverFaces;

        NvStripifier stripifier;
        stripifier.Stripify(taf.in_indices,
            in_numVertices,
            in_cacheSize,
            in_minStripLength,
            strips,
            leftoverFaces);

        unsigned int i;
        for (i = 0; i < strips.size(); ++i)
        {
        
            NvStripInfo *strip = strips[i];
            int nStripFaceCount = strip->m_faces.size();

            osg::DrawElementsUShort* elements = osgNew osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP);
            elements->reserve(nStripFaceCount+2);
            new_primitives.push_back(elements);

            NvFaceInfo tLastFace(0, 0, 0);

            // Handle the first face in the strip
            {
                NvFaceInfo tFirstFace(strip->m_faces[0]->m_v0, strip->m_faces[0]->m_v1, strip->m_faces[0]->m_v2);

                // If there is a second face, reorder vertices such that the
                // unique vertex is first
                if (nStripFaceCount > 1)
                {
                    int nUnique = NvStripifier::GetUniqueVertexInB(strip->m_faces[1], &tFirstFace);
                    if (nUnique == tFirstFace.m_v1)
                    {
                        std::swap(tFirstFace.m_v0, tFirstFace.m_v1);
                    }
                    else if (nUnique == tFirstFace.m_v2)
                    {
                        std::swap(tFirstFace.m_v0, tFirstFace.m_v2);
                    }

                    // If there is a third face, reorder vertices such that the
                    // shared vertex is last
                    if (nStripFaceCount > 2)
                    {
                        int nShared = NvStripifier::GetSharedVertex(strip->m_faces[2], &tFirstFace);
                        if (nShared == tFirstFace.m_v1)
                        {
                            std::swap(tFirstFace.m_v1, tFirstFace.m_v2);
                        }
                    }
                }

                elements->push_back(tFirstFace.m_v0);
                elements->push_back(tFirstFace.m_v1);
                elements->push_back(tFirstFace.m_v2);

                // Update last face info
                tLastFace = tFirstFace;
            }

            for (int j = 1; j < nStripFaceCount; j++)
            {
                int nUnique = NvStripifier::GetUniqueVertexInB(&tLastFace, strip->m_faces[j]);
                if (nUnique != -1)
                {
                    elements->push_back(nUnique);

                    // Update last face info
                    tLastFace.m_v0 = tLastFace.m_v1;
                    tLastFace.m_v1 = tLastFace.m_v2;
                    tLastFace.m_v2 = nUnique;
                }
            }

        }

        if (leftoverFaces.size())
        {

            osg::DrawElementsUShort* triangles = osgNew osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES);
            triangles->reserve(leftoverFaces.size()*3);
            new_primitives.push_back(triangles);

            for (i = 0; i < leftoverFaces.size(); ++i)
            {

                triangles->push_back(leftoverFaces[i]->m_v0);
                triangles->push_back(leftoverFaces[i]->m_v1);
                triangles->push_back(leftoverFaces[i]->m_v2);
            }
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
