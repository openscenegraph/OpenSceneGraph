//
// Copyright (C) 2004 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in tri_stripper.h
//
//////////////////////////////////////////////////////////////////////
// SVN: $Id: graph_array.h 93 2009-11-24 20:01:19Z gpsnoopy $
//////////////////////////////////////////////////////////////////////

#ifndef TRI_STRIPPER_HEADER_GUARD_GRAPH_ARRAY_H
#define TRI_STRIPPER_HEADER_GUARD_GRAPH_ARRAY_H

#include <cassert>
#include <algorithm>
#include <functional>
#include <limits>
#include <vector>




namespace triangle_stripper {

    namespace detail {




// graph_array main class
template <class nodetype>
class graph_array 
{
public:

    class arc;
    class node;

    // New types
    typedef size_t                                            nodeid;
    typedef nodetype                                        value_type;
    typedef std::vector<node>                                node_vector;
    typedef typename node_vector::iterator                    node_iterator;
    typedef typename node_vector::const_iterator            const_node_iterator;
    typedef typename node_vector::reverse_iterator            node_reverse_iterator;
    typedef typename node_vector::const_reverse_iterator    const_node_reverse_iterator;

    typedef graph_array<nodetype> graph_type;
    

    // graph_array::arc class
    class arc
    {
    public:
        node_iterator terminal() const;

    protected:
        friend class graph_array<nodetype>;

        arc(node_iterator Terminal);
    
        node_iterator    m_Terminal;
    };


    // New types
    typedef std::vector<arc>                    arc_list;
    typedef typename arc_list::iterator            out_arc_iterator;
    typedef typename arc_list::const_iterator    const_out_arc_iterator;


    // graph_array::node class
    class node
    {
    public:
        void mark();
        void unmark();
        bool marked() const;

        bool out_empty() const;
        size_t out_size() const;

        out_arc_iterator out_begin();
        out_arc_iterator out_end();
        const_out_arc_iterator out_begin() const;
        const_out_arc_iterator out_end() const;

        value_type & operator * ();
        value_type * operator -> ();
        const value_type & operator * () const;
        const value_type * operator -> () const;

        value_type & operator = (const value_type & Elem);

    protected:
        friend class graph_array<nodetype>;
        friend class std::vector<node>;

        node(arc_list & Arcs);

        arc_list &        m_Arcs;
        size_t            m_Begin;
        size_t            m_End;

        value_type        m_Elem;
        bool            m_Marker;
        private:
                node& operator = (const node&) { return *this; }
    };


    graph_array();
    explicit graph_array(size_t NbNodes);

    // Node related member functions
    bool empty() const;
    size_t size() const;

    node & operator [] (nodeid i);
    const node & operator [] (nodeid i) const;

    node_iterator begin();
    node_iterator end();
    const_node_iterator begin() const;
    const_node_iterator end() const;

    node_reverse_iterator rbegin();
    node_reverse_iterator rend();
    const_node_reverse_iterator rbegin() const;
    const_node_reverse_iterator rend() const;

    // Arc related member functions
    out_arc_iterator insert_arc(nodeid Initial, nodeid Terminal);
    out_arc_iterator insert_arc(node_iterator Initial, node_iterator Terminal);

    // Optimized (overloaded) functions
    void swap(graph_type & Right);
    friend void swap(graph_type & Left, graph_type & Right)                                        { Left.swap(Right); }

protected:
    graph_array(const graph_type &);
    graph_type & operator = (const graph_type &);

    node_vector        m_Nodes;
    arc_list        m_Arcs;
};



// Additional "low level", graph related, functions
template <class nodetype>
void unmark_nodes(graph_array<nodetype> & G);





//////////////////////////////////////////////////////////////////////////
// graph_array::arc inline functions
//////////////////////////////////////////////////////////////////////////

template <class N>
inline graph_array<N>::arc::arc(node_iterator Terminal)
    : m_Terminal(Terminal) { }


template <class N>
inline typename graph_array<N>::node_iterator graph_array<N>::arc::terminal() const
{
    return m_Terminal;
}



//////////////////////////////////////////////////////////////////////////
// graph_array::node inline functions
//////////////////////////////////////////////////////////////////////////

template <class N>
inline graph_array<N>::node::node(arc_list & Arcs)
    : m_Arcs(Arcs),
      m_Begin((std::numeric_limits<size_t>::max)()),
      m_End((std::numeric_limits<size_t>::max)()),
      m_Marker(false)
{

}


template <class N>
inline void graph_array<N>::node::mark()
{
    m_Marker = true;
}


template <class N>
inline void graph_array<N>::node::unmark()
{
    m_Marker = false;
}


template <class N>
inline bool graph_array<N>::node::marked() const
{
    return m_Marker;
}


template <class N>
inline bool graph_array<N>::node::out_empty() const
{
    return (m_Begin == m_End);
}


template <class N>
inline size_t graph_array<N>::node::out_size() const
{
    return (m_End - m_Begin);
}


template <class N>
inline typename graph_array<N>::out_arc_iterator graph_array<N>::node::out_begin()
{
    return (m_Arcs.begin() + m_Begin);
}


template <class N>
inline typename graph_array<N>::out_arc_iterator graph_array<N>::node::out_end()
{
    return (m_Arcs.begin() + m_End);
}


template <class N>
inline typename graph_array<N>::const_out_arc_iterator graph_array<N>::node::out_begin() const
{
    return (m_Arcs.begin() + m_Begin);
}


template <class N>
inline typename graph_array<N>::const_out_arc_iterator graph_array<N>::node::out_end() const
{
    return (m_Arcs.begin() + m_End);
}


template <class N>
inline N & graph_array<N>::node::operator * ()
{
    return m_Elem;
}


template <class N>
inline N * graph_array<N>::node::operator -> ()
{
    return &m_Elem;
}


template <class N>
inline const N & graph_array<N>::node::operator * () const
{
    return m_Elem;
}


template <class N>
inline const N * graph_array<N>::node::operator -> () const
{
    return &m_Elem;
}


template <class N>
inline N & graph_array<N>::node::operator = (const N & Elem)
{
    return (m_Elem = Elem);
}



//////////////////////////////////////////////////////////////////////////
// graph_array inline functions
//////////////////////////////////////////////////////////////////////////

template <class N>
inline graph_array<N>::graph_array() { }


template <class N>
inline graph_array<N>::graph_array(const size_t NbNodes)
    : m_Nodes(NbNodes, node(m_Arcs))
{
    // optimisation: we consider that, averagely, a triangle may have at least 2 neighbours
    // otherwise we are just wasting a bit of memory, but not that much
    m_Arcs.reserve(NbNodes * 2);
}


template <class N>
inline bool graph_array<N>::empty() const
{
    return m_Nodes.empty();
}


template <class N>
inline size_t graph_array<N>::size() const 
{
    return m_Nodes.size();
}


template <class N>
inline typename graph_array<N>::node & graph_array<N>::operator [] (const nodeid i)
{
    assert(i < size());

    return m_Nodes[i];
}


template <class N>
inline const typename graph_array<N>::node & graph_array<N>::operator [] (const nodeid i) const
{
    assert(i < size());

    return m_Nodes[i];
}


template <class N>
inline typename graph_array<N>::node_iterator graph_array<N>::begin()
{
    return m_Nodes.begin();
}


template <class N>
inline typename graph_array<N>::node_iterator graph_array<N>::end()
{
    return m_Nodes.end();
}


template <class N>
inline typename graph_array<N>::const_node_iterator graph_array<N>::begin() const
{
    return m_Nodes.begin();
}


template <class N>
inline typename graph_array<N>::const_node_iterator graph_array<N>::end() const
{
    return m_Nodes.end();
}


template <class N>
inline typename graph_array<N>::node_reverse_iterator graph_array<N>::rbegin()
{
    return m_Nodes.rbegin();
}


template <class N>
inline typename graph_array<N>::node_reverse_iterator graph_array<N>::rend()
{
    return m_Nodes.rend();
}


template <class N>
inline typename graph_array<N>::const_node_reverse_iterator graph_array<N>::rbegin() const
{
    return m_Nodes.rbegin();
}


template <class N>
inline typename graph_array<N>::const_node_reverse_iterator graph_array<N>::rend() const
{
    return m_Nodes.rend();
}


template <class N>
inline typename graph_array<N>::out_arc_iterator graph_array<N>::insert_arc(const nodeid Initial, const nodeid Terminal)
{
    assert(Initial < size());
    assert(Terminal < size());

    return insert_arc(m_Nodes.begin() + Initial, m_Nodes.begin() + Terminal);
}


template <class N>
inline typename graph_array<N>::out_arc_iterator graph_array<N>::insert_arc(const node_iterator Initial, const node_iterator Terminal)
{
    assert((Initial >= begin()) && (Initial < end()));
    assert((Terminal >= begin()) && (Terminal < end()));

    node & Node = * Initial;

    if (Node.out_empty()) {

        Node.m_Begin = m_Arcs.size();
        Node.m_End = m_Arcs.size() + 1;

    } else {

        // we optimise here for make_connectivity_graph()
        // we know all the arcs for a given node are successively and sequentially added
        assert(Node.m_End == m_Arcs.size());
        
        ++(Node.m_End);
    }

    m_Arcs.push_back(arc(Terminal));

    out_arc_iterator it = m_Arcs.end();
    return (--it);
}


template <class N>
inline void graph_array<N>::swap(graph_type & Right)
{
    std::swap(m_Nodes, Right.m_Nodes);
    std::swap(m_Arcs, Right.m_Arcs);
}



//////////////////////////////////////////////////////////////////////////
// additional functions
//////////////////////////////////////////////////////////////////////////

template <class N>
inline void unmark_nodes(graph_array<N> & G)
{
    std::for_each(G.begin(), G.end(), std::mem_fun_ref(&graph_array<N>::node::unmark));
}




    } // namespace detail

} // namespace triangle_stripper




#endif // TRI_STRIPPER_HEADER_GUARD_GRAPH_ARRAY_H
