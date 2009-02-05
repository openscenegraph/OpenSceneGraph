// graph_array.h: interface for the graph_array class.
//
//////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2002 Tanguy Fautré.
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
//  softdev@pandora.be
//
//////////////////////////////////////////////////////////////////////
//
//                        Semi-dynamic directed graph
//                        ***************************
//
// Current version: 3.00 BETA 3 (04/12/2002)
//
// Comment: graph_array is equivalent to an array of nodes linked by
//          arcs.
//          This means you can't change the size (the number of nodes)
//          of the graph once you created it (setsize() will delete
//          any previous nodes and arcs).
//          But you can add or remove arcs.
//          
// History: - 3.00 BETA 3 (04/12/2002) - Added empty()
//                                     - Changed some parameters from copy to reference
//                                     - Fixed a bug with erase_arc
//                                     - Un-inlined external functions
//                                     - Added "insert_arc" which is equivalent to "insert"
//          - 3.00 BETA 2 (16/11/2002) - Improved portability
//          - 3.00 BETA 1 (27/08/2002) - First public release
//
//////////////////////////////////////////////////////////////////////

#ifndef TRISTRIP_GRAPH_ARRAY_H
#define TRISTRIP_GRAPH_ARRAY_H

// namespace common_structures
namespace common_structures {




// graph_array main class
template <class nodetype, class arctype>
class graph_array 
{
public:

    class arc;
    class node;

    // New types
    typedef size_t                                        nodeid;
    typedef typename std::vector<node>::iterator                    node_iterator;
    typedef typename std::vector<node>::const_iterator            const_node_iterator;
    typedef typename std::vector<node>::reverse_iterator            node_reverse_iterator;
    typedef typename std::vector<node>::const_reverse_iterator    const_node_reverse_iterator;

    typedef graph_array<nodetype, arctype> _mytype;
    

    // graph_array::arc class
    class arc
    {
    public:
        arc() {}
        arc & mark()                                    { m_Marker = true; return (* this); }
        arc & unmark()                                    { m_Marker = false; return (* this); }
        bool marked() const                                { return m_Marker; }

        node_iterator initial() const                    { return m_Initial; }
        node_iterator terminal() const                    { return m_Terminal; }

        arctype & operator * ()                            { return m_Elem; }
        const arctype & operator * () const                { return m_Elem; }

    protected:
        friend class graph_array<nodetype, arctype>;

        arc(const node_iterator & Initial, const node_iterator & Terminal)
            : m_Initial(Initial), m_Terminal(Terminal), m_Marker(false) { }

        arc(const node_iterator & Initial, const node_iterator & Terminal, const arctype & Elem)
            : m_Initial(Initial), m_Terminal(Terminal), m_Elem(Elem), m_Marker(false) { }
    
        node_iterator    m_Initial;
        node_iterator    m_Terminal;
        arctype            m_Elem;
        bool            m_Marker;
    };


    // New types
    typedef typename std::list<arc>::iterator            out_arc_iterator;
    typedef typename std::list<arc>::const_iterator        const_out_arc_iterator;


    // graph_array::node class
    class node
    {
    public:
        node & mark()                                    { m_Marker = true; return (* this); }
        node & unmark()                                    { m_Marker = false; return (* this); }
        bool marked() const                                { return m_Marker; }

        bool out_empty() const                            { return m_OutArcs.empty(); }
        size_t number_of_out_arcs() const                { return m_OutArcs.size(); }

        out_arc_iterator out_begin()                    { return m_OutArcs.begin(); }
        out_arc_iterator out_end()                        { return m_OutArcs.end(); }
        const_out_arc_iterator out_begin() const        { return m_OutArcs.begin(); }
        const_out_arc_iterator out_end() const            { return m_OutArcs.end(); }

        nodetype & operator * ()                        { return m_Elem; }
        nodetype * operator -> ()                        { return &m_Elem; }
        const nodetype & operator * () const            { return m_Elem; }
        const nodetype * operator -> () const            { return &m_Elem; }

        nodetype & operator = (const nodetype & Elem)    { return (m_Elem = Elem); }

        node() : m_Marker(false) { }
    protected:
        friend class graph_array<nodetype, arctype>;
        friend class std::vector<node>;


        std::list<arc>    m_OutArcs;
        nodetype        m_Elem;
        bool            m_Marker;
    };


    // Construction/Destruction
    graph_array();
    explicit graph_array(const size_t NbNodes);

    // Node related member functions
    void clear();
    bool empty() const;
    void setsize(const size_t NbNodes);
    size_t size() const;

    node & operator [] (const nodeid & i);
    const node & operator [] (const nodeid & i) const;

    node_iterator begin();
    node_iterator end();
    const_node_iterator begin() const;
    const_node_iterator end() const;

    node_reverse_iterator rbegin();
    node_reverse_iterator rend();
    const_node_reverse_iterator rbegin() const;
    const_node_reverse_iterator rend() const;

    // Arc related member functions
    size_t number_of_arcs() const;

    void erase_arcs();
    void erase_arcs(const node_iterator & Initial);
    out_arc_iterator erase_arc(const out_arc_iterator & Pos);

    out_arc_iterator insert_arc(const nodeid & Initial, const nodeid & Terminal);
    out_arc_iterator insert_arc(const nodeid & Initial, const nodeid & Terminal, const arctype & Elem);
    out_arc_iterator insert_arc(const node_iterator & Initial, const node_iterator & Terminal);
    out_arc_iterator insert_arc(const node_iterator & Initial, const node_iterator & Terminal, const arctype & Elem);

    // Another interface for insert_arc
    out_arc_iterator insert(const nodeid & Initial, const nodeid & Terminal)                                        { return insert_arc(Initial, Terminal); }
    out_arc_iterator insert(const nodeid & Initial, const nodeid & Terminal, const arctype & Elem)                    { return insert_arc(Initial, Terminal, Elem); }
    out_arc_iterator insert(const node_iterator & Initial, const node_iterator & Terminal)                            { return insert_arc(Initial, Terminal); }
    out_arc_iterator insert(const node_iterator & Initial, const node_iterator & Terminal, const arctype & Elem)    { return insert_arc(Initial, Terminal, Elem); }

    // Optimized (overloaded) functions
    void swap(_mytype & Right);
// removed since it was causing g++ 2.95.3 to produce many compile errors
// presumably due to implicit import of the std::swap implementation.
// Robert Osfield, Jan 2002.
//    friend void swap(_mytype & Left, _mytype & Right)    { Left.swap(Right); }

protected:

    graph_array& operator = (const graph_array&) { return *this; }

    size_t                m_NbArcs;
    std::vector<node>    m_Nodes;
};



// Additional "low level", graph related, functions
template <class nodetype, class arctype>
void unmark_nodes(graph_array<nodetype, arctype> & G);

template <class nodetype, class arctype>
void unmark_arcs_from_node(typename graph_array<nodetype, arctype>::node & N);

template <class nodetype, class arctype>
void unmark_arcs(graph_array<nodetype, arctype> & G);




//////////////////////////////////////////////////////////////////////////
// graph_array Inline functions
//////////////////////////////////////////////////////////////////////////

template <class nodetype, class arctype>
inline graph_array<nodetype, arctype>::graph_array() : m_NbArcs(0) { }


template <class nodetype, class arctype>
inline graph_array<nodetype, arctype>::graph_array(const size_t NbNodes) : m_NbArcs(0), m_Nodes(NbNodes) { }


template <class nodetype, class arctype>
inline void graph_array<nodetype, arctype>::clear() {
    m_NbArcs = 0;
    m_Nodes.clear();
}



template <class nodetype, class arctype>
inline bool graph_array<nodetype, arctype>::empty() const {
    return m_Nodes.empty();
}


template <class nodetype, class arctype>
inline size_t graph_array<nodetype, arctype>::size() const {
    return m_Nodes.size();
}


template <class nodetype, class arctype>
inline void graph_array<nodetype, arctype>::setsize(const size_t NbNodes) {
    clear();
    m_Nodes.resize(NbNodes);
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::node & graph_array<nodetype, arctype>::operator [] (const nodeid & i) {
    // Debug check
    // assert(i < size());
    if (i >= size()) throw "graph_array<nodetype, arctype>::operator [] out of range";

    return m_Nodes[i];
}


template <class nodetype, class arctype>
inline const typename graph_array<nodetype, arctype>::node & graph_array<nodetype, arctype>::operator [] (const nodeid & i) const {
    // Debug check
    // assert(i < size());
    if (i >= size()) throw "graph_array<nodetype, arctype>::operator [] out of range";

    return m_Nodes[i];
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::node_iterator graph_array<nodetype, arctype>::begin() {
    return m_Nodes.begin();
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::node_iterator graph_array<nodetype, arctype>::end() {
    return m_Nodes.end();
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::const_node_iterator graph_array<nodetype, arctype>::begin() const {
    return m_Nodes.begin();
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::const_node_iterator graph_array<nodetype, arctype>::end() const {
    return m_Nodes.end();
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::node_reverse_iterator graph_array<nodetype, arctype>::rbegin() {
    return m_Nodes.rbegin();
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::node_reverse_iterator graph_array<nodetype, arctype>::rend() {
    return m_Nodes.rend();
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::const_node_reverse_iterator graph_array<nodetype, arctype>::rbegin() const {
    return m_Nodes.rbegin();
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::const_node_reverse_iterator graph_array<nodetype, arctype>::rend() const {
    return m_Nodes.rend();
}


template <class nodetype, class arctype>
inline size_t graph_array<nodetype, arctype>::number_of_arcs() const {
    return m_NbArcs;
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::out_arc_iterator graph_array<nodetype, arctype>::insert_arc(const nodeid & Initial, const nodeid & Terminal) {
    return (insert(begin() + Initial, begin() + Terminal));
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::out_arc_iterator graph_array<nodetype, arctype>::insert_arc(const nodeid & Initial, const nodeid & Terminal, const arctype & Elem) {
    return (insert(begin() + Initial, begin() + Terminal, Elem));
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::out_arc_iterator graph_array<nodetype, arctype>::insert_arc(const node_iterator & Initial, const node_iterator & Terminal) {
    ++m_NbArcs;
    Initial->m_OutArcs.push_back(arc(Initial, Terminal));
    return (--(Initial->m_OutArcs.end()));
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::out_arc_iterator graph_array<nodetype, arctype>::insert_arc(const node_iterator & Initial, const node_iterator & Terminal, const arctype & Elem) {
    ++m_NbArcs;
    Initial->m_OutArcs.push_back(arc(Initial, Terminal, Elem));
    return (--(Initial->m_OutArcs.end()));
}


template <class nodetype, class arctype>
inline typename graph_array<nodetype, arctype>::out_arc_iterator graph_array<nodetype, arctype>::erase_arc(const out_arc_iterator & Pos) {
    --m_NbArcs;
    return (Pos->initial()->m_OutArcs.erase(Pos));
}


template <class nodetype, class arctype>
inline void graph_array<nodetype, arctype>::erase_arcs(const node_iterator & Initial) {
    m_NbArcs -= (Initial->m_OutArcs.size());
    Initial->m_OutArcs.clear();
}


template <class nodetype, class arctype>
inline void graph_array<nodetype, arctype>::erase_arcs() {
    m_NbArcs = 0;
    for (nodeid i = 0; i < this->Size(); ++i)
        m_Nodes[i].m_OutArcs.clear();
}


template <class nodetype, class arctype>
inline void graph_array<nodetype, arctype>::swap(_mytype & Right) {
    std::swap(m_NbArcs, Right.m_NbArcs);
    std::swap(m_Nodes, Right.m_Nodes);
}



//////////////////////////////////////////////////////////////////////////
// additional functions
//////////////////////////////////////////////////////////////////////////

template <class nodetype, class arctype>
void unmark_nodes(graph_array<nodetype, arctype> & G)
{
    typedef typename graph_array<nodetype, arctype>::node_iterator node_it;

    for (node_it NodeIt = G.begin(); NodeIt != G.end(); ++NodeIt)
        NodeIt->unmark();
}


template <class nodetype, class arctype>
void unmark_arcs_from_node(typename graph_array<nodetype, arctype>::node & N)
{
    typedef typename graph_array<nodetype, arctype>::out_arc_iterator arc_it;

    for (arc_it ArcIt = N.out_begin(); ArcIt != N.out_end(); ++ArcIt)
        ArcIt->unmark();
}


template <class nodetype, class arctype>
void unmark_arcs(graph_array<nodetype, arctype> & G)
{
    typedef typename graph_array<nodetype, arctype>::node_iterator node_it;

    for (node_it NodeIt = G.begin(); NodeIt != G.end(); ++NodeIt)
        unmark_arcs_from_node(* NodeIt);
}




} // namespace common_structures

#endif
