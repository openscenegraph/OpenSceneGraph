//
// Copyright (C) 2004 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in tri_stripper.h
//
//////////////////////////////////////////////////////////////////////
// SVN: $Id: heap_array.h 86 2005-06-08 17:47:27Z gpsnoopy $
//////////////////////////////////////////////////////////////////////

#ifndef TRI_STRIPPER_HEADER_GUARD_HEAP_ARRAY_H
#define TRI_STRIPPER_HEADER_GUARD_HEAP_ARRAY_H

#include <vector>




namespace triangle_stripper {

    namespace detail {




// mutable heap
// can be interfaced pretty muck like an array
template <class T, class CmpT = std::less<T> > 
class heap_array
{
public:

    // Pre = PreCondition, Post = PostCondition 

    heap_array() : m_Locked(false) { }        // Post: ((size() == 0) && ! locked())

    void clear();                            // Post: ((size() == 0) && ! locked())

    void reserve(size_t Size);
    size_t size() const;

    bool empty() const;
    bool locked() const;
    bool removed(size_t i) const;            // Pre: (valid(i))
    bool valid(size_t i) const;

    size_t position(size_t i) const;        // Pre: (valid(i))

    const T & top() const;                    // Pre: (! empty())
    const T & peek(size_t i) const;            // Pre: (! removed(i))
    const T & operator [] (size_t i) const;    // Pre: (! removed(i))

    void lock();                            // Pre: (! locked())   Post: (locked())
    size_t push(const T & Elem);            // Pre: (! locked())

    void pop();                                // Pre: (locked() && ! empty())
    void erase(size_t i);                    // Pre: (locked() && ! removed(i))
    void update(size_t i, const T & Elem);    // Pre: (locked() && ! removed(i))

protected:

    heap_array(const heap_array &);
    heap_array & operator = (const heap_array &);

    class linker
    {
    public:
        linker(const T & Elem, size_t i)
            : m_Elem(Elem), m_Index(i) { }

        T        m_Elem;
        size_t    m_Index;
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
// heap_indexed inline functions
//////////////////////////////////////////////////////////////////////////

template <class T, class CmpT> 
inline void heap_array<T, CmpT>::clear()
{
    m_Heap.clear();
    m_Finder.clear();
    m_Locked = false;
}


template <class T, class CmpT> 
inline bool heap_array<T, CmpT>::empty() const
{
    return m_Heap.empty();
}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::locked() const
{
    return m_Locked;
}


template <class T, class CmpT>
inline void heap_array<T, CmpT>::reserve(const size_t Size)
{
    m_Heap.reserve(Size);
    m_Finder.reserve(Size);
}


template <class T, class CmpT> 
inline size_t heap_array<T, CmpT>::size() const
{
    return m_Heap.size();
}


template <class T, class CmpT> 
inline const T & heap_array<T, CmpT>::top() const
{
    assert(! empty());

    return m_Heap.front().m_Elem;
}


template <class T, class CmpT> 
inline const T & heap_array<T, CmpT>::peek(const size_t i) const
{
    assert(! removed(i));

    return (m_Heap[m_Finder[i]].m_Elem);
}


template <class T, class CmpT> 
inline const T & heap_array<T, CmpT>::operator [] (const size_t i) const
{
    return peek(i);
}


template <class T, class CmpT> 
inline void heap_array<T, CmpT>::pop()
{
    assert(locked());
    assert(! empty());

    Swap(0, size() - 1);
    m_Heap.pop_back();

    if (! empty())
        Adjust(0);
}


template <class T, class CmpT>
inline void heap_array<T, CmpT>::lock()
{
    assert(! locked());

    m_Locked =true;
}


template <class T, class CmpT> 
inline size_t heap_array<T, CmpT>::push(const T & Elem)
{
    assert(! locked());

    const size_t Id = size();
    m_Finder.push_back(Id);
    m_Heap.push_back(linker(Elem, Id));
    Adjust(Id);

    return Id;
}


template <class T, class CmpT>
inline void heap_array<T, CmpT>::erase(const size_t i)
{
    assert(locked());
    assert(! removed(i));

    const size_t j = m_Finder[i];
    Swap(j, size() - 1);
    m_Heap.pop_back();

    if (j != size())
        Adjust(j);
}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::removed(const size_t i) const
{
    assert(valid(i));

    return (m_Finder[i] >= m_Heap.size());
}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::valid(const size_t i) const
{
    return (i < m_Finder.size());
}


template <class T, class CmpT>
inline size_t heap_array<T, CmpT>::position(const size_t i) const
{
    assert(valid(i));

    return (m_Heap[i].m_Index);
}


template <class T, class CmpT> 
inline void heap_array<T, CmpT>::update(const size_t i, const T & Elem)
{
    assert(locked());
    assert(! removed(i));

    const size_t j = m_Finder[i];
    m_Heap[j].m_Elem = Elem;
    Adjust(j);
}


template <class T, class CmpT> 
inline void heap_array<T, CmpT>::Adjust(size_t i)
{
    assert(i < m_Heap.size());

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
inline void heap_array<T, CmpT>::Swap(const size_t a, const size_t b)
{
    std::swap(m_Heap[a], m_Heap[b]);

    m_Finder[(m_Heap[a].m_Index)] = a;
    m_Finder[(m_Heap[b].m_Index)] = b;
}


template <class T, class CmpT>
inline bool heap_array<T, CmpT>::Less(const linker & a, const linker & b) const
{
    return m_Compare(a.m_Elem, b.m_Elem);
}




    } // namespace detail

} // namespace triangle_stripper




#endif // TRI_STRIPPER_HEADER_GUARD_HEAP_ARRAY_H
