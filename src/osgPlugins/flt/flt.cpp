// flt.cpp

#include "flt.h"

using namespace flt;

////////////////////////////////////////////////////////////////////

int flt::isLittleEndianMachine()
{
    int a = 1;
    return (int)(*(char*)&a);
}


                                 /*nDst*/
void flt::endian2(void* pSrc, int nSrc, void* pDst, int )
{
    if (nSrc == 2)
    {
        short tmp1;
        tmp1 = *(short *)pSrc;
        tmp1 = (tmp1 << 8) | ((tmp1 >> 8) & 0xff);
        *(short *)pDst = tmp1;
    }
    else if (nSrc == 4)
    {
        long tmp1;
        tmp1 = *(long *)pSrc;
        tmp1 = (tmp1 << 24) | ((tmp1 << 8) & 0xff0000) | ((tmp1 >> 8) & 0xff00) | ((tmp1 >> 24) & 0xff);
        *(long *)pDst = tmp1;
    }
    else if (nSrc == 8)
    {
        long tmp1, tmp2;
        tmp1 = *(long *)pSrc;
        tmp2 = *(1 + (long *)pSrc);
        tmp1 = (tmp1 << 24) | ((tmp1 << 8) & 0xff0000) | ((tmp1 >> 8) & 0xff00) | ((tmp1 >> 24) & 0xff);
        tmp2 = (tmp2 << 24) | ((tmp2 << 8) & 0xff0000) | ((tmp2 >> 8) & 0xff00) | ((tmp2 >> 24) & 0xff);
        *(long *)pDst = tmp2;
        *(1 + (long *)pDst) = tmp1;
    }
}

