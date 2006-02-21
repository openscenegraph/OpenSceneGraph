// LightPointRecord.h

#ifndef __FLT_LIGHT_POINT_RECORD_H
#define __FLT_LIGHT_POINT_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

struct SLightPoint
{
    SRecHeader    RecHeader;
    char        szIdent[8];         // 7 char ASCII ID; 0 terminates
    int16       iMaterial;          // Surface material code (for DFAD)
    int16       iFeature;           // Feature ID (for DFAD)
    int32        dwBackColor;        // Back color for all bidirectional points
    int32       diMode;             // Display mode
                                    // 0 = RASTER
                                    // 1 = CALLIGRAPHIC
                                    // 2 = EITHER
    float32     sfIntensityFront;   // Intensity - scalar for front colors
    float32     sfIntensityBack;    //           - scalar for back color
    float32     sfMinDefocus;       // Minimum defocus - limit (0.0 - 1.0) for calligraphic points
    float32     sfMaxDefocus;       // Maximum defocus - limit (0.0 - 1.0) for calligraphic points
    int32       diFadeMode;         // Fading mode
                                    // 0 = enable perspective fading calculations
                                    // 1 = disable calculations
    int32       diFogPunchMode;     // Fog Punch mode
                                    // 0 = enable fog punch through calculations
                                    // 1 = disable calculations
    int32       diDirectionalMode;  // Directional mode
                                    // 0 = enable directional calculations
                                    // 1 = disable calculations
    int32       diRangeMode;        // Range mode
                                    // 0 = use depth (Z) buffer calculation
                                    // 1 = use slant range calculation
    float32     sfMinPixelSize;     // Minimum pixel size
                                    // Minimum diameter of points in pixels
    float32     sfMaxPixelSize;     // Maximum pixel size
                                    // Maximum diameter of points in pixels
    float32     afActualPixelSize;  // Actual size
                                    // Actual diameter of points in database coordinates
    float32     sfTranspFalloff;    // Transparent falloff pixel size
                                    // Diameter in pixels when points become transparent
    float32     sfTranspFalloffExponent;  // Transparent falloff exponent
                                    // >= 0 - falloff multiplier exponent (1.0 = linear falloff)
    float32     sfTranspFalloffScalar;    // Transparent falloff scalar
                                    // > 0 - falloff multiplier scale factor
    float32     sfTranspFalloffClamp;     // Transparent falloff clamp
                                    // Minimum permissible falloff multiplier result
    float32     sfFog;              // Fog scalar
                                    // >= 0 - adjusts range of points for punch threw effect.
    float32     sfReserved;
    float32     sfSize;             // Size difference threshold
                                    // Point size transition hint to renderer
    int32       diDirection;        // Directional type
                                    // 0 = OMNIDIRECTIONAL
                                    // 1 = UNIDIRECTIONAL
                                    // 2 = BIDIRECTIONAL
    float32     sfLobeHoriz;        // Horizontal lobe angle [degrees]
    float32     sfLobeVert;         // Vertical lobe angle [degrees]
    float32     sfLobeRoll;         // Rotation of lobe about local Y axis [degrees]
    float32     sfFalloff;          // Directional falloff exponent
                                    // >= 0 - falloff multiplier exponent
                                    // (1.0 = linear falloff)
    float32     sfAmbientIntensity; // Directional ambient intensity
    float32     sfAnimPeriod;       // Animation period [seconds]
    float32     sfAnimPhaseDelay;   // Animation phase delay [seconds]
    float32     sfAnimPeriodEnable; // Animation enabled period [seconds]
    float32     sfSignificance;     // Drop out priority for RASCAL lights (0.0 - 1.0)
    int32       sfDrawOrder;        // Calligraphic draw order
    uint32      sfFlags;            // Flags (bits, from left to right)
                                    // 0 = reserved
                                    // 1 = No back color
                                    // TRUE = don’t use back color for
                                    // bidirectional points
                                    // FALSE = use back color for
                                    // bidirectional points
                                    // 2 = reserved
                                    // 3 = Calligraphic proximity occulting (Debunching)
                                    // 4 = Reflective, non-emissive point
                                    // 5-7 = Randomize intensity
                                    // 0 = never
                                    // 1 = low
                                    // 2 = medium
                                    // 3 = high
                                    // 8 = Perspective mode
                                    // 9 = Flashing
                                    // 10 = Rotating
                                    // 11 = Rotate Counter Clockwise
                                    // Direction of rotation about local Z axis
                                    // 12 = reserved
                                    // 13-14 = Quality
                                    // 0 = Low
                                    // 1 = Medium
                                    // 2 = High
                                    // 3 = Undefined
                                    // 15 = Visible during day
                                    // 16 = Visible during dusk
                                    // 17 = Visible during night
                                    // 18-31 = Spare
    float32x3   animRot;            // Axis of rotation for rotating animation
};

class LightPointRecord : public PrimNodeRecord
{
    public:
        LightPointRecord();

        virtual Record* clone() const { return new LightPointRecord(); }
        virtual const char* className() const { return "LightPointRecord"; }
        virtual int classOpcode() const { return LIGHT_POINT_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SLightPoint* getData() const;
        
    protected:
        virtual ~LightPointRecord();

        virtual void endian();
};



struct SLightPointIndex
{
    SRecHeader  RecHeader;
    char        szIdent[8];         // 7 char ASCII ID; 0 terminates
    int32       iAppearanceIndex;   // Index into lt pt appearance palette
    int32       iAnimationIndex;    // Index into lt pt animation palette
    int32       iDrawOrder;         // Calligraphic draw order
    int32       iReserved_0;        // Reserved
};

class LightPointIndexRecord : public PrimNodeRecord
{
    public:
        LightPointIndexRecord();

        virtual Record* clone() const { return new LightPointIndexRecord(); }
        virtual const char* className() const { return "LightPointIndexRecord"; }
        virtual int classOpcode() const { return INDEXED_LIGHT_PT_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SLightPointIndex* getData() const { return (SLightPointIndex*)_pData; }

    protected:
        virtual ~LightPointIndexRecord();

        virtual void endian();
};




}; // end namespace flt

#endif

