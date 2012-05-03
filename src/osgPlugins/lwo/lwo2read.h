
/****************************************************************************

    Functions for reading basic data types from LWO2 files

    Copyright (C) 2002 by Marco Jez

****************************************************************************/

#ifndef LWO2READ_
#define LWO2READ_

#include "lwo2types.h"

namespace lwo2
{

template<class Iter>
I1 read_I1(Iter &it)
{
    return static_cast<I1>(*(it++));
}

template<class Iter>
I2 read_I2(Iter &it)
{
    I2 i2(((static_cast<I2>(*(it)) & 0xFF) << 8) |
          (static_cast<I2>(*(it+1)) & 0xFF));
    it += 2;
    return i2;
}

template<class Iter>
I4 read_I4(Iter &it)
{
    I4 i4(((static_cast<I4>(*(it)) & 0xFF) << 24) |
          ((static_cast<I4>(*(it+1)) & 0xFF) << 16) |
          ((static_cast<I4>(*(it+2)) & 0xFF) << 8) |
          (static_cast<I4>(*(it+3)) & 0xFF));
    it += 4;
    return i4;
}

template<class Iter>
U1 read_U1(Iter &it)
{
    return static_cast<U1>(*(it++));
}

template<class Iter>
U2 read_U2(Iter &it)
{
    U2 u2(((static_cast<U2>(*(it)) & 0xFF) << 8) |
          (static_cast<U2>(*(it+1)) & 0xFF));
    it += 2;
    return u2;
}

template<class Iter>
U4 read_U4(Iter &it)
{
    U4 u4(((static_cast<U4>(*(it)) & 0xFF) << 24) |
          ((static_cast<U4>(*(it+1)) & 0xFF) << 16) |
          ((static_cast<U4>(*(it+2)) & 0xFF) << 8) |
          (static_cast<U4>(*(it+3)) & 0xFF));
    it += 4;
    return u4;
}

template<typename D, typename S>
D changeType4(S src)
{
    D dest;
    char* dest_ptr = reinterpret_cast<char*>(&dest);
    char* src_ptr = reinterpret_cast<char*>(&src);
    for(int i=0; i<4; ++i)
    {
        dest_ptr[i] = src_ptr[i];
    }
    return dest;
}

template<class Iter>
F4 read_F4(Iter &it)
{
    return changeType4<F4, U4>(read_U4(it));
}

template<class Iter>
ID4 read_ID4(Iter &it)
{
    ID4 value;
    for (int i=0; i<4; ++i) value.id[i] = *(it++);
    return value;
}

template<class Iter>
S0 read_S0(Iter &it)
{
    S0 value;
    while (*it) {
        value += *(it++);
    }
    ++it;
    if (value.length() % 2 == 0) ++it;
    return value;
}

template<class Iter>
VX read_VX(Iter &it)
{
    VX vx;
    if ((*it & 0xFF) == 0xFF) {
        vx.index = read_U4(it) & 0x00FFFFFF;
    } else {
        vx.index = static_cast<U4>(read_U2(it));
    }
    return vx;
}

template<class Iter>
COL12 read_COL12(Iter &it)
{
    COL12 value;
    value.red = read_F4(it);
    value.green = read_F4(it);
    value.blue = read_F4(it);
    return value;
}

template<class Iter>
VEC12 read_VEC12(Iter &it)
{
    VEC12 value;
    value.X = read_F4(it);
    value.Y = read_F4(it);
    value.Z = read_F4(it);
    return value;
}

template<class Iter>
FP4 read_FP4(Iter &it)
{
    FP4 value;
    value.fraction = read_F4(it);
    return value;
}

template<class Iter>
ANG4 read_ANG4(Iter &it)
{
    ANG4 value;
    value.radians = read_F4(it);
    return value;
}

template<class Iter>
FNAM0 read_FNAM0(Iter &it)
{
    FNAM0 value;
    value.name = read_S0(it);
    return value;
}

}

#endif
