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
class Material;
class Texture;
};


namespace flt {

class Record;
class HeaderRecord;
class ColorPaletteRecord;
class MaterialPaletteRecord;
class OldMaterialPaletteRecord;
class TexturePaletteRecord;
class LtPtAppearancePaletteRecord;
class LtPtAnimationPaletteRecord;
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
class MeshRecord;
class MeshPrimitiveRecord;
class MatrixRecord;
class GeneralMatrixRecord;
class ExternalRecord;
class LightPointRecord;
class LightPointIndexRecord;
class LightPointSystemRecord;
class VertexListRecord;
class MorphVertexListRecord;
class LocalVertexPoolRecord;
class LongIDRecord;
class CommentRecord;
class InstanceDefinitionRecord;
class InstanceReferenceRecord;
class MultiTextureRecord;
class UVListRecord;
class LightSourceRecord;
class LightSourcePaletteRecord;
class BSPRecord;
struct SFace;

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
            (DGSET)->addColor((COLOR_POOL)->getOldColor((VERTEX)->color_index)); \
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

        osg::Group* convert(HeaderRecord* rec);

        osg::Group* visitAncillary(osg::Group& osgParent, osg::Group& osgPrimary, PrimNodeRecord* rec);
        osg::Group* visitPrimaryNode(osg::Group& osgParent, PrimNodeRecord* rec);

        // Ancillary records
        osg::Group* visitMatrix(osg::Group& osgParent, const osg::Group& osgPrimary, MatrixRecord* rec);
        osg::Group* visitGeneralMatrix(osg::Group& osgParent, const osg::Group& osgPrimary, GeneralMatrixRecord* rec);
        void visitLongID(osg::Group& osgParent, LongIDRecord* rec);
        void visitComment(osg::Node& osgParent, CommentRecord* rec);

        // Palette records
        void visitColorPalette(osg::Group& osgParent, ColorPaletteRecord* rec);
        void visitLightSourcePalette(osg::Group& osgParent, LightSourcePaletteRecord* rec);
        void visitMaterialPalette(osg::Group& osgParent, MaterialPaletteRecord* rec);
        void visitOldMaterialPalette(osg::Group& osgParent, OldMaterialPaletteRecord* rec);
        void visitTexturePalette(osg::Group& osgParent, TexturePaletteRecord* rec);
        void visitLtPtAppearancePalette(osg::Group& osgParent, LtPtAppearancePaletteRecord* rec);
        void visitLtPtAnimationPalette(osg::Group& osgParent, LtPtAnimationPaletteRecord* rec);
        void visitVertexPalette(osg::Group& osgParent, VertexPaletteRecord* rec);
        void visitVertex(osg::Group& osgParent, VertexRecord* rec);
        void visitNormalVertex(osg::Group& osgParent, NormalVertexRecord* rec);
        void visitTextureVertex(osg::Group& osgParent, TextureVertexRecord* rec);
        void visitNormalTextureVertex(osg::Group& osgParent, NormalTextureVertexRecord* rec);

        // Primary records
        osg::Group* visitHeader(HeaderRecord* rec);
        osg::Group* visitGroup(osg::Group& osgParent, GroupRecord* rec);
        osg::Group* visitBSP(osg::Group& osgParent, BSPRecord* rec);
        osg::Group* visitLightSource(osg::Group& osgParent, LightSourceRecord* rec);
        osg::Group* visitRoadConstruction(osg::Group& osgParent, GroupRecord* rec);
        osg::Group* visitRoadSegment(osg::Group& osgParent, GroupRecord* rec);
        osg::Group* visitLOD(osg::Group& osgParent, LodRecord* rec);
        osg::Group* visitOldLOD(osg::Group& osgParent, OldLodRecord* rec);
        osg::Group* visitDOF(osg::Group& osgParent, DofRecord* rec);
        osg::Group* visitSwitch(osg::Group& osgParent, SwitchRecord* rec);
        osg::Group* visitObject(osg::Group& osgParent, ObjectRecord* rec);
        osg::Group* visitExternal(osg::Group& osgParent, ExternalRecord* rec);
        osg::Group* visitInstanceDefinition(osg::Group& osgParent,InstanceDefinitionRecord* rec);
        osg::Group* visitInstanceReference(osg::Group& osgParent,InstanceReferenceRecord* rec);

        void visitFace(GeoSetBuilder* pParent, osg::Group& osgParent, FaceRecord* rec);
        void visitMesh(osg::Group& osgParent,GeoSetBuilder* pParent, MeshRecord* rec);
        void visitMeshPrimitive(osg::Group& osgParent, GeoSetBuilder* pBuilder, MeshPrimitiveRecord* rec);
        void visitLightPoint(GeoSetBuilder* pBuilder, osg::Group& osgParent, LightPointRecord* rec);
        void visitLightPoint(osg::Group& osgParent, LightPointRecord* rec);
        void visitLightPointIndex(osg::Group& osgParent, LightPointIndexRecord* rec);
        osg::Group* visitLightPointSystem(osg::Group& osgParent, LightPointSystemRecord* rec);
        int  visitVertexList(GeoSetBuilder* pParent, VertexListRecord* rec);
        int  visitMorphVertexList(GeoSetBuilder* pParent, MorphVertexListRecord* rec);
        int  visitLocalVertexPool(GeoSetBuilder* pBuilder, LocalVertexPoolRecord* rec);


        void setUseTextureAlphaForTransparancyBinning(bool flag) { _useTextureAlphaForTranspancyBinning=flag; }
        bool getUseTextureAlphaForTransparancyBinning() const { return _useTextureAlphaForTranspancyBinning; }
        void setDoUnitsConversion(bool flag) { _doUnitsConversion=flag; }
        bool getDoUnitsConversion() const { return _doUnitsConversion; }

    private:

        int addMeshPrimitives ( osg::Group &osgParent, GeoSetBuilder *pBuilder, MeshRecord *rec );
        int addVertices(GeoSetBuilder* pBuilder, osg::Group& osgParent, PrimNodeRecord* primRec);
        int addVertex(DynGeoSet* dgset, Record* rec);
        int addVertex(GeoSetBuilder* pBuilder, Record* rec) {return addVertex( pBuilder->getDynGeoSet(), rec);} ;
        Record* getVertexFromPool(int nOffset);
        void regisiterVertex(int nOffset, Record* pRec);
        void visitFaceOrMeshCommonCode(GeoSetBuilder* pBuilder, FaceRecord* rec);
        uint32 setMeshCoordinates ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry );
        uint32 setMeshNormals ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry );
        uint32 setMeshColors ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry );
        void setMeshTexCoordinates ( const uint32 &numVerts, const LocalVertexPoolRecord *pool, MeshPrimitiveRecord *mesh, osg::Geometry *geometry );

        void setCullFaceAndWireframe ( const SFace *pSFace, osg::StateSet *osgStateSet, DynGeoSet *dgset );
        void setDirectionalLight();
        void setLightingAndColorBinding ( const FaceRecord *rec, const SFace *pSFace, osg::StateSet *osgStateSet, DynGeoSet *dgset );
        void setColor ( FaceRecord *rec, SFace *pSFace, DynGeoSet *dgset, bool &bBlend );
        void setMaterial ( FaceRecord *rec, SFace *pSFace, osg::StateSet *osgStateSet, bool &bBlend );
        void setTexture ( FaceRecord *rec, SFace *pSFace, osg::StateSet *osgStateSet, DynGeoSet *dgset, bool &bBlend );
        void setTransparency ( osg::StateSet *osgStateSet, bool &bBlend );

        // multitexturing
        void addMultiTexture( DynGeoSet* dgset, MultiTextureRecord* mtr );
        void addUVList( DynGeoSet* dgset, UVListRecord* mtr );

        typedef std::map<int,Record*> VertexPaletteOffsetMap;
        VertexPaletteOffsetMap _VertexPaletteOffsetMap;

        int                     _diOpenFlightVersion;
        int                     _diCurrentOffset;
        unsigned short          _wObjTransparency;
        int                     _nSubfaceLevel;
        double                  _unitScale;
        bool                    _bHdrRgbMode;
        osg::Vec4               _faceColor;
        bool                    _useTextureAlphaForTranspancyBinning;
        bool                    _doUnitsConversion;

        osg::Group*             _osgParent;
        
        LocalVertexPoolRecord* _currentLocalVertexPool;
};

    
}; // end namespace flt

#endif // __FLT_2_OSG_H

