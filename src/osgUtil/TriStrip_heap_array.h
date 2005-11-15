// heap_array.h: interface for the heap_array class.
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
//                        Semi-dynamic indexed heap
//                        *************************
//
// Current version: 1.00 BETA 1 (24/10/2002)
//
// Comment: heap_array acts like a normal heap, you can push elements
//          and then get the greatest one.
//          However you cannot push any more element once an element
//          has been removed (pop, erase, etc...).
//          Elements can be modified after they've been pushed into
//          the heap via their indice.
//          
// History: -
//
//////////////////////////////////////////////////////////////////////

#ifndef TRISTRIP_HEAP_ARRAY_H
#define TRISTRIP_HEAP_ARRAY_H


// namespace common_structures
namespace common_structures {




template <class T, class CmpT = std::less<T> > 
class heap_array
{
public:

    struct heap_is_locked {    };


    // heap_array main interface. Pre = PreCondition, Post = PostCondition 

    heap_array() : m_Locked(false) { }        // Post: ((size() == 0) && ! locked())

    void clear();                            // Post: ((size() == 0) && ! locked())

    void reserve(size_t Size);
    size_t size() const;

    bool empty() const;
    bool locked() const;
    bool removed(size_t i) const;            // Pre: (valid(i))
    bool valid(size_t i) const;

    const T & top() const;                    // Pre: (! empty())
    const T & peek(size_t i) const;            // Pre: (valid(i) && ! removed(i))
    const T & operator [] (size_t i) const;    // Pre: (valid(i) && ! removed(i))

    size_t push(const T & Elem);            // Pre: (! locked()) else throw (heap_is_locked)

    void pop();                                // Pre: (! empty())                  Post: (locked())
    void erase(size_t i);                    // Pre: (valid(i) && ! removed(i))   Post: (locked())
    void update(size_t i, const T & Elem);    // Pre: (valid(i) && ! removed(i))   Post: (locked())

protected:

    struct linker {
        linker(const T & Elem, size_t i) : m_Elem(Elem), m_Indice(i) { }

        T        m_Elem;
        size_t    m_Indice;
    };

    typedef std::vector<linker> linked_heap;
    typedef std::vector<size_t> finder;

    void Adjust(size_t i);
    void Swap(size_t a, size_t b);
    bool Less(const linker & a, const linker & b) const;

    linked_heap    m_Heap;
    finder        m_Finder;
    CmpT        m_Compare;
    bool        m_Locked;
};




//////////////////////////////////////////////////////////////////////////
// heap_indexed Inline functions
//////////////////////////////////////////////////////////////////////////

template <class T, class CmpT> 
inline void heap_array<T, CmpT>::clear() {
    m_Heap.clear();
    m_Finder.clear();
    m_Locked = false;
}


template <class T, class CmpT> 
inline bool heap_array<T, CmpT>::empty() const {
    return m_Heap.empty();
}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::locked() const {
    return m_Locked;
}


template <class T, class CmpT>
inline void heap_array<T, CmpT>::reserve(size_t Size) {
    m_Heap.reserve(Size);
    m_Finder.reserve(Size);
}


template <class T, class CmpT> 
inline size_t heap_array<T, CmpT>::size() const {
    return m_Heap.size();
}


template <class T, class CmpT> 
inline const T & heap_array<T, CmpT>::top() const {
    // Debug check to ensure heap is not empty
    //assert(! empty());
    if (empty()) throw "heap_array<T, CmpT>::top() error, heap empty";

    return m_Heap.front().m_Elem;
}


template <class T, class CmpT> 
inline const T & heap_array<T, CmpT>::peek(size_t i) const {
    // Debug check to ensure element is still present
    //assert(! removed(i));
    if (removed(i)) throw "heap_array<T, CmpT>::peek(size_t i) error";

    return (m_Heap[m_Finder[i]].m_Elem);
}


template <class T, class CmpT> 
inline const T & heap_array<T, CmpT>::operator [] (size_t i) const {
    return peek(i);
}


template <class T, class CmpT> 
inline void heap_array<T, CmpT>::pop() {
    m_Locked = true;

    // Debug check to ensure heap is not empty
    //assert(! empty());
    if (empty()) throw "heap_array<T, CmpT>::pop() error, heap empty";
    
    Swap(0, size() - 1);
    m_Heap.pop_back();
    Adjust(0);
}


template <class T, class CmpT> 
inline size_t heap_array<T, CmpT>::push(const T & Elem) {
    if (m_Locked)
        throw "heap_is_locked";

    size_t Id = size();
    m_Finder.push_back(Id);
    m_Heap.push_back(linker(Elem, Id));
    Adjust(Id);

    return Id;
}


template <class T, class CmpT>
inline void heap_array<T, CmpT>::erase(size_t i) {
    m_Locked = true;

    // Debug check to ensure element is still present
    if (removed(i)) throw "heap_array<T, CmpT>::erase(size_t i) error";

    size_t j = m_Finder[i];

    if (j==m_Heap.size()-1)
    {
        m_Heap.pop_back();
    }
    else
    {
        Swap(j, size() - 1);
        m_Heap.pop_back();
        Adjust(j);
    }


}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::removed(size_t i) const {
    return (m_Finder[i] >= m_Heap.size());
}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::valid(size_t i) const {
    return (i < m_Finder.size());
}


template <class T, class CmpT> 
inline void heap_array<T, CmpT>::update(size_t i, const T & Elem) {
    // Debug check to ensure element is still present
    // assert(! removed(i));
    if (removed(i)) throw "heap_array<T, CmpT>::update(size_t i, const T & Elem) error";

    size_t j = m_Finder[i];
    m_Heap[j].m_Elem = Elem;
    Adjust(j);
}


template <class T, class CmpT> 
inline void heap_array<T, CmpT>::Adjust(size_t i)
{
    if (m_Heap.size()<=1) return; // nothing to swap, so just return.

    size_t j;

    // Check the upper part of the heap
    for (j = i; (j > 0) && (Less(m_Heap[(j - 1) / 2], m_Heap[j])); j = ((j - 1) / 2))
        Swap(j, (j - 1) / 2);

    // Check the lower part of the heap
    for (i = j; (j = 2 * i + 1) < size(); i = j) {
         if ((j + 1 < size()) && (Less(m_Heap[j], m_Heap[j + 1])))
            ++j;

        if (Less(m_Heap[j], m_Heap[i]))
            return;

        Swap(i, j);
    }
}


template <class T, class CmpT> 
inline void heap_array<T, CmpT>::Swap(size_t a, size_t b) {
    std::swap(m_Heap[a], m_Heap[b]);

    // use (size_t &) to get rid of a bogus compile warning
    (size_t &) (m_Finder[(m_Heap[a].m_Indice)]) = a;
    (size_t &) (m_Finder[(m_Heap[b].m_Indice)]) = b;
}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::Less(const linker & a, const linker & b) const {
    return m_Compare(a.m_Elem, b.m_Elem);
}




} // namespace common_structures

#endif
