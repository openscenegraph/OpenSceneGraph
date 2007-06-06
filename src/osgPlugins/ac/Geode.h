#ifndef AC3D_GEODE
#define AC3D_GEODE 1

#include <osg/Geode>
#include <osg/Group>

namespace ac3d
{
    class Geode : public osg::Geode
    {
    public:
        const int ProcessMaterial(std::ostream& fout, const unsigned int igeode);
        void ProcessGeometry(std::ostream& fout, const unsigned int igeode);
    private:
        void OutputTriangle(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputTriangleStrip(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputTriangleFan(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputQuads(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputQuadStrip(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputLineStrip(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputLineLoop(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputLines(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        void OutputPolygon(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrays* drawArray, std::ostream& fout);
        //== output for prims with draw array lengths
        void OutputLineDARR(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, std::ostream& fout);
        void OutputTriangleDARR(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, std::ostream& fout);
        void OutputTriangleStripDARR(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, std::ostream& fout);
        void OutputTriangleFanDARR(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, std::ostream& fout);
        void OutputQuadStripDARR(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, std::ostream& fout);
        void OutputQuadsDARR(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, std::ostream& fout);
        void OutputPolygonDARR(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawArrayLengths* drawArrayLengths, std::ostream& fout);
        // OutputTriangleDelsUByte
        // draw elements - 3 types: UByte, UShort, Uint
        void OutputTriangleDelsUByte(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUByte* drawElements, std::ostream& fout);
        void OutputTriangleStripDelsUByte(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUByte* drawElements, std::ostream& fout);
        void OutputTriangleFanDelsUByte(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUByte* drawElements, std::ostream& fout);
        void OutputQuadStripDelsUByte(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUByte* drawElements, std::ostream& fout);
        void OutputQuadsDelsUByte(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUByte* drawElements, std::ostream& fout);
        void OutputPolygonDelsUByte(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUByte* drawElements, std::ostream& fout);
        // for UShorts
        void OutputTriangleDelsUShort(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUShort* drawElements, std::ostream& fout);
        void OutputTriangleStripDelsUShort(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUShort* drawElements, std::ostream& fout);
        void OutputTriangleFanDelsUShort(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUShort* drawElements, std::ostream& fout);
        void OutputQuadStripDelsUShort(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUShort* drawElements, std::ostream& fout);
        void OutputQuadsDelsUShort(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUShort* drawElements, std::ostream& fout);
        void OutputPolygonDelsUShort(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUShort* drawElements, std::ostream& fout);
        // for UInts
        void OutputTriangleDelsUInt(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUInt* drawElements, std::ostream& fout);
        void OutputTriangleStripDelsUInt(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUInt* drawElements, std::ostream& fout);
        void OutputTriangleFanDelsUInt(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUInt* drawElements, std::ostream& fout);
        void OutputQuadStripDelsUInt(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUInt* drawElements, std::ostream& fout);
        void OutputQuadsDelsUInt(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUInt* drawElements, std::ostream& fout);
        void OutputPolygonDelsUInt(const int iCurrentMaterial,const unsigned int surfaceFlags,
            const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices,const osg::DrawElementsUInt* drawElements, std::ostream& fout);
        // general output for all types
        void OutputVertex(int Index, const osg::IndexArray *pVertexIndices, const osg::Vec2 *pTexCoords, const osg::IndexArray *pTexIndices, std::ostream& fout);
        inline void OutputSurfHead(const int iCurrentMaterial,const unsigned int surfaceFlags, const int nv, std::ostream& fout) {
            fout << "SURF 0x" << std::hex  << ((int)surfaceFlags) << std::endl;
            if (iCurrentMaterial >= 0)
                fout << "mat " <<  std::dec << iCurrentMaterial << std::endl;
            fout << "refs " <<  std::dec << nv << std::endl;
        }
    };

}

#endif
