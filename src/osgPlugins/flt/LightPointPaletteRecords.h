
#ifndef __FLT_LIGHT_POINT_PALETTE_RECORDS_H
#define __FLT_LIGHT_POINT_PALETTE_RECORDS_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct SLightPointAppearancePalette
{
    SRecHeader  RecHeader;
    int32       reserved_0;
    char        name[256];
    int32       index;
    int16       surfMatCode;
    int16       featureID;
    uint32      backColor;      // Back facing color for bidirectional light points
    int32       displayMode;    // Display Mode: 0 -- Raster
                                //               1 -- Calligraphic
                                //               2 -- Either
    float32     intensity;
    float32     backIntensity;
    float32     minDefocus;     // Min and max defocus values, 0.0 to 1.0,
    float32     maxDefocus;     //   for use with calligraphic lights
    int32       fadeMode;       // Fading mode: 0 -- Enable perspective fading calculations
                                //              1 -- Disable calculations
    int32       fogPunch;       // Fog punch mode: 0 -- Enable fog punch-through calculations
                                //                 1 -- Disable calculations
    int32       dirMode;        // Directional mode: 0 -- Enable directional calculations
                                //                   1 -- Disable calculations
    int32       rangeMode;      // Range Mode: 0 -- Use depth (Z) buffer calculations
                                //             1 -- Use slant range calculations
    float32     minPixelSize;
    float32     maxPixelSize;
    float32     actualSize;     // Actual light size in DB units
    float32     transFalloffPixelSize;
    float32     transFalloffExp;
    float32     transFalloffScalar;
    float32     transFalloffClamp;
    float32     fogScalar;
    float32     fogIntensity;
    float32     sizeDiffThreshold;
    int32       directionality; // Directionality: 0 -- Omnidirectional
                                //                 1 -- Unidirectional
                                //                 2 -- Bidirectional
    float32     horizLobeAngle;
    float32     vertLobeAngle;
    float32     lobeRollAngle;
    float32     dirFalloffExp;
    float32     dirAmbientIntensity;
    float32     significance;
    int32       flags;          //  Flag bits: 0 -- Reserved
                                //             1 -- Don't use back color
                                //             2 -- Reserved
                                //   ... several others ...
                                //             18-31 -- Spare
    float32     visRange;
    float32     fadeRangeRatio;
    float32     fadeInDurationSecs;
    float32     adeOutDurationSecs;
    float32     lodRangeRatio;
    float32     lodScale;
};


class LtPtAppearancePaletteRecord : public AncillaryRecord
{
    public:

        LtPtAppearancePaletteRecord();

        virtual Record* clone() const { return new LtPtAppearancePaletteRecord(); }
        virtual const char* className() const { return "LtPtAppearancePaletteRecord"; }
        virtual int classOpcode() const { return LIGHT_PT_APPEARANCE_PALETTE_OP; }
        virtual size_t sizeofData() const { return sizeof(SLightPointAppearancePalette); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }

    protected:

        virtual ~LtPtAppearancePaletteRecord();

        virtual void endian();
};


}; // end namespace flt


#endif
