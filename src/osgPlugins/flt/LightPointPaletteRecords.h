
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



struct SLightPointAnimationPalette
{
    SRecHeader  RecHeader;
    int32       reserved_0;
    char        name[256];
    int32       index;
	float32     period;     // in seconds
	float32     phaseDelay; // inb seconds, from start of period
	float32     enabledPeriod; // time on, in seconds
	float32     axis[3];    // for rotating animations
	uint32      flags;      // flags bits: 0 -- flashing
							//             1 -- rotating
							//             3 -- rotate counter clockwise
							//             4-31 -- reserved
	int32     animType;     // animation type: 0 -- flashing sequence
	                        //                 1 -- rotating
	                        //                 2 -- strobe
	                        //                 3 -- Morse code
	int32     morseTiming;  // Morse timing: 0 -- standard timing
		                    //               1 -- Farnsworth timing
	int32     wordRate;     // for Farnsworth timing
	int32     charRate;     // for Farnsworth timing
	char      morseString[1024];
	int32     numSequences; // for flashing sequences
};
// Repeated numSequenses times:
struct SLightPointAnimationSequence
{
	uint32    seqState;     // sequence state: 0 -- On
	                        //                 1 -- Off
	                        //                 2 -- Color Change
	float32   duration;     // duration of sequence in seconds
	uint32    seqColor;     // color, if state is On or Color Change
};

class LtPtAnimationPaletteRecord : public AncillaryRecord
{
    public:

        LtPtAnimationPaletteRecord();

        virtual Record* clone() const { return new LtPtAnimationPaletteRecord(); }
        virtual const char* className() const { return "LtPtAnimationPaletteRecord"; }
        virtual int classOpcode() const { return LIGHT_PT_ANIMATION_PALETTE_OP; }
        virtual size_t sizeofData() const { return sizeof(SLightPointAnimationPalette); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }

		SLightPointAnimationSequence* sequence( int idx );

		enum FlagsBits {
			FLASHING = 0x80000000,
			ROTATING = 0x40000000,
			ROT_COUNTER_CLOCKWISE = 0x20000000
		};
		enum AnimationType {
			SEQ_TYPE = 0,
			ROT_TYPE = 1,
			STROBE_TYPE = 2,
			MORSE_TYPE = 3
		};
		enum SequenceState {
			SEQ_ON = 0,
			SEQ_OFF = 1,
			SEQ_COLOR = 2
		};

    protected:
        virtual ~LtPtAnimationPaletteRecord();

        virtual void endian();
};


}; // end namespace flt


#endif
