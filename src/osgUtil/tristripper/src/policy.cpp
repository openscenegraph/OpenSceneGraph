//
// Copyright (C) 2004 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in tri_stripper.h
//
//////////////////////////////////////////////////////////////////////
// SVN: $Id: policy.cpp 86 2005-06-08 17:47:27Z gpsnoopy $
//////////////////////////////////////////////////////////////////////

#include "detail/policy.h"




namespace triangle_stripper {

    namespace detail {




void policy::Challenge(strip Strip, size_t Degree, size_t CacheHits)
{
    if (Strip.Size() < m_MinStripSize)
        return;

    // Cache is disabled, take the longest strip
    if (! m_Cache) {

        if (Strip.Size() > m_Strip.Size())
            m_Strip = Strip;

    // Cache simulator enabled
    } else {

        // Priority 1: Keep the strip with the best cache hit count
        if (CacheHits > m_CacheHits) {
            m_Strip = Strip;
            m_Degree = Degree;
            m_CacheHits = CacheHits;

        } else if (CacheHits == m_CacheHits) {

            // Priority 2: Keep the strip with the loneliest start triangle
            if ((m_Strip.Size() != 0) && (Degree < m_Degree)) {
                m_Strip = Strip;
                m_Degree = Degree;

            // Priority 3: Keep the longest strip 
            } else if (Strip.Size() > m_Strip.Size()) {
                m_Strip = Strip;
                m_Degree = Degree;
            }
        }
    }
}




    } // namespace detail

} // namespace triangle_stripper
