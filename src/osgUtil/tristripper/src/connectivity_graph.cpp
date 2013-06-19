//
// Copyright (C) 2004 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in tri_stripper.h
//
//////////////////////////////////////////////////////////////////////
// SVN: $Id: connectivity_graph.cpp 86 2005-06-08 17:47:27Z gpsnoopy $
//////////////////////////////////////////////////////////////////////

#include "detail/connectivity_graph.h"

#include <algorithm>




namespace triangle_stripper {

    namespace detail {




namespace
{

    class tri_edge : public triangle_edge
    {
    public:
        tri_edge(index A, index B, size_t TriPos)
            : triangle_edge(A, B), m_TriPos(TriPos) { }

        size_t TriPos() const { return m_TriPos; }

    private:
        size_t    m_TriPos;
    };


    class cmp_tri_edge_lt
    {
    public:
        bool operator() (const tri_edge & a, const tri_edge & b) const;
    };


    typedef std::vector<tri_edge> edge_map;


    void LinkNeighbours(graph_array<triangle> & Triangles, const edge_map & EdgeMap, const tri_edge Edge);

}




void make_connectivity_graph(graph_array<triangle> & Triangles, const indices & Indices)
{
    assert(Triangles.size() == (Indices.size() / 3));

    // Fill the triangle data
    for (size_t i = 0; i < Triangles.size(); ++i)
        Triangles[i] = triangle(Indices[i * 3 + 0], Indices[i * 3 + 1], Indices[i * 3 + 2]);

    // Build an edge lookup table
    edge_map EdgeMap;
    EdgeMap.reserve(Triangles.size() * 3);

    for (size_t i = 0; i < Triangles.size(); ++i) {

        const triangle & Tri = * Triangles[i];

        EdgeMap.push_back(tri_edge(Tri.A(), Tri.B(), i)); 
        EdgeMap.push_back(tri_edge(Tri.B(), Tri.C(), i)); 
        EdgeMap.push_back(tri_edge(Tri.C(), Tri.A(), i)); 
    }

    std::sort(EdgeMap.begin(), EdgeMap.end(), cmp_tri_edge_lt());

    // Link neighbour triangles together using the lookup table
    for (size_t i = 0; i < Triangles.size(); ++i) {

        const triangle & Tri = * Triangles[i];

        LinkNeighbours(Triangles, EdgeMap, tri_edge(Tri.B(), Tri.A(), i)); 
        LinkNeighbours(Triangles, EdgeMap, tri_edge(Tri.C(), Tri.B(), i)); 
        LinkNeighbours(Triangles, EdgeMap, tri_edge(Tri.A(), Tri.C(), i)); 
    }
}



namespace
{
    
    inline bool cmp_tri_edge_lt::operator() (const tri_edge & a, const tri_edge & b) const
    {
        const index A1 = a.A();
        const index B1 = a.B();
        const index A2 = b.A();
        const index B2 = b.B();

        if ((A1 < A2) || ((A1 == A2) && (B1 < B2)))
            return true;
        else
            return false;
    }


    void LinkNeighbours(graph_array<triangle> & Triangles, const edge_map & EdgeMap, const tri_edge Edge)
    {
        // Find the first edge equal to Edge
        edge_map::const_iterator it = std::lower_bound(EdgeMap.begin(), EdgeMap.end(), Edge, cmp_tri_edge_lt());

        // See if there are any other edges that are equal
        // (if so, it means that more than 2 triangles are sharing the same edge,
        //  which is unlikely but not impossible)
        for (; (it != EdgeMap.end()) && (Edge == (* it)); ++it)
            Triangles.insert_arc(Edge.TriPos(), it->TriPos());

        // Note: degenerated triangles will also point themselves as neighbour triangles
    }

}




    } // namespace detail

} // namespace detail

