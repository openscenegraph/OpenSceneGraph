// LodRecord.h

#ifndef __FLT_LOD_RECORD_H
#define __FLT_LOD_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

////////////////////////////////////////////////////////////////////
//
//                          LodRecord
//
////////////////////////////////////////////////////////////////////

struct SLevelOfDetail
{
    SRecHeader    RecHeader;
    char        szIdent[8];     // 7 char ASCII ID; 0 terminates
    int32        iSpare;            // Spare
    float64        dfSwitchInDist;    // Switch in distance
    float64        dfSwitchOutDist;// Switch out distance
    int16        iSpecialId_1;    // Special effects ID 1 - defined by real time
    int16        iSpecialId_2;    // Special effects ID 2 - defined by real time
    int32        diFlags;        // Flags (bits, from left to right)
                            // 0 = Use previous slant range
                            // 1 = SPT flag: 0 if replacement LOD, 1 for additive LOD
                            // 2 = Freeze center (don't recalculate)
                            // 3-31 Spare
    float64x3    Center;        // Center coordinate (x,y,z) of LOD block
    float64    dfTransitionRange;            // Transition Range for Morphing
	float64    dfSignificantSize;   // Multigen-Paradigm-internal for 15.8
};



class LodRecord : public PrimNodeRecord
{
    public:
        LodRecord();

        virtual Record* clone() const { return new LodRecord(); }
        virtual const char* className() const { return "LodRecord"; }
        virtual int classOpcode() const { return LOD_OP; }
        virtual size_t sizeofData() const { return sizeof(SLevelOfDetail); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SLevelOfDetail* getData() const { return (SLevelOfDetail*)_pData; }
        virtual const std::string getName( void ) const { return std::string(getData()->szIdent); }

    protected:
        virtual ~LodRecord();

        virtual void endian();
};



////////////////////////////////////////////////////////////////////
//
//                          OldLodRecord
//
////////////////////////////////////////////////////////////////////


struct SOldLOD
{
    SRecHeader    RecHeader;
    char        szIdent[8];         // 7 char ASCII ID; 0 terminates
    uint32        dwSwitchInDist;     // Switch in distance
    uint32        dwSwitchOutDist;    // Switch out distance
    int16        iSpecialId_1;       // Special effects ID 1 - defined by real time
    int16        iSpecialId_2;       // Special effects ID 2 - defined by real time
    int32        diFlags;            // Flags (bits, from left to right)
                                    // 0 = Use previous slant range
                                    // 1 = SPT flag: 0 if replacement LOD, 1 for additive LOD
                                    // 2 = Freeze center (don't recalculate)
                                    // 3-31 Spare
    int32       Center[3];          // Center coordinate (x,y,z) of LOD block
//  int32       spare[14];
};



class OldLodRecord : public PrimNodeRecord
{
    public:
        OldLodRecord();

        virtual Record* clone() const { return new OldLodRecord(); }
        virtual const char* className() const { return "OldLodRecord"; }
        virtual int classOpcode() const { return OLD_LOD_OP; }
        virtual size_t sizeofData() const { return sizeof(SOldLOD); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SOldLOD* getData() const { return (SOldLOD*)_pData; }
//      virtual const std::string getName( void ) const { return std::string(getData()->szIdent); }

    protected:
        virtual ~OldLodRecord();

        virtual void endian();
};


}; // end namespace flt

#endif

