#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <stdio.h>

#include <osg/GeoSet>
#include <osg/Geode>
#include <osg/Types>

#include <osgUtil/TriStripVisitor>

#include "NvTriStripObjects.h"

using namespace osg;
using namespace osgUtil;

// triangle functor.
struct TriangleAcumulatorFunctor
{

    WordVec in_indices;
    const Vec3* _vbase;

    TriangleAcumulatorFunctor( const Vec3* vbase ) : _vbase(vbase) {}

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

void createStrips(
    NvStripInfoVec& strips,
    NvFaceInfoVec& leftoverFaces,
    int& noPrims,
    int  **lens,
    osg::ushort **osg_indices)
{
    int nStripCount = strips.size();
    assert(nStripCount > 0);

    noPrims = strips.size()+leftoverFaces.size();
    *lens = new int [noPrims];

    int* lensPtr = *lens;

    unsigned int i;
    int noIndices = 0;
    for (i = 0; i < strips.size(); i++)
    {
        noIndices += strips[i]->m_faces.size()+2;
    }

    noIndices += leftoverFaces.size()*3;

    *osg_indices = new osg::ushort[noIndices];
    osg::ushort *osg_indicesPtr = *osg_indices;

    for (i = 0; i < strips.size(); i++)
    {
        NvStripInfo *strip = strips[i];
        int nStripFaceCount = strip->m_faces.size();

        *(lensPtr++) = nStripFaceCount+2;

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

            *(osg_indicesPtr++) = tFirstFace.m_v0;
            *(osg_indicesPtr++) = tFirstFace.m_v1;
            *(osg_indicesPtr++) = tFirstFace.m_v2;

            // Update last face info
            tLastFace = tFirstFace;
        }

        for (int j = 1; j < nStripFaceCount; j++)
        {
            int nUnique = NvStripifier::GetUniqueVertexInB(&tLastFace, strip->m_faces[j]);
            if (nUnique != -1)
            {
                *(osg_indicesPtr++) = nUnique;

                // Update last face info
                tLastFace.m_v0 = tLastFace.m_v1;
                tLastFace.m_v1 = tLastFace.m_v2;
                tLastFace.m_v2 = nUnique;
            }
        }

    }

    for (i = 0; i < leftoverFaces.size(); ++i)
    {

        *(lensPtr++) = 3;

        *(osg_indicesPtr++) = leftoverFaces[i]->m_v0;
        *(osg_indicesPtr++) = leftoverFaces[i]->m_v1;
        *(osg_indicesPtr++) = leftoverFaces[i]->m_v2;

    }

}


void TriStripVisitor::stripify(GeoSet& gset)
{
    GeoSet::PrimitiveType primTypeIn = gset.getPrimType();
    GeoSet::PrimitiveType primTypeOut = gset.getPrimType();

    // determine whether to do smoothing or not, and if
    // the primitive type needs to be modified enable smoothed normals.
    bool doStripify;
    switch(primTypeIn)
    {
        case(GeoSet::QUADS):
        case(GeoSet::QUAD_STRIP):
        case(GeoSet::POLYGON):
        case(GeoSet::TRIANGLES):
        case(GeoSet::TRIANGLE_STRIP):
        case(GeoSet::TRIANGLE_FAN):
            primTypeOut = GeoSet::TRIANGLE_STRIP;
            doStripify = true;
            break;
        case(GeoSet::FLAT_TRIANGLE_STRIP):
        case(GeoSet::FLAT_TRIANGLE_FAN):
// comment out for time being since per vertex colors and normals need
// to take account of the osg::GeoSet::_flat_shaded_skip
//             primTypeOut = GeoSet::FLAT_TRIANGLE_STRIP;
//             doStripify = true;
//             break;
        default:                 // points and lines etc.
            doStripify = false;
            break;
    }

    if (doStripify)
    {

        TriangleAcumulatorFunctor tt(gset.getCoords());
        for_each_triangle( gset, tt );

        if (!tt.in_indices.empty())
        {
            int in_numVertices = -1;
            for(WordVec::iterator itr=tt.in_indices.begin();
                itr!=tt.in_indices.end();
                ++itr)
            {
                if (*itr>in_numVertices) in_numVertices=*itr;
            }
            // the largest indice is in_numVertices, but indices start at 0
            // so increment to give to the corrent number of verticies.
            ++in_numVertices;            
            
            int in_cacheSize = 16;
            int in_minStripLength = 1;
            NvStripInfoVec strips;
            NvFaceInfoVec leftoverFaces;

            NvStripifier stripifier;
            stripifier.Stripify(tt.in_indices,
                in_numVertices,
                in_cacheSize,
                in_minStripLength,
                strips,
                leftoverFaces);

            int noPrims;
            int *lens;
            osg::ushort* osg_indices;

            createStrips(strips,leftoverFaces,noPrims,&lens,&osg_indices);

            if (primTypeIn!=primTypeOut)
                gset.setPrimType( primTypeOut );

            gset.setPrimLengths(lens);
            gset.setNumPrims(noPrims);

            gset.setCoords(gset.getCoords(),osg_indices);

            if (gset.getTextureCoords())
            {
                switch(gset.getTextureBinding())
                {
                    case(GeoSet::BIND_OVERALL):
                        // leave as before
                        break;
                    case(GeoSet::BIND_PERPRIM):
                        // switch off tex coords..
                        gset.setTextureBinding(GeoSet::BIND_OFF);
                        gset.setTextureCoords(NULL);
                        break;
                    case(GeoSet::BIND_PERVERTEX):
                        // set up the indexing.
                        gset.setTextureCoords(gset.getTextureCoords(),osg_indices);
                        break;
                    case(GeoSet::BIND_OFF):
                    case(GeoSet::BIND_DEFAULT):
                        // switch off tex coords..
                        gset.setTextureCoords(NULL);
                        break;
                }
            }

            if (gset.getColors())
            {
                switch(gset.getColorBinding())
                {
                    case(GeoSet::BIND_OVERALL):
                        // leave as before
                        break;
                    case(GeoSet::BIND_PERPRIM):
                        // switch off colors..
                        gset.setColorBinding(GeoSet::BIND_OFF);
                        gset.setColors(NULL);
                        break;
                    case(GeoSet::BIND_PERVERTEX):
                        // set up the indexing.
                        gset.setColors(gset.getColors(),osg_indices);
                        break;
                    case(GeoSet::BIND_OFF):
                    case(GeoSet::BIND_DEFAULT):
                        // switch off colors..
                        gset.setColors(NULL);
                        break;
                }
            }

            if (gset.getNormals())
            {
                switch(gset.getNormalBinding())
                {
                    case(GeoSet::BIND_OVERALL):
                        // leave as before
                        break;
                    case(GeoSet::BIND_PERPRIM):
                        // switch off normals..
                        gset.setNormalBinding(GeoSet::BIND_OFF);
                        gset.setNormals(NULL);
                        break;
                    case(GeoSet::BIND_PERVERTEX):
                        // set up the indexing.
                        gset.setNormals(gset.getNormals(),osg_indices);
                        break;
                    case(GeoSet::BIND_OFF):
                    case(GeoSet::BIND_DEFAULT):
                        // switch off normals..
                        gset.setNormals(NULL);
                        break;
                }
            }

            gset.dirtyDisplayList();
        }
        else
        {
            cout << "No triangles to stripify"<<endl;
        }

    }
}


void TriStripVisitor::apply(Geode& geode)
{
    for(int i = 0; i < geode.getNumDrawables(); ++i )
    {
        osg::GeoSet* gset = dynamic_cast<osg::GeoSet*>(geode.getDrawable(i));
        if (gset) stripify(*gset);
    }
}
