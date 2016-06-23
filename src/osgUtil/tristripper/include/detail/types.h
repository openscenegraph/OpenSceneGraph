//
// Copyright (C) 2004 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in tri_stripper.h
//
//////////////////////////////////////////////////////////////////////
// SVN: $Id: types.h 86 2005-06-08 17:47:27Z gpsnoopy $
//////////////////////////////////////////////////////////////////////

#ifndef TRI_STRIPPER_HEADER_GUARD_TYPES_H
#define TRI_STRIPPER_HEADER_GUARD_TYPES_H




namespace triangle_stripper {

    namespace detail {




class triangle
{
public:
    triangle()
        : m_A(0), m_B(0), m_C(0), m_StripID(0) { }

    triangle(index A, index B, index C)
        : m_A(A), m_B(B), m_C(C), m_StripID(0) { }

    void ResetStripID()                            { m_StripID = 0; }
    void SetStripID(size_t StripID)                { m_StripID = StripID; }
    size_t StripID() const                        { return m_StripID; }

    index A() const                                { return m_A; }
    index B() const                                { return m_B; }
    index C() const                                { return m_C; }

private:
    index    m_A;
    index    m_B;
    index    m_C;

    size_t    m_StripID;
};



class triangle_edge
{
public:
    triangle_edge(index A, index B)
        : m_A(A), m_B(B) { }

    index A() const                                { return m_A; }
    index B() const                                { return m_B; }

    bool operator == (const triangle_edge & Right) const {
        return ((A() == Right.A()) && (B() == Right.B()));
    }

private:
    index    m_A;
    index    m_B;
};



enum triangle_order { ABC, BCA, CAB };



class strip
{
public:
    strip()
        : m_Start(0), m_Order(ABC), m_Size(0) { }

    strip(size_t Start, triangle_order Order, size_t Size)
        : m_Start(Start), m_Order(Order), m_Size(Size) { }

    size_t Start() const                        { return m_Start; }
    triangle_order Order() const                { return m_Order; }
    size_t Size() const                            { return m_Size; }

private:
    size_t            m_Start;
    triangle_order    m_Order;
    size_t            m_Size;
};




    } // namespace detail

} // namespace triangle_stripper




#endif // TRI_STRIPPER_HEADER_GUARD_TYPES_H
