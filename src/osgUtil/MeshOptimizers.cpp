/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2010 Tim Moore
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

#include <cassert>
#include <limits>

#include <algorithm>
#include <vector>

#include <iostream>

#include <osg/Geometry>
#include <osg/Math>
#include <osg/PrimitiveSet>
#include <osg/TriangleIndexFunctor>
#include <osg/TriangleLinePointIndexFunctor>

#include <osgUtil/MeshOptimizers>

using namespace osg;

namespace osgUtil
{

void GeometryCollector::reset()
{
    _geometryList.clear();
}

void GeometryCollector::apply(Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); ++i )
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) _geometryList.insert(geom);
    }
}

namespace
{
typedef std::vector<unsigned int> IndexList;

// A helper class that gathers up all the attribute arrays of an
// osg::Geometry object that are BIND_PER_VERTEX and runs an
// ArrayVisitor on them.
struct GeometryArrayGatherer
{
    typedef std::vector<osg::Array*> ArrayList;

    GeometryArrayGatherer(osg::Geometry& geometry)
    {
        add(geometry.getVertexArray());
        add(geometry.getNormalArray());
        add(geometry.getColorArray());
        add(geometry.getSecondaryColorArray());
        add(geometry.getFogCoordArray());
        unsigned int i;
        for(i=0;i<geometry.getNumTexCoordArrays();++i)
        {
            add(geometry.getTexCoordArray(i));
        }
        for(i=0;i<geometry.getNumVertexAttribArrays();++i)
        {
            add(geometry.getVertexAttribArray(i));
        }
    }

    void add(osg::Array* array)
    {
        if (array && array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            _arrayList.push_back(array);
        }
    }

    void accept(osg::ArrayVisitor& av)
    {
        for(ArrayList::iterator itr=_arrayList.begin();
            itr!=_arrayList.end();
            ++itr)
        {
            (*itr)->accept(av);
        }
    }

    ArrayList _arrayList;
};

// Compare vertices in a mesh using all their attributes. The vertices
// are identified by their index. Extracted from TriStripVisitor.cpp
struct VertexAttribComparitor : public GeometryArrayGatherer
{
    VertexAttribComparitor(osg::Geometry& geometry)
        : GeometryArrayGatherer(geometry)
    {
    }

    bool operator() (unsigned int lhs, unsigned int rhs) const
    {
        for(ArrayList::const_iterator itr=_arrayList.begin();
            itr!=_arrayList.end();
            ++itr)
        {
            int compare = (*itr)->compare(lhs,rhs);
            if (compare==-1) return true;
            if (compare==1) return false;
        }
        return false;
    }

    int compare(unsigned int lhs, unsigned int rhs)
    {
        for(ArrayList::iterator itr=_arrayList.begin();
            itr!=_arrayList.end();
            ++itr)
        {
            int compare = (*itr)->compare(lhs,rhs);
            if (compare==-1) return -1;
            if (compare==1) return 1;
        }
        return 0;
    }

protected:
    VertexAttribComparitor& operator = (const VertexAttribComparitor&) { return *this; }

};

// Compact the vertex attribute arrays. Also stolen from TriStripVisitor
class RemapArray : public osg::ArrayVisitor
{
    public:
        RemapArray(const IndexList& remapping):_remapping(remapping) {}

        const IndexList& _remapping;

        template<class T>
        inline void remap(T& array)
        {
            for(unsigned int i=0;i<_remapping.size();++i)
            {
                if (i!=_remapping[i])
                {
                    array[i] = array[_remapping[i]];
                }
            }
            array.erase(array.begin()+_remapping.size(),array.end());
        }

        virtual void apply(osg::Array&) {}
        virtual void apply(osg::ByteArray& array) { remap(array); }
        virtual void apply(osg::ShortArray& array) { remap(array); }
        virtual void apply(osg::IntArray& array) { remap(array); }
        virtual void apply(osg::UByteArray& array) { remap(array); }
        virtual void apply(osg::UShortArray& array) { remap(array); }
        virtual void apply(osg::UIntArray& array) { remap(array); }
        virtual void apply(osg::FloatArray& array) { remap(array); }
        virtual void apply(osg::DoubleArray& array) { remap(array); }

        virtual void apply(osg::Vec2Array& array) { remap(array); }
        virtual void apply(osg::Vec3Array& array) { remap(array); }
        virtual void apply(osg::Vec4Array& array) { remap(array); }

        virtual void apply(osg::Vec4ubArray& array) { remap(array); }

        virtual void apply(osg::Vec2bArray& array) { remap(array); }
        virtual void apply(osg::Vec3bArray& array) { remap(array); }
        virtual void apply(osg::Vec4bArray& array) { remap(array); }

        virtual void apply(osg::Vec2sArray& array) { remap(array); }
        virtual void apply(osg::Vec3sArray& array) { remap(array); }
        virtual void apply(osg::Vec4sArray& array) { remap(array); }

        virtual void apply(osg::Vec2dArray& array) { remap(array); }
        virtual void apply(osg::Vec3dArray& array) { remap(array); }
        virtual void apply(osg::Vec4dArray& array) { remap(array); }

        virtual void apply(osg::MatrixfArray& array) { remap(array); }
protected:

        RemapArray& operator = (const RemapArray&) { return *this; }
};


// Construct an index list of triangles for DrawElements for any input
// primitives.
struct MyTriangleOperator
{

    IndexList _remapIndices;
    IndexList _in_indices;

    inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        if (_remapIndices.empty())
        {
            _in_indices.push_back(p1);
            _in_indices.push_back(p2);
            _in_indices.push_back(p3);
        }
        else
        {
            _in_indices.push_back(_remapIndices[p1]);
            _in_indices.push_back(_remapIndices[p2]);
            _in_indices.push_back(_remapIndices[p3]);
        }
    }

};
typedef osg::TriangleIndexFunctor<MyTriangleOperator> MyTriangleIndexFunctor;
}

void IndexMeshVisitor::makeMesh(Geometry& geom)
{
    if (geom.containsDeprecatedData()) geom.fixDeprecatedData();

    if (osg::getBinding(geom.getNormalArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    if (osg::getBinding(geom.getColorArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    if (osg::getBinding(geom.getSecondaryColorArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    if (osg::getBinding(geom.getFogCoordArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    // no point optimizing if we don't have enough vertices.
    if (!geom.getVertexArray() || geom.getVertexArray()->getNumElements()<3) return;

    // check for the existence of surface primitives
    unsigned int numSurfacePrimitives = 0;
    unsigned int numNonIndexedPrimitives = 0;
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
                // For now, only deal with polygons
                return;
        }
        PrimitiveSet::Type type = (*itr)->getType();
        if (!(type == PrimitiveSet::DrawElementsUBytePrimitiveType
              || type == PrimitiveSet::DrawElementsUShortPrimitiveType
              || type == PrimitiveSet::DrawElementsUIntPrimitiveType))
            numNonIndexedPrimitives++;
    }

    // nothing to index
    if (!numSurfacePrimitives || !numNonIndexedPrimitives) return;

    // duplicate shared arrays as it isn't safe to rearrange vertices when arrays are shared.
    if (geom.containsSharedArrays()) geom.duplicateSharedArrays();

    // compute duplicate vertices
    typedef std::vector<unsigned int> IndexList;
    unsigned int numVertices = geom.getVertexArray()->getNumElements();
    IndexList indices(numVertices);
    unsigned int i,j;
    for(i=0;i<numVertices;++i)
    {
        indices[i] = i;
    }

    VertexAttribComparitor arrayComparitor(geom);
    std::sort(indices.begin(),indices.end(),arrayComparitor);

    unsigned int lastUnique = 0;
    unsigned int numUnique = 1;
    for(i=1;i<numVertices;++i)
    {
        if (arrayComparitor.compare(indices[lastUnique],indices[i]) != 0)
        {
            lastUnique = i;
            ++numUnique;
        }

    }
    IndexList remapDuplicatesToOrignals(numVertices);
    lastUnique = 0;
    for(i=1;i<numVertices;++i)
    {
        if (arrayComparitor.compare(indices[lastUnique],indices[i])!=0)
        {
            // found a new vertex entry, so previous run of duplicates needs
            // to be put together.
            unsigned int min_index = indices[lastUnique];
            for(j=lastUnique+1;j<i;++j)
            {
                min_index = osg::minimum(min_index,indices[j]);
            }
            for(j=lastUnique;j<i;++j)
            {
                remapDuplicatesToOrignals[indices[j]]=min_index;
            }
            lastUnique = i;
        }

    }
    unsigned int min_index = indices[lastUnique];
    for(j=lastUnique+1;j<i;++j)
    {
        min_index = osg::minimum(min_index,indices[j]);
    }
    for(j=lastUnique;j<i;++j)
    {
        remapDuplicatesToOrignals[indices[j]]=min_index;
    }


    // copy the arrays.
    IndexList finalMapping(numVertices);
    IndexList copyMapping;
    copyMapping.reserve(numUnique);
    unsigned int currentIndex=0;
    for(i=0;i<numVertices;++i)
    {
        if (remapDuplicatesToOrignals[i]==i)
        {
            finalMapping[i] = currentIndex;
            copyMapping.push_back(i);
            currentIndex++;
        }
    }

    for(i=0;i<numVertices;++i)
    {
        if (remapDuplicatesToOrignals[i]!=i)
        {
            finalMapping[i] = finalMapping[remapDuplicatesToOrignals[i]];
        }
    }


    MyTriangleIndexFunctor taf;
    taf._remapIndices.swap(finalMapping);

    Geometry::PrimitiveSetList new_primitives;
    new_primitives.reserve(primitives.size());

    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        // For now we only have primitive sets that play nicely with
        // the TriangleIndexFunctor.
        (*itr)->accept(taf);
    }

    // remap any shared vertex attributes
    RemapArray ra(copyMapping);
    arrayComparitor.accept(ra);
    if (taf._in_indices.size() < 65536)
    {
        osg::DrawElementsUShort* elements = new DrawElementsUShort(GL_TRIANGLES);
        for (IndexList::iterator itr = taf._in_indices.begin(),
                 end = taf._in_indices.end();
             itr != end;
            ++itr)
        {
            elements->push_back((GLushort)(*itr));
        }
        new_primitives.push_back(elements);
    }
    else
    {
        osg::DrawElementsUInt* elements
            = new DrawElementsUInt(GL_TRIANGLES, taf._in_indices.begin(),
                                   taf._in_indices.end());
        new_primitives.push_back(elements);
    }
    geom.setPrimitiveSetList(new_primitives);
}

void IndexMeshVisitor::makeMesh()
{
    for(GeometryList::iterator itr=_geometryList.begin();
        itr!=_geometryList.end();
        ++itr)
    {
        makeMesh(*(*itr));
    }
}

namespace
{
// A simulation of a Least Recently Used cache. Position in the cache
// corresponds to when the vertex was used i.e., 0 is the most
// recently used.
struct LRUCache
{
    LRUCache(size_t maxSize_) : maxSize(maxSize_)
    {
        entries.reserve(maxSize_ + 3);
    }
    std::vector<unsigned> entries;
    size_t maxSize;

    // Add a list of values to the cache
    void addEntries(unsigned* begin, unsigned* end)
    {
        // If any of the new vertices are already in the cache, remove
        // them, leaving some room at the front of the cache.
        std::vector<unsigned>::reverse_iterator validEnd = entries.rend();
        for (unsigned* pent = begin; pent != end; ++pent)
        {
            validEnd = std::remove(entries.rbegin(), validEnd, *pent);
        }
        // Now make room still needed for new cache entries
        size_t newEnts = end - begin;
        int spaceNeeded = newEnts - (entries.rend() - validEnd);
        if (spaceNeeded > 0)
        {
            if (maxSize - entries.size() >= static_cast<unsigned>(spaceNeeded))
                entries.resize(entries.size() + spaceNeeded);
            else if (entries.size() < maxSize)
                entries.resize(maxSize);
            std::copy_backward(entries.begin() + newEnts - spaceNeeded,
                               entries.end() - spaceNeeded, entries.end());
        }
        std::copy(begin, end, entries.begin());
    }

    bool empty()
    {
        return entries.empty();
    }
};

// A list of triangles that use a vertex is associated with the Vertex
// structure in another array.
struct Vertex
{
    int cachePosition;
    float score;
    int trisUsing;
    int numActiveTris;          // triangles left to process
    size_t triList;             // index to start of triangle storage
    Vertex()
        : cachePosition(-1), score(0.0f), trisUsing(0), numActiveTris(0), triList(0)
    {
    }
};

// Code from Tom Forsyth's article. The algorithm is described in
// detail at
// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html.
// In summary, vertices are assigned a score based on the number of
// triangles that use it (valence) and the vertex's position in the
// model of the vertex cache, if any. Triangles are also given a
// score, which is the sum of the scores of its vertices. The triangle
// with the best score is added to the draw list, its vertices are
// added and / or moved in the cache, the scores of vertices in the
// cache and ejected from the cache are updated. Then the scores of
// triangles that use those vertices are updated.
//
// Unless the cache is empty, the "best" triangle is found by
// searching triangles that use vertices in the cache, not the whole
// mesh. This keeps the algorithm running in time proportional to the
// size of the mesh.

// The "magic" scoring functions are described in the paper.
const float cacheDecayPower = 1.5f;
const float lastTriScore = 0.75f;
const float valenceBoostScale = 2.0f;
const float valenceBoostPower = 0.5f;

const int maxCacheSize = 32;

float findVertexScore (Vertex& vert)
{

    if (vert.numActiveTris == 0)
    {
        // No tri needs this vertex!
        return -1.0f;
    }
    float score = 0.0f;
    int cachePosition = vert.cachePosition;

    if (cachePosition < 0)
    {
        // Vertex is not in FIFO cache - no score.
    }
    else
    {
        if (cachePosition < 3)
        {
            // This vertex was used in the last triangle,
            // so it has a fixed score, whichever of the three
            // it's in. Otherwise, you can get very different
            // answers depending on whether you add
            // the triangle 1,2,3 or 3,1,2 - which is silly.
            score = lastTriScore;
        }
        else
        {
            assert (cachePosition < maxCacheSize);
            // Points for being high in the cache.
            const float scaler = 1.0f / (maxCacheSize - 3);
            score = 1.0f - (cachePosition - 3 ) * scaler;
            score = powf(score, cacheDecayPower);
        }
    }
    // Bonus points for having a low number of tris still to
    // use the vert, so we get rid of lone verts quickly.
    float valenceBoost = powf(vert.numActiveTris, -valenceBoostPower);
    score += valenceBoostScale * valenceBoost;
    return score;
}


typedef std::vector<Vertex> VertexList;

struct Triangle
{
    float score;
    unsigned verts[3];
};

typedef std::vector<Triangle> TriangleList;

inline float findTriangleScore(Triangle& tri, const VertexList& vertices)
{
    float result = 0.0f;
    for (int i = 0; i < 3; ++i)
        result += vertices[tri.verts[i]].score;
    return result;
}

typedef std::pair<unsigned, float> TriangleScore;

TriangleScore computeTriScores(Vertex& vert, const VertexList& vertices,
                               TriangleList& triangles, std::vector<unsigned>& triStore)
{
    float bestScore = 0.0;
    unsigned bestTri = 0;
    for (size_t i = vert.triList;
         i < vert.triList + vert.numActiveTris;
         ++i)
    {
        unsigned tri = triStore[i];
        float score = triangles[tri].score = findTriangleScore(triangles[tri],
                                                               vertices);
        if (score > bestScore)
        {
            bestScore = score;
            bestTri = tri;
        }
    }
    return std::make_pair(bestTri, bestScore);
}

typedef std::vector<Triangle> TriangleList;

struct TriangleCounterOperator
{
    VertexList* vertices;
    int triangleCount;
    TriangleCounterOperator() : vertices(0), triangleCount(0) {}

    void doVertex(unsigned p)
    {
        if (vertices->size() <= p)
            vertices->resize(p + 1);
        (*vertices)[p].trisUsing++;
    }

    void operator() (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        if (p1 == p2 || p2 == p3 || p1 == p3)
            return;
        doVertex(p1);
        doVertex(p2);
        doVertex(p3);
        triangleCount++;
    }
};

struct TriangleCounter : public TriangleIndexFunctor<TriangleCounterOperator>
{
    TriangleCounter(std::vector<Vertex>* vertices_)
    {
        vertices = vertices_;
    }
};

// Initialize the vertex triangle lists and the triangle data structures
struct TriangleAddOperator
{
    VertexList* vertices;
    std::vector<unsigned>* vertexTris;
    TriangleList* triangles;
    int triIdx;
    TriangleAddOperator() : vertices(0), triIdx(0) {}

    void doVertex(unsigned p)
    {
        (*vertexTris)[(*vertices)[p].triList + (*vertices)[p].numActiveTris++]
            = triIdx;
    }

    void operator() (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        if (p1 == p2 || p2 == p3 || p1 == p3)
            return;
        doVertex(p1);
        doVertex(p2);
        doVertex(p3);
    (*triangles)[triIdx].verts[0] = p1;
    (*triangles)[triIdx].verts[1] = p2;
    (*triangles)[triIdx].verts[2] = p3;
    triIdx++;
    }
};

struct TriangleAdder : public TriangleIndexFunctor<TriangleAddOperator>
{
    TriangleAdder(VertexList* vertices_, std::vector<unsigned>* vertexTris_,
                  TriangleList* triangles_)
    {
        vertices = vertices_;
        vertexTris = vertexTris_;
        triangles = triangles_;
    }
};

struct CompareTriangle
{
    bool operator()(const Triangle& lhs, const Triangle& rhs)
    {
        return lhs.score < rhs.score;
    }
};
}

void VertexCacheVisitor::optimizeVertices(Geometry& geom)
{
    Array* vertArray = geom.getVertexArray();
    if (!vertArray)
        return;
    unsigned vertArraySize = vertArray->getNumElements();
    // If all the vertices fit in the cache, there's no point in
    // doing this optimization.
    if (vertArraySize <= 16)
        return;
    Geometry::PrimitiveSetList& primSets = geom.getPrimitiveSetList();
    for (Geometry::PrimitiveSetList::iterator itr = primSets.begin(),
             end = primSets.end();
         itr != end;
         ++itr)
    {
        // Can only deal with polygons.
        switch ((*itr)->getMode())
        {
        case(PrimitiveSet::TRIANGLES):
        case(PrimitiveSet::TRIANGLE_STRIP):
        case(PrimitiveSet::TRIANGLE_FAN):
        case(PrimitiveSet::QUADS):
        case(PrimitiveSet::QUAD_STRIP):
        case(PrimitiveSet::POLYGON):
            break;
        default:
            return;
        }
        PrimitiveSet::Type type = (*itr)->getType();
        if (type != PrimitiveSet::DrawElementsUBytePrimitiveType
            && type != PrimitiveSet::DrawElementsUShortPrimitiveType
            && type != PrimitiveSet::DrawElementsUIntPrimitiveType)
            return;
    }
#if 0
    VertexCacheMissVisitor missv;
    missv.doGeometry(geom);
    cout << "Before optimization: misses: " << missv.misses
         << " triangles: " << missv.triangles << " acmr: ";
    if (missv.triangles > 0.0)
        cout << (double)missv.misses / (double)missv.triangles << "\n";
    else
        cout << "0.0\n";
    missv.reset();
#endif
    std::vector<unsigned> newVertList;
    doVertexOptimization(geom, newVertList);
    Geometry::PrimitiveSetList newPrims;
    if (vertArraySize < 65536)
    {
        osg::DrawElementsUShort* elements = new DrawElementsUShort(GL_TRIANGLES);
        elements->reserve(newVertList.size());
        for (std::vector<unsigned>::iterator itr = newVertList.begin(),
                 end = newVertList.end();
             itr != end;
             ++itr)
            elements->push_back((GLushort)*itr);
        if (geom.getUseVertexBufferObjects())
        {
            elements->setElementBufferObject(new ElementBufferObject);
        }
        newPrims.push_back(elements);
    }
    else
    {
        osg::DrawElementsUInt* elements
            = new DrawElementsUInt(GL_TRIANGLES, newVertList.begin(),
                                   newVertList.end());
        if (geom.getUseVertexBufferObjects())
        {
            elements->setElementBufferObject(new ElementBufferObject);
        }
        newPrims.push_back(elements);
    }

    geom.setPrimitiveSetList(newPrims);
#if 0
    missv.doGeometry(geom);
    cout << "After optimization: misses: " << missv.misses
         << " triangles: " << missv.triangles << " acmr: ";
    if (missv.triangles > 0.0)
        cout << (double)missv.misses / (double)missv.triangles << "\n";
    else
        cout << "0.0\n";
#endif
    geom.dirtyDisplayList();
}

// The main optimization loop
void VertexCacheVisitor::doVertexOptimization(Geometry& geom,
                                              std::vector<unsigned>& vertDrawList)
{
    Geometry::PrimitiveSetList& primSets = geom.getPrimitiveSetList();
    // lists for all the vertices and triangles
    VertexList vertices;
    TriangleList triangles;
    TriangleCounter triCounter(&vertices);
    for (Geometry::PrimitiveSetList::iterator itr = primSets.begin(),
             end = primSets.end();
         itr != end;
         ++itr)
        (*itr)->accept(triCounter);
    triangles.resize(triCounter.triangleCount);
    // Get total of triangles used by all the vertices
    size_t vertTrisSize = 0;
    for (VertexList::iterator itr = vertices.begin(), end = vertices.end();
         itr != end;
         ++itr)
    {
        itr->triList = vertTrisSize;
        vertTrisSize += itr->trisUsing;
    }
    // Store for lists of triangles (indices) used by the vertices
    std::vector<unsigned> vertTriListStore(vertTrisSize);
    TriangleAdder triAdder(&vertices, &vertTriListStore, &triangles);
    for (Geometry::PrimitiveSetList::iterator itr = primSets.begin(),
             end = primSets.end();
         itr != end;
         ++itr)
        (*itr)->accept(triAdder);
    // Set up initial scores for vertices and triangles
    for (VertexList::iterator itr = vertices.begin(), end = vertices.end();
         itr != end;
         ++itr)
        itr->score = findVertexScore(*itr);
    for (TriangleList::iterator itr = triangles.begin(), end = triangles.end();
         itr != end;
         ++itr)
    {
        itr->score = 0.0f;
        for (int i = 0; i < 3; ++i)
            itr->score += vertices[itr->verts[i]].score;
    }
    // Add Triangles to the draw list until there are no more.
    unsigned trisLeft = triangles.size();
    vertDrawList.reserve(trisLeft * 3);
    LRUCache cache(maxCacheSize);
    while (trisLeft-- > 0)
    {
        Triangle* triToAdd = 0;
        float bestScore = 0.0;
        for (std::vector<unsigned>::const_iterator itr = cache.entries.begin(),
                 end = cache.entries.end();
             itr != end;
             ++itr)
        {
            TriangleScore tscore =  computeTriScores(vertices[*itr], vertices,
                                                     triangles, vertTriListStore);
            if (tscore.second > bestScore)
            {
                bestScore = tscore.second;
                triToAdd = &triangles[tscore.first];
            }
        }
        if (!triToAdd)
        {
            // The cache was empty, or all the triangles that use
            // vertices in the cache have already been added.
            OSG_DEBUG << "VertexCacheVisitor searching all triangles" << std::endl;
            TriangleList::iterator maxItr
                = std::max_element(triangles.begin(), triangles.end(),
                              CompareTriangle());
            triToAdd = &(*maxItr);
        }
        assert(triToAdd != 0 && triToAdd->score > 0.0);
        // Add triangle vertices, and remove triangle from the
        // vertices that use it.
        triToAdd->score = -1.0f;
        unsigned triToAddIdx = triToAdd - &triangles[0];
        for (unsigned i = 0; i < 3; ++i)
        {
            unsigned vertIdx = triToAdd->verts[i];
            Vertex* vert = &vertices[vertIdx];
            vertDrawList.push_back(vertIdx);
            std::remove(vertTriListStore.begin() + vert->triList,
                   vertTriListStore.begin() + vert->triList
                   + vert->numActiveTris,
                   triToAddIdx);
            vert->numActiveTris--;
        }
        // Assume that the three oldest cache entries will get kicked
        // out, so reset their scores and those of their
        // triangles. Otherwise we'll "lose" them and not be able to
        // change their scores.
        if (cache.maxSize - cache.entries.size() < 3)
        {
            for (std::vector<unsigned>::iterator itr = cache.entries.end() - 3,
                     end = cache.entries.end();
                 itr != end;
                ++itr)
            {
                vertices[*itr].cachePosition = -1;
                vertices[*itr].score = findVertexScore(vertices[*itr]);
            }
            for (std::vector<unsigned>::iterator itr = cache.entries.end() - 3,
                     end = cache.entries.end();
                 itr != end;
                ++itr)
                computeTriScores(vertices[*itr], vertices, triangles,
                                 vertTriListStore);
        }
        cache.addEntries(&triToAdd->verts[0], &triToAdd->verts[3]);
        for (std::vector<unsigned>::const_iterator itr = cache.entries.begin(),
                 end = cache.entries.end();
             itr != end;
             ++itr)
        {
            unsigned vertIdx = *itr;
            vertices[vertIdx].cachePosition = itr - cache.entries.begin();
            vertices[vertIdx].score = findVertexScore(vertices[vertIdx]);
        }
     }
}

void VertexCacheVisitor::optimizeVertices()
{
    for(GeometryList::iterator itr=_geometryList.begin();
        itr!=_geometryList.end();
        ++itr)
    {
        optimizeVertices(*(*itr));
    }
}

VertexCacheMissVisitor::VertexCacheMissVisitor(unsigned cacheSize)
    : osg::NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN), misses(0),
      triangles(0), _cacheSize(cacheSize)
{
}

void VertexCacheMissVisitor::reset()
{
    misses = 0;
    triangles = 0;
}

void VertexCacheMissVisitor::apply(Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); ++i )
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom)
            doGeometry(*geom);
    }
}

namespace
{
// The cache miss algorithm models an LRU cache because it results in an
// order that is insensitive to the actual cache size of the GPU. Real
// GPUs use a FIFO cache, so the statistics gatherer simulates that.
struct FIFOCache
{
    FIFOCache(size_t maxSize_) : maxSize(maxSize_)
    {
        entries.reserve(maxSize_);
    }
    std::vector<unsigned> entries;
    size_t maxSize;

    // Add a list of values to the cache
    void addEntries(unsigned* begin, unsigned* end)
    {
        // Make room for new cache entries
        size_t newEnts = end - begin;
        if (entries.size() < maxSize)
            entries.resize(osg::minimum(entries.size() + newEnts, maxSize));
        std::vector<unsigned>::iterator copyEnd = entries.end() - newEnts;
        std::copy_backward(entries.begin(), copyEnd, entries.end());
        std::copy(begin, end, entries.begin());
    }

    bool empty()
    {
        return entries.empty();
    }
};

// Insert vertices in a cache and record cache misses
struct CacheRecordOperator
{
    CacheRecordOperator() : cache(0), misses(0), triangles(0) {}
    FIFOCache* cache;
    unsigned misses;
    unsigned triangles;
    void operator()(unsigned p1, unsigned p2, unsigned p3)
    {
        unsigned verts[3];
        verts[0] = p1;
        verts[1] = p2;
        verts[2] = p3;
        triangles++;
        for (int i = 0; i < 3; ++i)
        {
            if (std::find(cache->entries.begin(), cache->entries.end(), verts[i])
                == cache->entries.end())
                misses++;
        }
        cache->addEntries(&verts[0], &verts[3]);
    }
};

struct CacheRecorder : public TriangleIndexFunctor<CacheRecordOperator>
{
    CacheRecorder(unsigned cacheSize)
    {
        cache = new FIFOCache(cacheSize);
    }

    ~CacheRecorder()
    {
        delete cache;
    }
};
}

void VertexCacheMissVisitor::doGeometry(Geometry& geom)
{
    Array* vertArray = geom.getVertexArray();
    if (!vertArray || vertArray->getNumElements()==0)
        return;
    Geometry::PrimitiveSetList& primSets = geom.getPrimitiveSetList();
    CacheRecorder recorder(_cacheSize);
    for (Geometry::PrimitiveSetList::iterator itr = primSets.begin(),
             end = primSets.end();
         itr != end;
         ++itr)
    {
        (*itr)->accept(recorder);
    }
    misses += recorder.misses;
    triangles += recorder.triangles;
}

namespace
{
// Move the values in an array to new positions, based on the
// remapping table. remapping[i] contains element i's new position, if
// any.  Unlike RemapArray in TriStripVisitor, this code doesn't
// assume that elements only move downward in the array.
class Remapper : public osg::ArrayVisitor
{
public:
    static const unsigned invalidIndex;
    Remapper(const std::vector<unsigned>& remapping)
        : _remapping(remapping), _newsize(0)
    {
        for (std::vector<unsigned>::const_iterator itr = _remapping.begin(),
                 end = _remapping.end();
             itr != end;
             ++itr)
            if (*itr != invalidIndex)
                ++_newsize;
    }

    const std::vector<unsigned>& _remapping;
    size_t _newsize;

    template<class T>
    inline void remap(T& array)
    {
        ref_ptr<T> newarray = new T(_newsize);
        T* newptr = newarray.get();
        for (size_t i = 0; i < array.size(); ++i)
            if (_remapping[i] != invalidIndex)
                (*newptr)[_remapping[i]] = array[i];
        array.swap(*newptr);

    }

    virtual void apply(osg::Array&) {}
    virtual void apply(osg::ByteArray& array) { remap(array); }
    virtual void apply(osg::ShortArray& array) { remap(array); }
    virtual void apply(osg::IntArray& array) { remap(array); }
    virtual void apply(osg::UByteArray& array) { remap(array); }
    virtual void apply(osg::UShortArray& array) { remap(array); }
    virtual void apply(osg::UIntArray& array) { remap(array); }
    virtual void apply(osg::FloatArray& array) { remap(array); }
    virtual void apply(osg::DoubleArray& array) { remap(array); }

    virtual void apply(osg::Vec2Array& array) { remap(array); }
    virtual void apply(osg::Vec3Array& array) { remap(array); }
    virtual void apply(osg::Vec4Array& array) { remap(array); }

    virtual void apply(osg::Vec4ubArray& array) { remap(array); }

    virtual void apply(osg::Vec2bArray& array) { remap(array); }
    virtual void apply(osg::Vec3bArray& array) { remap(array); }
    virtual void apply(osg::Vec4bArray& array) { remap(array); }

    virtual void apply(osg::Vec2sArray& array) { remap(array); }
    virtual void apply(osg::Vec3sArray& array) { remap(array); }
    virtual void apply(osg::Vec4sArray& array) { remap(array); }

    virtual void apply(osg::Vec2dArray& array) { remap(array); }
    virtual void apply(osg::Vec3dArray& array) { remap(array); }
    virtual void apply(osg::Vec4dArray& array) { remap(array); }

    virtual void apply(osg::MatrixfArray& array) { remap(array); }
};

const unsigned Remapper::invalidIndex = std::numeric_limits<unsigned>::max();

// Record the order in which vertices in a Geometry are used in triangle, line or point primitives.
struct VertexReorderOperator
{
    unsigned seq;
    std::vector<unsigned int> remap;

    VertexReorderOperator() : seq(0)
    {
    }

    void inline doVertex(unsigned v)
    {
        if (remap[v] == Remapper::invalidIndex) {
            remap[v] = seq ++;
        }
    }

    void operator()(unsigned p1, unsigned p2, unsigned p3)
    {
        doVertex(p1);
        doVertex(p2);
        doVertex(p3);
    }

    void operator()(unsigned p1, unsigned p2)
    {
        doVertex(p1);
        doVertex(p2);
    }

    void operator()(unsigned p1)
    {
        doVertex(p1);
    }
};

struct VertexReorder : public TriangleLinePointIndexFunctor<osgUtil::VertexReorderOperator>
{
    VertexReorder(unsigned numVerts)
    {
        remap.resize(numVerts, Remapper::invalidIndex);
    }
};
}

void VertexAccessOrderVisitor::optimizeOrder()
{
    for (GeometryList::iterator itr = _geometryList.begin(), end = _geometryList.end();
         itr != end;
         ++itr)
    {
        optimizeOrder(*(*itr));
    }
}

template<typename DE>
inline void reorderDrawElements(DE& drawElements,
                                const std::vector<unsigned>& reorder)
{
    for (typename DE::iterator itr = drawElements.begin(), end = drawElements.end();
         itr != end;
         ++itr)
    {
        *itr = static_cast<typename DE::value_type>(reorder[*itr]);
    }
}

void VertexAccessOrderVisitor::optimizeOrder(Geometry& geom)
{
    Array* vertArray = geom.getVertexArray();
    if (!vertArray || vertArray->getNumElements()==0)
        return;

    Geometry::PrimitiveSetList& primSets = geom.getPrimitiveSetList();

    // sort primitives: first triangles, then lines and finally points
    std::sort(primSets.begin(), primSets.end(), order_by_primitive_mode);

    VertexReorder vr(vertArray->getNumElements());
    for (Geometry::PrimitiveSetList::iterator itr = primSets.begin(),
             end = primSets.end();
         itr != end;
         ++itr)
    {
        PrimitiveSet* ps = itr->get();
        PrimitiveSet::Type type = ps->getType();
        if (type != PrimitiveSet::DrawElementsUBytePrimitiveType
            && type != PrimitiveSet::DrawElementsUShortPrimitiveType
            && type != PrimitiveSet::DrawElementsUIntPrimitiveType)
            return;
        ps->accept(vr);
    }

    // search for UVs array shared only within the geometry
    SharedArrayOptimizer deduplicator;
    deduplicator.findDuplicatedUVs(geom);

    // duplicate shared arrays as it isn't safe to rearrange vertices when arrays are shared.
    if (geom.containsSharedArrays()) geom.duplicateSharedArrays();
    GeometryArrayGatherer gatherer(geom);

    Remapper remapper(vr.remap);
    gatherer.accept(remapper);
    for (Geometry::PrimitiveSetList::iterator itr = primSets.begin(),
             end = primSets.end();
         itr != end;
         ++itr)
    {
        PrimitiveSet* ps = itr->get();
        switch (ps->getType())
        {
        case PrimitiveSet::DrawElementsUBytePrimitiveType:
            reorderDrawElements(*static_cast<DrawElementsUByte*>(ps), vr.remap);
            break;
        case PrimitiveSet::DrawElementsUShortPrimitiveType:
            reorderDrawElements(*static_cast<DrawElementsUShort*>(ps), vr.remap);
            break;
        case PrimitiveSet::DrawElementsUIntPrimitiveType:
            reorderDrawElements(*static_cast<DrawElementsUInt*>(ps), vr.remap);
            break;
        default:
            break;
        }
    }

    // deduplicate UVs array that were only shared within the geometry
    deduplicator.deduplicateUVs(geom);

    geom.dirtyDisplayList();
}

void SharedArrayOptimizer::findDuplicatedUVs(const osg::Geometry& geometry)
{
    _deduplicateUvs.clear();

    // look for all channels that are shared only *within* the geometry
    std::map<const osg::Array*, unsigned int> arrayPointerCounter;

    for(unsigned int id = 0 ; id < geometry.getNumTexCoordArrays() ; ++ id)
    {
        const osg::Array* channel = geometry.getTexCoordArray(id);
        if(channel && channel->getNumElements())
        {
            if(arrayPointerCounter.find(channel) == arrayPointerCounter.end())
            {
                arrayPointerCounter[channel] = 1;
            }
            else
            {
                arrayPointerCounter[channel] += 1;
            }
        }
    }

    std::map<const osg::Array*, unsigned int> references;

    for(unsigned int id = 0 ; id != geometry.getNumTexCoordArrays() ; ++ id)
    {
        const osg::Array* channel = geometry.getTexCoordArray(id);
        // test if array is shared outside the geometry
        if(channel && static_cast<unsigned int>(channel->referenceCount()) == arrayPointerCounter[channel])
        {
            std::map<const osg::Array*, unsigned int>::const_iterator reference = references.find(channel);
            if(reference == references.end())
            {
                references[channel] = id;
            }
            else
            {
                _deduplicateUvs[id] = reference->second;
            }
        }
    }
}

void SharedArrayOptimizer::deduplicateUVs(osg::Geometry& geometry)
{
    for(std::map<unsigned int, unsigned int>::const_iterator it_duplicate = _deduplicateUvs.begin() ;
        it_duplicate != _deduplicateUvs.end() ; ++ it_duplicate)
    {
        osg::Array* original = geometry.getTexCoordArray(it_duplicate->second);
        geometry.setTexCoordArray(it_duplicate->first,
                                  original,
                                  (original ? original->getBinding() : osg::Array::BIND_UNDEFINED));
    }
}

}
