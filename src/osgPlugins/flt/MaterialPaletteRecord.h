// MaterialPaletteRecord.h

#ifndef __FLT_MATERIAL_PALETTE_RECORD_H
#define __FLT_MATERIAL_PALETTE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct SMaterial
{
    SRecHeader    RecHeader;
    int32       diIndex;
    char        szName[12];
    uint32      diFlags;        // bit 0    Materials used
                                // bit 1-31 Spare
    float32x3    Ambient;         // Ambient  component of material
    float32x3    Diffuse;        // Diffuse  component of material
    float32x3    Specular;         // Specular component of material
    float32x3    Emissive;        // Emissive component of material
    float32        sfShininess;    // Shininess. [0.0-128.0]
    float32        sfAlpha;        // Alpha. [0.0-1.0], where 1.0 is opaque
    int32        diSpare;
};


class MaterialPaletteRecord : public AncillaryRecord
{
    public:

        MaterialPaletteRecord();

        virtual Record* clone() const { return new MaterialPaletteRecord(); }
        virtual const char* className() const { return "MaterialPaletteRecord"; }
        virtual int classOpcode() const { return MATERIAL_PALETTE_OP; }
        virtual size_t sizeofData() const { return sizeof(SMaterial); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~MaterialPaletteRecord();

        virtual void endian();
//      virtual void decode();

//      virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);

};


}; // end namespace flt

#endif
