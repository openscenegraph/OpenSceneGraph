// flt2osg.h

#ifndef __FLT_2_OSG_H
#define __FLT_2_OSG_H

#include <osg/ref_ptr>
#include <osg/Vec4>

#include "Record.h"
#include "FltFile.h"

#include <map>
#include <vector>
#include <string>


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

class GeoSetBuilder;

struct SMaterial;


class ConvertFromFLT
{
    public:

        ConvertFromFLT(FltFile* pFltFile);
        virtual ~ConvertFromFLT();

        osg::Node* convert(Record* rec);

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

        Record* getVertexFromPool(int nOffset);
        void regisiterVertex(int nOffset, Record* pRec);

        typedef std::map<int,Record*> VertexPaletteOffsetMap;
        VertexPaletteOffsetMap _VertexPaletteOffsetMap;

        osg::ref_ptr<FltFile>    _pFltFile;

        int                 _diOpenFlightVersion;
        int                 _diCurrentOffset;
        unsigned short      _wObjTransparency;
        int                 _nSubfaceLevel;
        float               _sfHdrUnitScale;       // iMultDivUnit
        bool                _bHdrRgbMode;
};

    
}; // end namespace flt

#endif // __FLT_2_OSG_H

