// LightSourcePaletteRecord.h

#ifndef __FLT_LIGHT_SOURCE_PALETTE_RECORD_H
#define __FLT_LIGHT_SOURCE_PALETTE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


typedef struct LightSourcePaletteTag
{
    SRecHeader    RecHeader;
    int32        diIndex;            // Palette index
    int32        diReserved_1[2];
    char        szName[20];            // Light source name
    int32        diReserved_2;
    float32        sfAmbientRGBA[4];    //  Alpha comp. currently unused
    float32        sfDiffuseRGBA[4];    //    Alpha comp. currently unused
    float32        sfSpecularRGBA[4];    //    Alpha comp. currently unused
    int32        diLightType;        // 0 = INFINITE
                                    // 1 = LOCAL
                                    // 2 = SPOT
    int32        diReserved_3[10];
    float32        sfDropoff;            // Spot exponential dropoff term
    float32        sfCutoff;            // Spot cutoff angle (radians)
    float32        sfYaw;
    float32        sfPitch;
    float32        sfConstantAttuenation;
    float32        sfLinearAttuenation;
    float32        sfQuadraticAttuenation;
    int32        diModelingLight;    // TRUE/FALSE
    int32        diSpare[19];
} SLightSourcePalette;



class LightSourcePaletteRecord : public AncillaryRecord
{
    public:

        LightSourcePaletteRecord();

        virtual Record* clone() const { return new LightSourcePaletteRecord(); }
        virtual const char* className() const { return "LightSourcePaletteRecord"; }
        virtual int classOpcode() const { return LIGHT_SOURCE_PALETTE_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~LightSourcePaletteRecord();

        virtual void endian();

};


}; // end namespace flt

#endif

