
#ifndef NV_TRISTRIP_OBJECTS_H
#define NV_TRISTRIP_OBJECTS_H

#include <assert.h>
#include <vector>
#include <list>

#include <osg/MemoryManager>

/////////////////////////////////////////////////////////////////////////////////
//
// Types defined for stripification
//
/////////////////////////////////////////////////////////////////////////////////

typedef int nvWORD;
typedef unsigned int UINT;

struct MyVertex
{
    float x, y, z;
    float nx, ny, nz;
};

typedef MyVertex MyVector;

struct MyFace
{
    int v1, v2, v3;
    float nx, ny, nz;
};

class VertexCache
{

    public:

        VertexCache(int size);
        VertexCache();
        ~VertexCache();

        bool InCache(int entry);
        int AddEntry(int entry);
        void Clear();

        void Copy(VertexCache* inVcache);
        int At(int index);
        void Set(int index, int value);

    private:

        int *entries;
        int numEntries;

};

inline bool VertexCache::InCache(int entry)
{
    bool returnVal = false;

    for(int i = 0; i < numEntries; i++)
    {
        if(entries[i] == entry)
        {
            returnVal = true;
            break;
        }
    }

    return returnVal;
}


inline int VertexCache::AddEntry(int entry)
{
    int removed;

    removed = entries[numEntries - 1];

    //push everything right one
    for(int i = numEntries - 2; i >= 0; i--)
    {
        entries[i + 1] = entries[i];
    }

    entries[0] = entry;

    return removed;
}


class NvFaceInfo
{
    public:

        // vertex indices
        NvFaceInfo(int v0, int v1, int v2)
        {
            m_v0 = v0; m_v1 = v1; m_v2 = v2;
            m_stripId      = -1;
            m_testStripId  = -1;
            m_experimentId = -1;
        }

        // data members are left public
        int   m_v0, m_v1, m_v2;
        int   m_stripId;         // real strip Id
        int   m_testStripId;     // strip Id in an experiment
        int   m_experimentId;    // in what experiment was it given an experiment Id?
};

// nice and dumb edge class that points knows its
// indices, the two faces, and the next edge using
// the lesser of the indices
class NvEdgeInfo
{
    public:

        // constructor puts 1 ref on us
        NvEdgeInfo (int v0, int v1)
        {
            m_v0       = v0;
            m_v1       = v1;
            m_face0    = NULL;
            m_face1    = NULL;
            m_nextV0   = NULL;
            m_nextV1   = NULL;

            // we will appear in 2 lists.  this is a good
            // way to make sure we delete it the second time
            // we hit it in the edge infos
            m_refCount = 2;

        }

        // ref and unref
        void Unref () { if (--m_refCount == 0) osgDelete this; }

        // data members are left public
        UINT         m_refCount;
        NvFaceInfo  *m_face0, *m_face1;
        int          m_v0, m_v1;
        NvEdgeInfo  *m_nextV0, *m_nextV1;
};

// This class is a quick summary of parameters used
// to begin a triangle strip.  Some operations may
// want to create lists of such items, so they were
// pulled out into a class
class NvStripStartInfo
{
    public:
        NvStripStartInfo(NvFaceInfo *startFace, NvEdgeInfo *startEdge, bool toV1)
        {
            m_startFace    = startFace;
            m_startEdge    = startEdge;
            m_toV1         = toV1;
        }
        NvFaceInfo    *m_startFace;
        NvEdgeInfo    *m_startEdge;
        bool           m_toV1;
};

typedef std::vector<NvFaceInfo*>     NvFaceInfoVec;
typedef std::list  <NvFaceInfo*>     NvFaceInfoList;
typedef std::list  <NvFaceInfoVec*>  NvStripList;
typedef std::vector<NvEdgeInfo*>     NvEdgeInfoVec;

typedef std::vector<nvWORD> WordVec;
typedef std::vector<MyVertex> MyVertexVec;
typedef std::vector<MyFace> MyFaceVec;

// This is a summary of a strip that has been built
class NvStripInfo
{
    public:

        // A little information about the creation of the triangle strips
        NvStripInfo(const NvStripStartInfo &startInfo, int stripId, int experimentId = -1) :
        m_startInfo(startInfo)
        {
            m_stripId      = stripId;
            m_experimentId = experimentId;
            visited = false;
        }

        // This is an experiment if the experiment id is >= 0
        inline bool IsExperiment () const { return m_experimentId >= 0; }

        inline bool IsInStrip (const NvFaceInfo *faceInfo) const
        {
            if(faceInfo == NULL)
                return false;

            return (m_experimentId >= 0 ? faceInfo->m_testStripId == m_stripId : faceInfo->m_stripId == m_stripId);
        }

        bool SharesEdge(const NvFaceInfo* faceInfo, NvEdgeInfoVec &edgeInfos);

        // take the given forward and backward strips and combine them together
        void Combine(const NvFaceInfoVec &forward, const NvFaceInfoVec &backward);

        //returns true if the face is "unique", i.e. has a vertex which doesn't exist in the faceVec
        bool Unique(NvFaceInfoVec& faceVec, NvFaceInfo* face);

        // mark the triangle as taken by this strip
        bool IsMarked    (NvFaceInfo *faceInfo);
        void MarkTriangle(NvFaceInfo *faceInfo);

        // build the strip
        void Build(NvEdgeInfoVec &edgeInfos, NvFaceInfoVec &faceInfos);

        // public data members
        NvStripStartInfo m_startInfo;
        NvFaceInfoVec    m_faces;
        int              m_stripId;
        int              m_experimentId;

        bool visited;
};

typedef std::vector<NvStripInfo*>    NvStripInfoVec;

//The actual stripifier
class NvStripifier
{
    public:

        // Constructor
        NvStripifier();
        ~NvStripifier();

        //the target vertex cache size, the structure to place the strips in, and the input indices
        void Stripify(const WordVec &in_indices, const int in_numVertices, const int in_cacheSize, const int in_minStripLength,
            NvStripInfoVec &allStrips, NvFaceInfoVec &allFaces);

        static int GetUniqueVertexInB(NvFaceInfo *faceA, NvFaceInfo *faceB);
        static int GetSharedVertex(NvFaceInfo *faceA, NvFaceInfo *faceB);

        static bool NextIsCW(const int numIndices);
        static bool IsCW(NvFaceInfo *faceInfo, int v0, int v1);

        void CreateStrips(NvStripInfoVec& strips,
            NvFaceInfoVec& leftoverFaces,
            WordVec& stripIndices);

        void OptimizeVertices(NvStripInfoVec& strips,
            NvFaceInfoVec& leftoverFaces,
            WordVec& stripIndices,
            MyVertexVec& vertices,
            MyVertexVec& optimizedVerts);

    protected:

        WordVec indices;
        int cacheSize;
        unsigned int minStripLength;
        float meshJump;
        bool bFirstTimeResetPoint;

        /////////////////////////////////////////////////////////////////////////////////
        //
        // Big mess of functions called during stripification
        //
        /////////////////////////////////////////////////////////////////////////////////

        static int  GetNextIndex(const WordVec &indices, NvFaceInfo *face);
        static NvEdgeInfo *FindEdgeInfo(NvEdgeInfoVec &edgeInfos, int v0, int v1);
        static NvFaceInfo *FindOtherFace(NvEdgeInfoVec &edgeInfos, int v0, int v1, NvFaceInfo *faceInfo);
        NvFaceInfo *FindGoodResetPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos);

        void FindAllStrips(NvStripInfoVec &allStrips, NvFaceInfoVec &allFaceInfos, NvEdgeInfoVec &allEdgeInfos, int numSamples);
        void SplitUpStripsAndOptimize(NvStripInfoVec &allStrips, NvStripInfoVec &outStrips, NvEdgeInfoVec& edgeInfos, NvFaceInfoVec& outFaceList);
        void RemoveSmallStrips(NvStripInfoVec& allStrips, NvStripInfoVec& allBigStrips, NvFaceInfoVec& faceList);

        bool FindTraversal(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, NvStripInfo *strip, NvStripStartInfo &startInfo);
        int  CountRemainingTris(std::list<NvStripInfo*>::iterator iter, std::list<NvStripInfo*>::iterator  end);

        void CommitStrips(NvStripInfoVec &allStrips, const NvStripInfoVec &strips);

        float AvgStripSize(const NvStripInfoVec &strips);
        int FindStartPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos);

        void UpdateCacheStrip(VertexCache* vcache, NvStripInfo* strip);
        void UpdateCacheFace(VertexCache* vcache, NvFaceInfo* face);
        float CalcNumHitsStrip(VertexCache* vcache, NvStripInfo* strip);
        int CalcNumHitsFace(VertexCache* vcache, NvFaceInfo* face);
        int NumNeighbors(NvFaceInfo* face, NvEdgeInfoVec& edgeInfoVec);

        void BuildStripifyInfo(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, const int numVertices);
        bool AlreadyExists(NvFaceInfo* faceInfo, NvFaceInfoVec& faceInfos);

        // let our strip info classes and the other classes get
        // to these protected stripificaton methods if they want
        friend class NvStripInfo;
};
#endif
