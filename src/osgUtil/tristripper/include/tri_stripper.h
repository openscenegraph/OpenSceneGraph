
//////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004 Tanguy Fautré.
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//  Tanguy Fautré
//  softdev@telenet.be
//
//////////////////////////////////////////////////////////////////////
//
//                            Tri Stripper
//                            ************
//
// Post TnL cache aware triangle stripifier in O(n.log(n)).
//          
// History: see ChangeLog
//
//////////////////////////////////////////////////////////////////////
// SVN: $Id: tri_stripper.h 94 2009-11-24 20:08:20Z gpsnoopy $
//////////////////////////////////////////////////////////////////////

#ifndef TRI_STRIPPER_HEADER_GUARD_TRI_STRIPPER_H
#define TRI_STRIPPER_HEADER_GUARD_TRI_STRIPPER_H

#include "public_types.h"

#include "detail/cache_simulator.h"
#include "detail/graph_array.h"
#include "detail/heap_array.h"
#include "detail/types.h"




namespace triangle_stripper {




class tri_stripper
{
public:

    explicit tri_stripper(const indices & TriIndices);

    void Strip(primitive_vector * out_pPrimitivesVector);

    /* Stripifier Algorithm Settings */
    
    // Set the post-T&L cache size (0 disables the cache optimizer).
    void SetCacheSize(size_t CacheSize = 10);

    // Set the minimum size of a triangle strip (should be at least 2 triangles).
    // The stripifier discard any candidate strips that does not satisfy the minimum size condition.
    void SetMinStripSize(size_t MinStripSize = 2);

    // Set the backward search mode in addition to the forward search mode.
    // In forward mode, the candidate strips are build with the current candidate triangle being the first
    // triangle of the strip. When the backward mode is enabled, the stripifier also tests candidate strips
    // where the current candidate triangle is the last triangle of the strip.
    // Enable this if you want better results at the expense of being slightly slower.
    // Note: Do *NOT* use this when the cache optimizer is enabled; it only gives worse results.
    void SetBackwardSearch(bool Enabled = false);
    
    // Set the cache simulator FIFO behavior (does nothing if the cache optimizer is disabled).
    // When enabled, the cache is simulated as a simple FIFO structure. However, when
    // disabled, indices that trigger cache hits are not pushed into the FIFO structure.
    // This allows simulating some GPUs that do not duplicate cache entries (e.g. NV25 or greater).
    void SetPushCacheHits(bool Enabled = true);

    /* End Settings */

private:

    typedef detail::graph_array<detail::triangle> triangle_graph;
    typedef detail::heap_array<size_t, std::greater<size_t> > triangle_heap;
    typedef std::vector<size_t> candidates;
    typedef triangle_graph::node_iterator tri_iterator;
    typedef triangle_graph::const_node_iterator const_tri_iterator;
    typedef triangle_graph::out_arc_iterator link_iterator;
    typedef triangle_graph::const_out_arc_iterator const_link_iterator;

    void InitTriHeap();
    void Stripify();
    void AddLeftTriangles();
    void ResetStripIDs();

    detail::strip FindBestStrip();
    detail::strip ExtendToStrip(size_t Start, detail::triangle_order Order);
    detail::strip BackExtendToStrip(size_t Start, detail::triangle_order Order, bool ClockWise);
    const_link_iterator LinkToNeighbour(const_tri_iterator Node, bool ClockWise, detail::triangle_order & Order, bool NotSimulation);
    const_link_iterator BackLinkToNeighbour(const_tri_iterator Node, bool ClockWise, detail::triangle_order & Order);
    void BuildStrip(const detail::strip Strip);
    void MarkTriAsTaken(size_t i);
    void AddIndex(index i, bool NotSimulation);
    void BackAddIndex(index i);
    void AddTriangle(const detail::triangle & Tri, detail::triangle_order Order, bool NotSimulation);
    void BackAddTriangle(const detail::triangle & Tri, detail::triangle_order Order);

    bool Cache() const;
    size_t CacheSize() const;

    static detail::triangle_edge FirstEdge(const detail::triangle & Triangle, detail::triangle_order Order);
    static detail::triangle_edge LastEdge(const detail::triangle & Triangle, detail::triangle_order Order);

    primitive_vector            m_PrimitivesVector;
    triangle_graph                m_Triangles;
    triangle_heap                m_TriHeap;
    candidates                    m_Candidates;
    detail::cache_simulator        m_Cache;
    detail::cache_simulator        m_BackCache;
    size_t                        m_StripID;
    size_t                        m_MinStripSize;
    bool                        m_BackwardSearch;
    bool                        m_FirstRun;
};





//////////////////////////////////////////////////////////////////////////
// tri_stripper inline functions
//////////////////////////////////////////////////////////////////////////

inline void tri_stripper::SetCacheSize(const size_t CacheSize)
{
    m_Cache.resize(CacheSize);
    m_BackCache.resize(CacheSize);
}


inline void tri_stripper::SetMinStripSize(const size_t MinStripSize)
{
    if (MinStripSize < 2)
        m_MinStripSize = 2;
    else
        m_MinStripSize = MinStripSize;
}


inline void tri_stripper::SetBackwardSearch(const bool Enabled)
{
    m_BackwardSearch = Enabled;
}



inline void tri_stripper::SetPushCacheHits(bool Enabled)
{
    m_Cache.push_cache_hits(Enabled);
}




} // namespace triangle_stripper




#endif // TRI_STRIPPER_HEADER_GUARD_TRI_STRIPPER_H
