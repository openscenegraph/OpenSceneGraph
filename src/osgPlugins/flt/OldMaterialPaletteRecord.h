// OldMaterialPaletteRecord.h

#ifndef __FLT_OLD_MATERIAL_PALETTE_RECORD_H
#define __FLT_OLD_MATERIAL_PALETTE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct SOldMaterial
{
    SRecHeader        RecHeader;
    struct
    {
        float32x3    Ambient;         // Ambient  component of material
        float32x3    Diffuse;        // Diffuse  component of material
        float32x3    Specular;         // Specular component of material
        float32x3    Emissive;        // Emissive component of material
        float32        sfShininess;    // Shininess. [0.0-128.0]
        float32        sfAlpha;        // Alpha. [0.0-1.0], where 1.0 is opaque
        uint32      diFlags;        // bit 0    Materials used
                                    // bit 1-31 Spare
        uint32      spares[31];     // Spares for material
    } mat[64];
};


class OldMaterialPaletteRecord : public AncillaryRecord
{
    public:

        OldMaterialPaletteRecord();

        virtual Record* clone() const { return new OldMaterialPaletteRecord(); }
        virtual const char* className() const { return "OldMaterialPaletteRecord"; }
        virtual int classOpcode() const { return OLD_MATERIAL_PALETTE_OP; }
        virtual size_t sizeofData() const { return sizeof(SOldMaterial); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~OldMaterialPaletteRecord();

        virtual void endian();
//      virtual void decode();

//      virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);

};


}; // end namespace flt

#endif
