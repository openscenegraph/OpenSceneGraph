// flt2osg.h

#ifndef __FLT_2_OSG_H
#define __FLT_2_OSG_H

#include <osg/ref_ptr>
#include <osg/Vec4>

#include <map>
#include <vector>
#include <string>

#include "Record.h"
#include "GeoSetBuilder.h"

namespace osg {
class Object;
class Group;
class LOD;
class Geode;
class GeoSet;
class Material;
class Texture;
class Vec4;
};


namespace flt {

class Record;
class HeaderRecord;
class ColorPaletteRecord;
class MaterialPaletteRecord;
class OldMaterialPaletteRecord;
class TexturePaletteRecord;
class VertexPaletteRecord;
class VertexRecord;
class NormalVertexRecord;
class TextureVertexRecord;
class NormalTextureVertexRecord;
class GroupRecord;
class LodRecord;
class OldLodRecord;
class DofRecord;
class SwitchRecord;
class ObjectRecord;
class FaceRecord;
class MatrixRecord;
class ExternalRecord;
class LightPointRecord;
class VertexListRecord;
class LongIDRecord;

//class GeoSetBuilder;

struct SMaterial;

#define ADD_NORMAL(DGSET,VERTEX)                                \
        (DGSET)->addNormal(osg::Vec3(                           \
            (float)(VERTEX)->Normal.x(),                        \
            (float)(VERTEX)->Normal.y(),                        \
            (float)(VERTEX)->Normal.z()));

#define ADD_VERTEX_COLOR(DGSET,VERTEX,COLOR_POOL)               \
    {                                                           \
        if ((VERTEX)->swFlags & V_NO_COLOR_BIT)                 \
            (DGSET)->addColor(_faceColor);                      \
        else                                                    \
        {                                                       \
            if ((VERTEX)->swFlags & V_PACKED_COLOR_BIT)         \
                (DGSET)->addColor(pVert->PackedColor.get());    \
            else                                                \
                (DGSET)->addColor((COLOR_POOL)->getColor((VERTEX)->dwVertexColorIndex)); \
        }                                                       \
    }

#define ADD_TCOORD(DGSET,VERTEX)                                \
        (DGSET)->addTCoord(osg::Vec2(                           \
            (float)(VERTEX)->Texture.x(),                       \
            (float)(VERTEX)->Texture.y()));

#define ADD_OLD_COLOR(DGSET,VERTEX,COLOR_POOL)                  \
    {                                                           \
        if (COLOR_POOL)                                         \
            (DGSET)->addColor((COLOR_POOL)->getColor((VERTEX)->color_index)); \
        else                                                    \
            (DGSET)->addColor(osg::Vec4(1,1,1,1));              \
    }

#define ADD_OLD_TCOORD(DGSET,VERTEX)                            \
        (DGSET)->addTCoord(osg::Vec2(                           \
            (float)(VERTEX)->t[0],                              \
            (float)(VERTEX)->t[1]));

#define ADD_OLD_NORMAL(DGSET,VERTEX)                            \
        (DGSET)->addNormal(osg::Vec3(                           \
            (float)pVert->n[0] / (1<<30),                       \
            (float)pVert->n[1] / (1<<30),                       \
            (float)pVert->n[2] / (1<<30)));


class ConvertFromFLT
{
    public:

        ConvertFromFLT();
        virtual ~ConvertFromFLT();

        osg::Node* convert(HeaderRecord* rec);

        osg::Node* visitNode(osg::Group* osgParent,Record* rec);
        osg::Node* visitAncillary(osg::Group* osgParent, PrimNodeRecord* rec);
        osg::Node* visitPrimaryNode(osg::Group* osgParent, PrimNodeRecord* rec);

        osg::Node* visitLongID(osg::Group* osgParent, LongIDRecord* rec);

        osg::Node* visitHeader(osg::Group* osgParent, HeaderRecord* rec);
        osg::Node* visitColorPalette(osg::Group* osgParent, ColorPaletteRecord* rec);
        osg::Node* visitMaterialPalette(osg::Group* osgParent, MaterialPaletteRecord* rec);
        osg::Node* visitOldMaterialPalette(osg::Group* osgParent, OldMaterialPaletteRecord* rec);
        osg::Node* visitTexturePalette(osg::Group* osgParent, TexturePaletteRecord* rec);
        osg::Node* visitVertexPalette(osg::Group* osgParent, VertexPaletteRecord* rec);
        osg::Node* visitVertex(osg::Group* osgParent, VertexRecord* rec);
        osg::Node* visitNormalVertex(osg::Group* osgParent, NormalVertexRecord* rec);
        osg::Node* visitTextureVertex(osg::Group* osgParent, TextureVertexRecord* rec);
        osg::Node* visitNormalTextureVertex(osg::Group* osgParent, NormalTextureVertexRecord* rec);
        osg::Node* visitGroup(osg::Group* osgParent, GroupRecord* rec);
        osg::Node* visitLOD(osg::Group* osgParent, LodRecord* rec);
        osg::Node* visitOldLOD(osg::Group* osgParent, OldLodRecord* rec);
        osg::Node* visitDOF(osg::Group* osgParent, DofRecord* rec);
        osg::Node* visitSwitch(osg::Group* osgParent, SwitchRecord* rec);
        osg::Node* visitObject(osg::Group* osgParent, ObjectRecord* rec);
        osg::Node* visitMatrix(osg::Group* osgParent, MatrixRecord* rec);
        osg::Node* visitExternal(osg::Group* osgParent, ExternalRecord* rec);

        void visitFace(GeoSetBuilder* pParent, FaceRecord* rec);
        void visitLightPoint(GeoSetBuilder* pBuilder, LightPointRecord* rec);
        int  visitVertexList(GeoSetBuilder* pParent, VertexListRecord* rec);

    private:

        int addVertices(GeoSetBuilder* pBuilder, PrimNodeRecord* primRec);
        int addVertex(GeoSetBuilder* pBuilder, Record* rec);
        Record* getVertexFromPool(int nOffset);
        void regisiterVertex(int nOffset, Record* pRec);

        typedef std::map<int,Record*> VertexPaletteOffsetMap;
        VertexPaletteOffsetMap _VertexPaletteOffsetMap;

        int                 _diOpenFlightVersion;
        int                 _diCurrentOffset;
        unsigned short      _wObjTransparency;
        int                 _nSubfaceLevel;
        double              _unitScale;
        bool                _bHdrRgbMode;
        osg::Vec4           _faceColor;
};

    
}; // end namespace flt

#endif // __FLT_2_OSG_H

