//
// Copyright (C) 2004 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in tri_stripper.h
//
//////////////////////////////////////////////////////////////////////
// SVN: $Id: cache_simulator.h 93 2009-11-24 20:01:19Z gpsnoopy $
//////////////////////////////////////////////////////////////////////

#ifndef TRI_STRIPPER_HEADER_GUARD_CACHE_SIMULATOR_H
#define TRI_STRIPPER_HEADER_GUARD_CACHE_SIMULATOR_H

#include <algorithm>
#include <limits>
#include <deque>




namespace triangle_stripper {

    namespace detail {




class cache_simulator
{
public:
    cache_simulator();

    void clear();
    void resize(size_t Size);
    void reset();
    void push_cache_hits(bool Enabled = true);
    size_t size() const;

    void push(index i, bool CountCacheHit = false);
    void merge(const cache_simulator & Backward, size_t PossibleOverlap);

    void reset_hitcount();
    size_t hitcount() const;

protected:
    typedef std::deque<index> indices_deque;

    indices_deque    m_Cache;
    size_t            m_NbHits;
    bool            m_PushHits;
};





//////////////////////////////////////////////////////////////////////////
// cache_simulator inline functions
//////////////////////////////////////////////////////////////////////////

inline cache_simulator::cache_simulator()
    : m_NbHits(0),
      m_PushHits(true)
{

}


inline void cache_simulator::clear()
{
    reset_hitcount();
    m_Cache.clear();
}


inline void cache_simulator::resize(const size_t Size)
{
    m_Cache.resize(Size, (std::numeric_limits<index>::max)());
}


inline void cache_simulator::reset()
{
    std::fill(m_Cache.begin(), m_Cache.end(), (std::numeric_limits<index>::max)());
    reset_hitcount();
}


inline void cache_simulator::push_cache_hits(bool Enabled)
{
    m_PushHits = Enabled;
}


inline size_t cache_simulator::size() const
{
    return m_Cache.size();
}


inline void cache_simulator::push(const index i, const bool CountCacheHit)
{
    if (CountCacheHit || m_PushHits) {

        if (std::find(m_Cache.begin(), m_Cache.end(), i) != m_Cache.end()) {

            // Should we count the cache hits?
            if (CountCacheHit)
                ++m_NbHits;
            
            // Should we not push the index into the cache if it's a cache hit?
            if (! m_PushHits)
                return;
        }
    }
        
    // Manage the indices cache as a FIFO structure
    m_Cache.push_front(i);
    m_Cache.pop_back();
}


inline void cache_simulator::merge(const cache_simulator & Backward, const size_t PossibleOverlap)
{
    const size_t Overlap = (std::min)(PossibleOverlap, size());

    for (size_t i = 0; i < Overlap; ++i)
        push(Backward.m_Cache[i], true);

    m_NbHits += Backward.m_NbHits;
}


inline void cache_simulator::reset_hitcount()
{
    m_NbHits = 0;
}


inline size_t cache_simulator::hitcount() const
{
    return m_NbHits;
}




    } // namespace detail

} // namespace triangle_stripper




#endif // TRI_STRIPPER_HEADER_GUARD_CACHE_SIMULATOR_H
