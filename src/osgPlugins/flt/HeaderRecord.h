// HeaderRecord.h

#ifndef __FLT_HEADER_RECORD_H
#define __FLT_HEADER_RECORD_H

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

namespace flt {

struct SHeader
{
    SRecHeader    RecHeader;
    char    szIdent[8];            // ID field (Not curr used)
    int32    diFormatRevLev;        // Format revision level
    int32    diDatabaseRevLev;    // Edit rev. level
    char    szDaTimLastRev[32];    // Date and time last rev.
    int16    iNextGroup;            // Next group ID number
    int16    iNextLOD;            // Next LOD ID number
    int16    iNextObject;        // Next object ID number
    int16    iNextPolygon;        // Next polygon ID number
    int16    iMultDivUnit;        // Unit multiplier/divisor, always = 1
    uint8    swVertexCoordUnit;    // Vertex coordinate units
                                // 0 = Meters
                                // 1 = Kilometers
                                // 4 = Feet
                                // 5 = Inches
                                // 8 = Nautical miles
    uint8    swTexWhite;            // if TRUE set texwhite on new polygons
    uint32  dwFlags;            // Flags (bits, from left to right)
                                // 0 = Save vertex normals
                                // 1 = Packed Color mode
                                // 2 = CAD View mode
                                // 3-31 = Spare
    int32    diNotUsed_1[6];        // Not Used
    int32    diProjection;        // Projection Type
                                // 0 = Flat Earth
                                // 1 = Trapezoidal
                                // 2 = Round Earth
                                // 3 = Lambert
                                // 4 = UTM
                                // 5 = Geodetic
                                // 6 = Geocentric
    int32    diNotUsed_2[7];        // Not Used
    int16    iNextDegOfFreedom;    // Next degree of freedom ID number
    int16    iVertexStorage;        // Vertex Storage Type
                                // 1 = Double Precision Float
    int32    diDatabaseSource;    // Database Source
                                // 100 = OpenFlight
                                // 200 = DIG I/DIG II
                                // 300 = Evans and Sutherland CT5A/CT6
                                // 400 = PSP DIG
                                // 600 = General Electric CIV/CV / PT2000
                                // 700 = Evans and Sutherland GDF
    float64    dfSWDatabaseCoordX;    // Southwest Database Coordinate (x,y)
    float64 dfSWDatabaseCoordY;
    float64 dfDatabaseOffsetX;    // Delta (x,y) to Place Database
    float64 dfDatabaseOffsetY;
    int16    iNextSound;            // Next Sound Bead Id
    int16    iNextPath;            // Next Path Bead ID
    int16    iNextClippingRegion;// Next Clipping Region Bead ID
    int16    iNextText;            // Next Text Bead ID
    int16    iNextBSP;            // Next BSP ID
    int16    iNextSwitch;        // Next Switch Bead ID
    float64x2    SWCorner;    // South West Corner Lat/Lon (NB: dec. degrees)
    float64x2    NECorner;    // North East Corner Lat/Lon (NB: dec. degrees)
    float64x2    Origin;        // Origin Lat/Lon (NB: dec. degrees, not radians)
    float64    dfLambertUpperLat;    // Lambert Upper Latitude
    float64    dfLambertLowerLat;    // Lambert Lower Latitude
    int16    iNextLightSource;    // Next Light Source ID Number
    int16    iNextLightPoint;      // Next Light Point ID number
    int16    iNextRoad;            // Next road bead ID number
    int16    iNextCat;            // Next CAT bead ID number
    int32    diEllipsoid;        // Earth ellipsoid model
                                // 0 - WGS 1984
                                // 1 - WGS 1972
                                // 2 - Bessel
                                // 3 - Clarke 1866
                                // 4 - NAD 1927

    // New with 15.7.0 ...
    int16 iNextAdaptiveNodeID;    // Next Adaptive node ID number
    int16 iNextCurveNodeID;       // Next Curve node ID number
    float64 dfDatabaseDeltaZ;     // Delta z to place database (used in 
                                  // conjunction with existing Delta x and 
                                  // Delta y values)
    float64 dfRadius;             // Radius (distance from database origin to 
                                  // farthest corner)
    uint16 iNextMeshNodeID;       // Next Mesh node ID number

    // New with 15.8
    int16 iUTMZone; // UTM zone 1-60, negative indicates southern hemisphere
    uint16 iNextLightPointSysID; // Light point system ID
    float64 dfEarthMajorAxis; // Custom ellipsoid Earth major axis
    float64 dfEarthMinorAxis; // Custom ellipsoid Earth minor axis
};


class HeaderRecord : public PrimNodeRecord
{
    public:

        HeaderRecord();

        virtual Record* clone() const { return new HeaderRecord(); }
        virtual const char* className() const { return "HeaderRecord"; }
        virtual int classOpcode() const { return HEADER_OP; }
        virtual size_t sizeofData() const { return sizeof(SHeader); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }

        SHeader* getData() const { return (SHeader*)_pData; }
        virtual const std::string getName( void ) const { return std::string(getData()->szIdent); }

        enum CoordUnit
        {
            METERS = 0,
            KILOMETERS = 1,
            FEET = 4,
            INCHES = 5,
            NAUTICAL_MILES = 8
        };

    protected:

        virtual ~HeaderRecord();

        virtual void endian();
        virtual void decode();

        virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);

};

}; // end namespace flt

#endif
