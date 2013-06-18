/*  Code for writing AC3D format files, constructs one object per Geode
 * since geodes only have 1 material, and AC3D allows multiple materials you
 * may not get an exact vopy of an ac3d file used as input.
 *
 * originally by Roger James.
 * upgraded to different types of Geometry primitive by Geoff Michel.
 * Old GeoSet parsing code is commented out - will be removed eventually.
 */
#include <assert.h>
#include <list>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/Drawable>
#include <osg/Geometry>
#include <limits>
#include <iomanip>

#include "Exception.h"
#include "Geode.h"

using namespace ac3d;
using namespace std;



void Geode::OutputVertex(int Index, const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices, ostream& fout)
{
    int LocalTexIndex;
    int LocalVertexIndex;
    if (NULL == pVertexIndices)
        LocalVertexIndex = Index;
    else
        LocalVertexIndex = pVertexIndices->index(Index);
    if (NULL != pTexCoords)
    {
        // Got some tex coords
        // Check for an index
        if (NULL != pTexIndices)
            // Access tex coord array indirectly
            LocalTexIndex = pTexIndices->index(Index);
        else
            LocalTexIndex  = Index;
        fout << LocalVertexIndex << " " << pTexCoords[LocalTexIndex][0] << " " << pTexCoords[LocalTexIndex][1] << std::endl;
    }
    else
        fout << LocalVertexIndex << " 0 0" << std::endl;
}

void Geode::OutputLines(const int iCurrentMaterial, const unsigned int surfaceFlags,
                           const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, ostream& fout)
{
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd; vindex+=2)
    {
        OutputSurfHead(iCurrentMaterial,surfaceFlags,2, fout);
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindex+1, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputLineStrip(const int iCurrentMaterial, const unsigned int surfaceFlags,
                           const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, ostream& fout)
{
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    OutputSurfHead(iCurrentMaterial,surfaceFlags,indexEnd-drawArray->getFirst(), fout);
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd; ++vindex)
    {
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputLineLoop(const int iCurrentMaterial, const unsigned int surfaceFlags,
                           const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, ostream& fout)
{
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    OutputSurfHead(iCurrentMaterial,surfaceFlags,indexEnd-drawArray->getFirst(), fout);
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd; ++vindex)
    {
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputTriangle(const int iCurrentMaterial, const unsigned int surfaceFlags,
                           const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, ostream& fout)
{
    unsigned int primCount = 0;
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd; ++vindex,++primCount)
    {

        if ((primCount%3) == 0)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,3, fout);
        }
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputTriangleStrip(const int iCurrentMaterial, const unsigned int surfaceFlags,
                            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout)
{
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    unsigned int evenodd=0;
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd-2; ++vindex, evenodd++)
    {

        OutputSurfHead(iCurrentMaterial,surfaceFlags,3, fout);
        if (evenodd%2==0) {
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+1, pVertexIndices, pTexCoords, pTexIndices, fout);
        } else {
            OutputVertex(vindex+1, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        }
        OutputVertex(vindex+2, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputTriangleFan(const int iCurrentMaterial, const unsigned int surfaceFlags,
                           const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout)
{
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    for(unsigned int vindex=drawArray->getFirst()+1; vindex<indexEnd-1; ++vindex)
    {
        OutputSurfHead(iCurrentMaterial,surfaceFlags,3, fout);
        OutputVertex(drawArray->getFirst(), pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindex+1, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuads(const int iCurrentMaterial, const unsigned int surfaceFlags,
                            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout)
{
    unsigned int primCount = 0;
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd; ++vindex,++primCount)
    {

        if ((primCount%4) == 0)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,4, fout);
        }
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuadStrip(const int iCurrentMaterial, const unsigned int surfaceFlags,
                            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout)
{
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd-2; vindex+=2)
    {
        OutputSurfHead(iCurrentMaterial,surfaceFlags,4, fout);
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindex+1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindex+3, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindex+2, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputPolygon(const int iCurrentMaterial, const unsigned int surfaceFlags,
                            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout)
{
    unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();
    OutputSurfHead(iCurrentMaterial,surfaceFlags,drawArray->getCount(), fout);
    for(unsigned int vindex=drawArray->getFirst(); vindex<indexEnd; vindex++)
    {
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
//=======  draw array length cases
void Geode::OutputLineDARR(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, ostream& fout)
{
    unsigned int vindex = drawArrayLengths->getFirst();
    for(osg::DrawArrayLengths::const_iterator primItr = drawArrayLengths->begin(); primItr <drawArrayLengths->end(); ++primItr)
    {
        unsigned int localPrimLength;
        localPrimLength = 2;

        for(GLsizei primCount = 0; primCount < *primItr; ++primCount)
        {
            if ((primCount%localPrimLength)==0)
            {
                OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
            }
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            ++vindex;
        }

    }
}

void Geode::OutputTriangleDARR(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, ostream& fout)
{
    unsigned int vindex = drawArrayLengths->getFirst();
    for(osg::DrawArrayLengths::const_iterator primItr = drawArrayLengths->begin(); primItr <drawArrayLengths->end(); ++primItr)
    {
        unsigned int localPrimLength;
        localPrimLength = 3;

        for(GLsizei primCount = 0; primCount < *primItr; ++primCount)
        {
            if ((primCount%localPrimLength)==0)
            {
                OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
            }
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            ++vindex;
        }

    }
}

void Geode::OutputQuadsDARR(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, ostream& fout)
{
    unsigned int vindex = drawArrayLengths->getFirst();
    for(osg::DrawArrayLengths::const_iterator primItr = drawArrayLengths->begin(); primItr <drawArrayLengths->end()-4; primItr+=4)
    {
        unsigned int localPrimLength;
        localPrimLength = 4;

        for(GLsizei primCount = 0; primCount < *primItr; ++primCount)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+1, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+2, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+3, pVertexIndices, pTexCoords, pTexIndices, fout);
            vindex+=4;
        }

    }
}
void Geode::OutputQuadStripDARR(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, ostream& fout)
{
    unsigned int vindex = drawArrayLengths->getFirst();
    for(osg::DrawArrayLengths::const_iterator primItr = drawArrayLengths->begin(); primItr <drawArrayLengths->end()-2; primItr+=2)
    {
        unsigned int localPrimLength;
        localPrimLength = *primItr;

        for(GLsizei primCount = 0; primCount < *primItr; ++primCount)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+1, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+3, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+2, pVertexIndices, pTexCoords, pTexIndices, fout);
            vindex+=2;
        }

    }
}
void Geode::OutputPolygonDARR(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, ostream& fout)
{
    unsigned int vindex = drawArrayLengths->getFirst();
    for(osg::DrawArrayLengths::const_iterator primItr = drawArrayLengths->begin(); primItr <drawArrayLengths->end(); ++primItr)
    {
        unsigned int localPrimLength;
        localPrimLength = *primItr;

        for(GLsizei primCount = 0; primCount < *primItr; ++primCount)
        {
            if ((primCount%localPrimLength)==0)
            {
                OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
            }
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            ++vindex;
        }

    }
}
void Geode::OutputTriangleStripDARR(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, ostream& fout)
{
    unsigned int vindex = drawArrayLengths->getFirst();
    for(osg::DrawArrayLengths::const_iterator primItr = drawArrayLengths->begin(); primItr <drawArrayLengths->end(); ++primItr)
    {
        // RFJ!!!!!!!!!! fixes for indexing
        int localPrimLength= *primItr;
        bool evenodd=true;

        for(GLsizei primCount = 0; primCount < localPrimLength - 2; ++primCount)
        {
            OutputSurfHead(iCurrentMaterial, surfaceFlags, 3, fout);
            if (evenodd) {
                OutputVertex(vindex + primCount, pVertexIndices, pTexCoords, pTexIndices, fout);
                OutputVertex(vindex + primCount + 1, pVertexIndices, pTexCoords, pTexIndices, fout);
            } else {
                OutputVertex(vindex + primCount + 1 , pVertexIndices, pTexCoords, pTexIndices, fout);
                OutputVertex(vindex + primCount, pVertexIndices, pTexCoords, pTexIndices, fout);
            }
            OutputVertex(vindex + primCount + 2, pVertexIndices, pTexCoords, pTexIndices, fout);
            evenodd=!evenodd;
        }
        vindex += localPrimLength;
    }
}
void Geode::OutputTriangleFanDARR(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, ostream& fout)
{
    unsigned int vindex = drawArrayLengths->getFirst();
    for(osg::DrawArrayLengths::const_iterator primItr = drawArrayLengths->begin(); primItr <drawArrayLengths->end(); ++primItr)
    {
        int localPrimLength = *primItr;

        for(GLsizei primCount = 0; primCount < localPrimLength - 2; primCount++)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,3, fout);
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+1+primCount, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex+2+primCount, pVertexIndices, pTexCoords, pTexIndices, fout);
        }
        vindex += localPrimLength;
    }
}

// DrawElements .... Ubyte
void Geode::OutputTriangleDelsUByte(const int iCurrentMaterial, const unsigned int surfaceFlags,
                                    const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
                                    const osg::DrawElementsUByte* drawElements, ostream& fout)
{
    unsigned int primLength =3;

    unsigned int primCount = 0;
    for(osg::DrawElementsUByte::const_iterator primItr=drawElements->begin(); primItr<drawElements->end(); ++primCount,++primItr)
    {
        if ((primCount%primLength) == 0)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
        }

        unsigned int vindex=*primItr;
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputTriangleStripDelsUByte(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUByte* drawElements, ostream& fout)
{
    unsigned int localPrimLength = 3;
    bool evenodd=true;
    for(osg::DrawElementsUByte::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-2; ++primItr)
    {

        unsigned int vindex=*primItr;
        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
        if (evenodd) {
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        } else {
            OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        }
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        evenodd=!evenodd;
    }
}
void Geode::OutputTriangleFanDelsUByte(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUByte* drawElements, ostream& fout)
{
    const unsigned int localPrimLength = 3;
    unsigned int vindex=*(drawElements->begin());
    for(osg::DrawElementsUByte::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-2; ++primItr)
    {
        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuadStripDelsUByte(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUByte* drawElements, ostream& fout)
{
    const unsigned int localPrimLength=4;
    for(osg::DrawElementsUByte::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-3; primItr+=2)
    {
        unsigned int vindex=*primItr;

        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+3);
        unsigned int vindexp3=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp3, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuadsDelsUByte(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUByte* drawElements, ostream& fout)
{
    const unsigned int localPrimLength=4;
    for(osg::DrawElementsUByte::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-3; primItr+=4)
    {
        unsigned int vindex=*primItr;

        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        unsigned int vindexp3=*(primItr+3);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp3, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputPolygonDelsUByte(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUByte* drawElements, ostream& fout)
{
    unsigned int primLength =drawElements->size();
    unsigned int primCount = 0;

    OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
    for(osg::DrawElementsUByte::const_iterator primItr=drawElements->begin(); primItr<drawElements->end(); ++primCount,++primItr)
    {
        unsigned int vindex=*primItr;
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
// DrawElements .... UShort
void Geode::OutputTriangleDelsUShort(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUShort* drawElements, ostream& fout)
{
    unsigned int primLength =3;

    unsigned int primCount = 0;
    for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin(); primItr<drawElements->end(); ++primCount,++primItr)
    {

        if ((primCount%primLength) == 0)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
        }

        unsigned int vindex=*primItr;
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputTriangleStripDelsUShort(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUShort* drawElements, ostream& fout)
{
    unsigned int localPrimLength = 3;
    bool evenodd=true;
    for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-2; ++primItr)
    {
        unsigned int vindex=*primItr;
        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
        if (evenodd) {
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        } else {
            OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        }
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        evenodd=!evenodd;
    }
}
void Geode::OutputTriangleFanDelsUShort(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUShort* drawElements, ostream& fout)
{
    const unsigned int localPrimLength = 3;
    unsigned int vindex=*(drawElements->begin());
    for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-2; ++primItr)
    {
        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuadStripDelsUShort(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUShort* drawElements, ostream& fout)
{
    const unsigned int localPrimLength=4;
    for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-3; primItr+=2)
    {
        unsigned int vindex=*primItr;

        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+3);
        unsigned int vindexp3=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp3, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuadsDelsUShort(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUShort* drawElements, ostream& fout)
{
    const unsigned int localPrimLength=4;
    for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-3; primItr+=4)
    {
        unsigned int vindex=*primItr;

        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        unsigned int vindexp3=*(primItr+3);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp3, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputPolygonDelsUShort(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUShort* drawElements, ostream& fout)
{
    unsigned int primLength =drawElements->size();
    unsigned int primCount = 0;

    OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
    for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin(); primItr<drawElements->end(); ++primCount,++primItr)
    {
        unsigned int vindex=*primItr;
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
// DrawElements .... UInt
void Geode::OutputTriangleDelsUInt(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUInt* drawElements, ostream& fout)
{
    unsigned int primLength =3;

    unsigned int primCount = 0;
    for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin(); primItr<drawElements->end(); ++primCount,++primItr)
    {

        if ((primCount%primLength) == 0)
        {
            OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
        }

        unsigned int vindex=*primItr;
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputTriangleStripDelsUInt(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUInt* drawElements, ostream& fout)
{
    unsigned int localPrimLength = 3;
    bool evenodd=true;
    for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-2; ++primItr)
    {

        unsigned int vindex=*primItr;
        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);
        if (evenodd) {
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        } else {
            OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
            OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        }
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        evenodd=!evenodd;
    }
}
void Geode::OutputTriangleFanDelsUInt(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUInt* drawElements, ostream& fout)
{
    const unsigned int localPrimLength = 3;
    unsigned int vindex=*(drawElements->begin());
    for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-2; ++primItr)
    {
        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuadStripDelsUInt(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUInt* drawElements, ostream& fout)
{
    const unsigned int localPrimLength = 4;
    for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-3; primItr+=2)
    {
        unsigned int vindex=*primItr;

        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+3);
        unsigned int vindexp3=*(primItr+2);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp3, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputQuadsDelsUInt(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUInt* drawElements, ostream& fout)
{
    const unsigned int localPrimLength=4;
    for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin(); primItr<drawElements->end()-3; primItr+=4)
    {
        unsigned int vindex=*primItr;

        unsigned int vindexp1=*(primItr+1);
        unsigned int vindexp2=*(primItr+2);
        unsigned int vindexp3=*(primItr+3);
        OutputSurfHead(iCurrentMaterial,surfaceFlags,localPrimLength, fout);

        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp1, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp2, pVertexIndices, pTexCoords, pTexIndices, fout);
        OutputVertex(vindexp3, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}
void Geode::OutputPolygonDelsUInt(const int iCurrentMaterial, const unsigned int surfaceFlags,
        const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,
        const osg::DrawElementsUInt* drawElements, ostream& fout)
{
    unsigned int primLength =drawElements->size();
    unsigned int primCount = 0;

    OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
    for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin(); primItr<drawElements->end(); ++primCount,++primItr)
    {
        unsigned int vindex=*primItr;
        OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
    }
}


const int Geode::ProcessMaterial(ostream& fout, const unsigned int igeode)
{
    // outputs materials from one geode
    // extended for multiple geode models, GWM 2003.
    // called before all the geometry as ac3d expects all materials in the header.
    // returns number of materials made
    unsigned int i;
    const unsigned int iNumDrawables = getNumDrawables();
    const osg::StateSet::RefAttributePair* pRAP;
    unsigned int iNumMaterials=0;
    // Let ac3d optimise the file
    // whiz round and get a list of materials
    // these may be duplicates of materials on other Geode/geometry sets.

    // Scan for materials
    for (i = 0; i < iNumDrawables; i++)
    {
        const osg::Drawable* Drawable = getDrawable(i);
        if (Drawable) {
            const osg::StateSet* theState = Drawable->getStateSet();
            if (theState) {
                pRAP = theState->getAttributePair(osg::StateAttribute::MATERIAL);
                if (NULL != pRAP)
                {
                    const osg::Material *pMaterial = dynamic_cast<const osg::Material*>(pRAP->first.get());
                    if (NULL != pMaterial)
                    {
                        const osg::Vec4& Diffuse = pMaterial->getDiffuse(osg::Material::FRONT_AND_BACK);
                        const osg::Vec4& Ambient = pMaterial->getAmbient(osg::Material::FRONT_AND_BACK);
                        const osg::Vec4& Emissive = pMaterial->getEmission(osg::Material::FRONT_AND_BACK);
                        const osg::Vec4& Specular = pMaterial->getSpecular(osg::Material::FRONT_AND_BACK);
                        fout << "MATERIAL "
                            << "\"osg"<<igeode<<"mat"<<i
                            << "\" rgb " << Diffuse[0] << " " << Diffuse[1] << " " << Diffuse[2] << " "
                            << "amb " << Ambient[0] << " " << Ambient[1] << " " << Ambient[2] << " "
                            << "emis " << Emissive[0] << " " << Emissive[1] << " " << Emissive[2] << " "
                            << "spec " << Specular[0] << " " << Specular[1] << " " << Specular[2] << " "
                            << "shi " << (int)pMaterial->getShininess(osg::Material::FRONT_AND_BACK) << " "
                            << "trans " << 1.0 - Diffuse[3] << std::endl;
                        iNumMaterials++;
                    }
                }
            }
        }
    }
    return iNumMaterials;
}
void Geode::ProcessGeometry(ostream& fout, const unsigned int ioffset)
{
    // outputs one geode
    // extended for multiple geode models, GWM 2003.
    unsigned int i, j; //, k, m;
    const unsigned int iNumDrawables = getNumDrawables();
    int iNumMaterials = 0;
    const osg::StateSet::RefAttributePair* pRAP=NULL;
    // Let ac3d optimise the file
    // whiz round and get a list of materails
    // write them out
    // write out an object for each drawable.
    // Write out world object
    int ngeometry=0; // not all drawables are geometry, text is not converted to facets.
    for (i = 0; i < iNumDrawables; i++)
    { // so here we count the geometries to be converted to AC3D
        const osg::Drawable* Drawable = getDrawable(i);
        if (Drawable) {
            const osg::Geometry *pGeometry = Drawable->asGeometry();
            if (NULL != pGeometry) ngeometry++;
        }
    }
    if (ngeometry>1) { // create a group
        fout << "OBJECT group" << std::endl;
        fout << "kids " << ngeometry << std::endl;
    }

    // Process each drawable in turn
    for (i = 0; i < iNumDrawables; i++)
    {
        const osg::Drawable* Drawable = getDrawable(i);
        if (Drawable) {
            const osg::StateSet* theState = Drawable->getStateSet();
            const osg::Geometry *pGeometry = Drawable->asGeometry();
            if (NULL != pGeometry)
            {
                int iCurrentMaterial = -1;

                if (theState) {
                    pRAP = theState->getAttributePair(osg::StateAttribute::MATERIAL);
                    if (NULL != pRAP)
                    {
                        iCurrentMaterial = ioffset+iNumMaterials;
                        iNumMaterials++;
                    }
                }

                //const osg::Vec3Array
                const osg::Array *pVertexArray = pGeometry->getVertexArray();
                if (NULL != pVertexArray)
                {
                    const unsigned int iNumVertices = pVertexArray->getNumElements(); // size();
                    const osg::IndexArray *pVertexIndices = 0;
                    const osg::IndexArray * pTexIndices = 0;
                    const osg::Vec2 *pTexCoords = NULL;
                    fout << "OBJECT poly" << std::endl;
                    fout << "name \"" << getName() << "\"" << std::endl;

                    // Use zero offset co-ordinate as location IS OPTIONAL
                    // fout << "loc " << "0 0 0" << std::endl;
                    /* you could have an offset for the coordinates;  it was suggested that the first coord would do.
                    if((*pVertexArray).getType()==osg::Array::Vec3ArrayType) {
                    const osg::Vec3Array *verts=static_cast<const osg::Vec3Array*>(pVertexArray);
                    fout << (*verts)[0][0] << " " << (*verts)[0][1] << " " << (*verts)[0][2] << std::endl;
                    } else if((*pVertexArray).getType()==osg::Array::Vec2ArrayType) {
                    const osg::Vec2Array *verts=static_cast<const osg::Vec2Array*>(pVertexArray);
                    fout << (*verts)[0][0] << " " << (*verts)[0][1] << " " << 0 << std::endl;
                    } else if((*pVertexArray).getType()==osg::Array::Vec4ArrayType) {
                    const osg::Vec4Array *verts=static_cast<const osg::Vec4Array*>(pVertexArray);
                    fout << (*verts)[0][0] << " " << (*verts)[0][1] << " " << (*verts)[0][2] << std::endl;
                    }
                    << (*pVertexArray)[0][0] << " "
                    << (*pVertexArray)[0][1] << " "
                    << (*pVertexArray)[0][2] << std::endl; */

                    // Check for a texture
                    if (theState)
                    {
                    const osg::StateSet::TextureAttributeList& TextureAttributeList = theState->getTextureAttributeList();
                    if (TextureAttributeList.size() > 0)
                    {
                        // Check for a single mode of GL_TEXTURE_2D and ON
                        const osg::StateSet::AttributeList& AttributeList = TextureAttributeList[0];
                        //                    assert(AttributeList.size() == 1);
                        const osg::Texture2D *pTexture2D = dynamic_cast<const osg::Texture2D*>(AttributeList.begin()->second.first.get());
                        //                    assert(NULL != pTexture2D);
                        if (NULL != pTexture2D)
                        {
                            pTexCoords = (const osg::Vec2*)pGeometry->getTexCoordArray(0)->getDataPointer();

                            // OK now see if I can calcualate the repeats
                            osg::Texture::WrapMode eWrapMode_s = pTexture2D->getWrap(osg::Texture::WRAP_S);
                            //osg::Texture::WrapMode eWrapMode_t = pTexture2D->getWrap(osg::Texture::WRAP_T);

                            if (eWrapMode_s == osg::Texture::REPEAT)
                            {
                                if (NULL != pTexCoords)
                                {
                                    // Find max min s coords
                                    float fMin = std::numeric_limits<float>::max();
                                    float fMax = std::numeric_limits<float>::min();
                                    unsigned int iNumTexCoords = pGeometry->getTexCoordArray(0)->getNumElements();

                                    for (j = 0; j < iNumTexCoords; j++)
                                    {
                                        if (pTexCoords[j][0] > fMax)
                                            fMax = pTexCoords[j][0];
                                        if (pTexCoords[j][0] < fMin)
                                            fMin = pTexCoords[j][0];
                                    }
                                    fMin = std::numeric_limits<float>::max();
                                    fMax = std::numeric_limits<float>::min();
                                    for (j = 0; j < iNumTexCoords; j++)
                                    {
                                        if (pTexCoords[j][1] > fMax)
                                            fMax = pTexCoords[j][1];
                                        if (pTexCoords[j][1] < fMin)
                                            fMin = pTexCoords[j][1];
                                    }
                                }
                            }

                            { // replace back slash with / for ac3d convention GWM Sep 2003
                                std::string fname=pTexture2D->getImage()->getFileName();
                                unsigned int pos;
                                for (pos=0; pos< fname.length(); pos++) {
                                    if (fname[pos] == '\\') fname[pos]='/';
                                }
                                fout << "texture \"" << fname << "\"" << std::endl;
                            }
                            // Temp frig
                            fout << "texrep 1 1" << std::endl;
                            fout << "texoff 0 0" << std::endl;
                        }
                    }
                    }

                    fout << "numvert " << iNumVertices << std::endl;
                    for (j = 0; j < iNumVertices; j++)
                    { // use 3 types of osg::Vec for coordinates....
                        if((*pVertexArray).getType()==osg::Array::Vec3ArrayType) {
                            const osg::Vec3Array *verts=static_cast<const osg::Vec3Array*>(pVertexArray);
                            fout << (*verts)[j][0] << " " << (*verts)[j][1] << " " << (*verts)[j][2] << std::endl;
                        } else if((*pVertexArray).getType()==osg::Array::Vec2ArrayType) {
                            const osg::Vec2Array *verts=static_cast<const osg::Vec2Array*>(pVertexArray);
                            fout << (*verts)[j][0] << " " << (*verts)[j][1] << " " << 0 << std::endl;
                        } else if((*pVertexArray).getType()==osg::Array::Vec4ArrayType) {
                            const osg::Vec4Array *verts=static_cast<const osg::Vec4Array*>(pVertexArray);
                            fout << (*verts)[j][0] << " " << (*verts)[j][1] << " " << (*verts)[j][2] << std::endl;
                        }
                    }


                    // Generate a surface for each primitive
                    unsigned int iNumSurfaces = 0; // complex tri-strip etc prims use more triangles
                    osg::Geometry::PrimitiveSetList::const_iterator pItr;
                    for(pItr = pGeometry->getPrimitiveSetList().begin(); pItr != pGeometry->getPrimitiveSetList().end(); ++pItr) {
                        const osg::PrimitiveSet* primitiveset = pItr->get();
                        //const osg::PrimitiveSet::Type type = primitiveset->getType();
                        unsigned int iNumPrimitives = primitiveset->getNumPrimitives();
                        unsigned int iNumIndices = primitiveset->getNumIndices();
                        GLenum mode=primitiveset->getMode();
                        switch(mode) {
                        case(osg::PrimitiveSet::POINTS):
                            iNumSurfaces+=1; // all points go in one big list
                            break;
                        case(osg::PrimitiveSet::LINES): // each line is a pair of vertices
                        case(osg::PrimitiveSet::TRIANGLES): // each tri = 3 verts
                        case(osg::PrimitiveSet::QUADS):
                        case(osg::PrimitiveSet::LINE_LOOP):
                        case(osg::PrimitiveSet::LINE_STRIP):
                        case(osg::PrimitiveSet::POLYGON):
                            iNumSurfaces+=iNumPrimitives;
                            break;
                        case(osg::PrimitiveSet::TRIANGLE_STRIP):
                        case(osg::PrimitiveSet::TRIANGLE_FAN):
                            iNumSurfaces+=iNumIndices-2*iNumPrimitives;
                            break;
                        case(osg::PrimitiveSet::QUAD_STRIP):
                            iNumSurfaces+=(iNumIndices-2*iNumPrimitives)/2;
                            break;
                        default:
                            break; // unknown shape
                        }
                    }
                    fout << "numsurf " << iNumSurfaces << std::endl;

                    for(pItr = pGeometry->getPrimitiveSetList().begin(); pItr != pGeometry->getPrimitiveSetList().end(); ++pItr)
                    {
                        const osg::PrimitiveSet* primitiveset = pItr->get();
                        GLenum mode=primitiveset->getMode();

                        unsigned int surfaceFlags = 0x00;

                        switch(mode)
                        {
                        case(osg::PrimitiveSet::POINTS):
                            surfaceFlags = 0x02;
                            break;
                        case(osg::PrimitiveSet::LINES):
                            surfaceFlags = 0x02;
                            break;
                        case(osg::PrimitiveSet::TRIANGLES):
                            break;
                        case(osg::PrimitiveSet::QUADS):
                            break;
                        default:
                            break; // compute later when =0.
                        }

//                        osg::StateAttribute::GLModeValue backface =theState->getMode(osg::StateAttribute::CULLFACE);
//                        if (backface==osg::StateAttribute::ON) surfaceFlags |= 0x10;
//                        else if (backface==osg::StateAttribute::OFF) surfaceFlags &= 0x0f;
                        const osg::DrawArrays* drawArray = static_cast<const osg::DrawArrays*>(primitiveset);
                        switch(primitiveset->getType())
                        {
                        case(osg::PrimitiveSet::DrawArraysPrimitiveType):
                            {
                                switch(mode)
                                {
                                case(osg::PrimitiveSet::POINTS):
                                    break;
                                case(osg::PrimitiveSet::LINES):
                                    OutputLines(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::LINE_LOOP):
                                    OutputLineLoop(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::LINE_STRIP):
                                    OutputLineStrip(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLES):
                                    OutputTriangle(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::QUADS):
                                    OutputQuads(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_STRIP):
                                    OutputTriangleStrip(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_FAN):
                                    OutputTriangleFan(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::QUAD_STRIP):
                                    OutputQuadStrip(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                case(osg::PrimitiveSet::POLYGON):
                                    OutputPolygon(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArray, fout);
                                    break;
                                default:
                                    break; // unknown shape
                                }
                                break;
                            }
                        case(osg::PrimitiveSet::DrawArrayLengthsPrimitiveType):
                            {

                                const osg::DrawArrayLengths* drawArrayLengths = static_cast<const osg::DrawArrayLengths*>(primitiveset);
                                switch(mode)
                                {
                                case(osg::PrimitiveSet::LINES):
                                    OutputLineDARR(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArrayLengths, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLES):
                                    OutputTriangleDARR(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArrayLengths, fout);
                                    break;
                                case(osg::PrimitiveSet::QUADS):
                                    OutputQuadsDARR(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArrayLengths, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_STRIP):
                                    OutputTriangleStripDARR(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArrayLengths, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_FAN):
                                    OutputTriangleFanDARR(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArrayLengths, fout);
                                    break;
                                case(osg::PrimitiveSet::QUAD_STRIP):
                                    OutputQuadStripDARR(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArrayLengths, fout);
                                    break;
                                case(osg::PrimitiveSet::POLYGON):
                                    OutputPolygonDARR(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawArrayLengths, fout);
                                    break;
                                default:
                                    break; // unknown shape
                                }
                                break;
                            }
                        case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
                            {
                                const osg::DrawElementsUByte* drawElements = static_cast<const osg::DrawElementsUByte*>(primitiveset);
                                switch(mode)
                                {
                                case(osg::PrimitiveSet::TRIANGLES):
                                    OutputTriangleDelsUByte(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::QUADS):
                                    OutputQuadsDelsUByte(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_STRIP):
                                    OutputTriangleStripDelsUByte(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_FAN):
                                    OutputTriangleFanDelsUByte(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::QUAD_STRIP):
                                    OutputQuadStripDelsUByte(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::POLYGON):
                                    OutputPolygonDelsUByte(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                default:
                                    break; // unknown shape
                                }

                                break;
                            }
                        case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
                            {
                                const osg::DrawElementsUShort* drawElements = static_cast<const osg::DrawElementsUShort*>(primitiveset);
                                switch(mode)
                                {
                                case(osg::PrimitiveSet::TRIANGLES):
                                    OutputTriangleDelsUShort(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::QUADS):
                                    OutputQuadsDelsUShort(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_STRIP):
                                    OutputTriangleStripDelsUShort(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_FAN):
                                    OutputTriangleFanDelsUShort(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::QUAD_STRIP):
                                    OutputQuadStripDelsUShort(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::POLYGON):
                                    OutputPolygonDelsUShort(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                default:
                                    break; // unknown shape
                                }
/*                                if (primLength == 0)
                                if (primLength == 0)
                                    primLength = primitiveset->getNumIndices();

                                const osg::DrawElementsUShort* drawElements = static_cast<const osg::DrawElementsUShort*>(primitiveset);

                                unsigned int primCount = 0;

                                for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin(); primItr!=drawElements->end(); ++primCount,++primItr)
                                {

                                    if ((primCount%primLength) == 0)
                                    {
        OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
                                    }

                                    unsigned int vindex=*primItr;
                                    OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
                                } */

                                break;
                            }
                        case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
                            {
                                const osg::DrawElementsUInt* drawElements = static_cast<const osg::DrawElementsUInt*>(primitiveset);
                                switch(mode)
                                {
                                case(osg::PrimitiveSet::TRIANGLES):
                                    OutputTriangleDelsUInt(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::QUADS):
                                    OutputQuadsDelsUInt(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_STRIP):
                                    OutputTriangleStripDelsUInt(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::TRIANGLE_FAN):
                                    OutputTriangleFanDelsUInt(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::QUAD_STRIP):
                                    OutputQuadStripDelsUInt(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                case(osg::PrimitiveSet::POLYGON):
                                    OutputPolygonDelsUInt(iCurrentMaterial,surfaceFlags, pVertexIndices, pTexCoords, pTexIndices, drawElements, fout);
                                    break;
                                default:
                                    break; // unknown shape
                                }
/*                                if (primLength == 0)
                                if (primLength == 0)
                                    primLength = primitiveset->getNumIndices();

                                const osg::DrawElementsUInt* drawElements = static_cast<const osg::DrawElementsUInt*>(primitiveset);

                                unsigned int primCount=0;
                                for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin(); primItr!=drawElements->end(); ++primCount,++primItr)
                                {

                                    if ((primCount%primLength)==0)
                                    {
        OutputSurfHead(iCurrentMaterial,surfaceFlags,primLength, fout);
                                    }

                                    unsigned int vindex=*primItr;
                                    OutputVertex(vindex, pVertexIndices, pTexCoords, pTexIndices, fout);
                                } */

                                break;
                            }
                        default:
                            {
                                break;
                            }
                        }
                    }
                }
                fout << "kids 0" << endl;
            }
        }
        else
        { // not sure what else it could be, but perhaps, perhaps, perhaps.
        }
    }
}
