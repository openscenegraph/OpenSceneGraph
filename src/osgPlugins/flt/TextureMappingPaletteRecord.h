// TextureMappingPaletteRecord.h

#ifndef __FLT_TEXTURE_MAPPING_PALETTE_RECORD_H
#define __FLT_TEXTURE_MAPPING_PALETTE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct STextureMapping
{
    SRecHeader    RecHeader;
    float32     reserved;           // Reserved
    int32       diIndex;            // Texture mapping index
    char        szName[20];         // Texture mapping name
    int32       diType;             // Texture mapping type
                                    //  0 = None
                                    //  1 = Put
                                    //  2 = 4 Point Put
                                    //  3 = Reserved
                                    //  4 = Spherical Project
                                    //  5 = Radial Project
                                    //  6 = Reserved
    int32       diWarpFlag;         // Warped flag; if TRUE, 8 point warp applied
    float64     dfMat[4][4];        // Transformation matrix (valid only for Types 1 & 2)
//  Variable    Variable;           // Parameters (see below for parameters for each mapping type)
};


// Parameters for Put Texture Mapping (Type 1)
struct SPutTextureMapping
{
    uint32      dwState;            // State of Put Texture tool
                                    //  0 = Start state - no points entered
                                    //  1 = One point entered
                                    //  2 = Two points entered
                                    //  3 = Three points entered
    int32       active;             // Active geometry point
                                    //  1 = Origin point
                                    //  2 = Alignment point
                                    //  3 = Shear point
    float64x3   lower_left;         // lower-left corner of bounding box
    float64x3   upper_right;        // upper-right corner of bounding box
    int32       sizeFlags[3];       // Use real world size flags for each of the three put points
    float64x3   dfTxtOrigin;        // Texture origin point
    float64x3   dfTxtAlignment;     // Texture alignment point
    float64x3   dfTxtShear;         // Texture shear point
    float64x3   dfGeoOrigin;        // Geometry origin point
    float64x3   dfGeoAlignment;     // Geometry alignment point
    float64x3   dfGeoShear;         // Geometry shear point
    int32       TxtActive;          // Active texture point
                                    //  1 = Origin point
                                    //  2 = Alignment point
                                    //  3 = Shear point
    int32       uvDisplayType;      // v15.8 (1580) UV display type
                                    //  1 = XY
                                    //  2 = UV
};


//Parameters for 4 Point Put Texture Mapping (Type 2)
struct SPointPutTextureMapping
{
    int32       state;              // State of Put Texture tool
                                    //  0 = Start state - no points entered
                                    //  1 = One point entered
                                    //  2 = Two points entered
                                    //  3 = Three points entered
                                    //  4 = Four points entered
    int32       active;             // Active geometry point
                                    //  1 = Origin point
                                    //  2 = Alignment point
                                    //  3 = Shear point
                                    //  4 = Perspective point
    float64x3   lower_left;         // lower-left corner of bounding box
    float64x3   upper_right;        // upper-right corner of bounding box
    int32       sizeFlags[4];       // Use real world size flags for each of the four put points
    float64x3   dfTxtOrigin;        // Texture origin point
    float64x3   dfTxtAlignment;     // Texture alignment point
    float64x3   dfTxtShear;         // Texture shear point
    float64x3   dfTxtPerspective;   // Texture perspective point
    float64x3   dfGeoOrigin;        // Geometry origin point
    float64x3   dfGeoAlignment;     // Geometry alignment point
    float64x3   dfGeoShear;         // Geometry shear point
    float64x3   dfGeoPerspective;   // Geometry perspective point

    int32       TxtActive;          // Active texture point
                                    //  1 = Origin point
                                    //  2 = Alignment point
                                    //  3 = Shear point
                                    //  4 = Perspective point
    int32       uvDisplayType;      // v15.8 (1580) UV display type
                                    //  1 = XY
                                    //  2 = UV
    float32     sfScale;            // Depth scale factor
	int32       reserved_0;         // New for 15.8
    float64     dfMat[4][4];        // Transformation matrix for the 4 point projection plane
	float32     sfURep;             // U repetition
	float32     sfVRep;             // V repetition
};


// Parameters for Spherical Project Mapping (Type 4)
struct SSphericalTextureMapping
{
    float32     sfScale;            // Scale
    float64x3   Center;             // Center of the projection sphere
    float32     sfMaxScale;         // Scale / (maximum dimension of the mapped geometry
                                    // bounding box)
    float32     sfMaxDimension;     // Maximum dimension of the mapped geometry
                                    // bounding box
};


// Parameters for Radial Project Mapping (Type 5)
struct SRadialTextureMapping
{
    int32       active;             // Active geometry point
                                    //  1 = End point 1 of cylinder center line
                                    //  2 = End point 2 of cylinder center line
    int32       reserved;           // Reserved
    float32     sfRadialScale;      // Radial scale
    float32     sfLengthScale;      // Scale along length of cylinder
    float64     dfMat[4][4];        // Trackplane to XY plane transformation matrix
    float64x3   endpoint1;          // End point 1 of cylinder center line
    float64x3   endpoint2;          // End point 2 of cylinder center line
};

// Parameters for Warped Mapping (Warped Flag Set)
struct SWarpedTextureMapping
{
    int32       active;             // Active geometry point
                                    //  0 = First warp FROM point
                                    //  1 = Second warp FROM point
                                    //  2 =Third warp FROM point
                                    //  3 = Fourth warp FROM point
                                    //  4 = Fifth warp FROM point
                                    //  5 = Sixth warp FROM point
                                    //  6 = Seventh warp FROM point
                                    //  7 = Eighth warp FROM point
                                    //  8 = First warp TO point
                                    //  9 = Second warp TO point
                                    //  10 = Third warp TO point
                                    //  11 = Fourth warp TO point
                                    //  12 = Fifth warp TO point
                                    //  13 = Sixth warp TO point
                                    //  14 = Seventh warp TO point
                                    //  15 = Eighth warp TO point
    int32       warpState;          // Warp tool state
                                    //  0 = Start state - no points entered
                                    //  1 = One FROM point entered
                                    //  2 = Two FROM point entered
                                    //  3 = Three FROM point entered
                                    //  4 = Four FROM point entered
                                    //  5 = Five FROM point entered
                                    //  6 = Six FROM point entered
                                    //  7 = Seven FROM point entered
                                    //  8 = All FROM point entered
    float64     dfMat[4][4];        // Trackplane to XY plane transformation matrix
/*
    Double 16*2 x, y of the first FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the second FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the third FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the fourth FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the fifth FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the sixth FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the seventh FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the eighth FROM point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the first TO point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the second TO point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the third TO point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the fourth TO point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the fifth TO point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the sixth TO point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the seventh TO point transformed to the XY plane by the above matrix
    Double 16*2 x, y of the eighth TO point transformed to the XY plane by the above matrix
*/
};



class TextureMappingPaletteRecord : public AncillaryRecord
{
    public:

        TextureMappingPaletteRecord();

        virtual Record* clone() const { return new TextureMappingPaletteRecord(); }
        virtual const char* className() const { return "TextureMappingPaletteRecord"; }
        virtual int classOpcode() const { return TEXTURE_MAPPING_PALETTE_OP; }
        virtual size_t sizeofData() const { return sizeof(STextureMapping); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~TextureMappingPaletteRecord();

        virtual void endian();

//      virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);

};


}; // end namespace flt

#endif
