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

typedef struct LevelOfDetailTag
{
	SRecHeader	RecHeader;
	char	    szIdent[8]; 	// 7 char ASCII ID; 0 terminates
	int32	    iSpare;			// Spare
	float64	    dfSwitchInDist;	// Switch in distance
	float64	    dfSwitchOutDist;// Switch out distance
	int16	    iSpecialId_1;	// Special effects ID 1 - defined by real time
	int16	    iSpecialId_2;	// Special effects ID 2 - defined by real time
	int32	    diFlags;		// Flags (bits, from left to right)
							// 0 = Use previous slant range
							// 1 = SPT flag: 0 if replacement LOD, 1 for additive LOD
							// 2 = Freeze center (don't recalculate)
							// 3-31 Spare
	float64x3	Center;	    // Center coordinate (x,y,z) of LOD block
	float64	dfTransitionRange;			// Transition Range for Morphing
} SLevelOfDetail;



class LodRecord : public PrimNodeRecord
{
    public:
        LodRecord();

        virtual Record* clone() const { return new LodRecord(); }
        virtual const char* className() const { return "LodRecord"; }
        virtual int classOpcode() const { return LOD_OP; }
        virtual int sizeofData() const { return sizeof(SLevelOfDetail); }
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


typedef struct OldLodTag
{
	SRecHeader	RecHeader;
	char	    szIdent[8]; 	// 7 char ASCII ID; 0 terminates
	int32	    iSpare;			// Spare
	float64	    dfSwitchInDist;	// Switch in distance
	float64	    dfSwitchOutDist;// Switch out distance
	int16	    iSpecialId_1;	// Special effects ID 1 - defined by real time
	int16	    iSpecialId_2;	// Special effects ID 2 - defined by real time
	int32	    diFlags;		// Flags (bits, from left to right)
							// 0 = Use previous slant range
							// 1 = SPT flag: 0 if replacement LOD, 1 for additive LOD
							// 2 = Freeze center (don't recalculate)
							// 3-31 Spare
	float64x3	Center;	    // Center coordinate (x,y,z) of LOD block
	float64	dfTransitionRange;			// Transition Range for Morphing
} SOldLOD;



class OldLodRecord : public PrimNodeRecord
{
    public:
        OldLodRecord();

        virtual Record* clone() const { return new OldLodRecord(); }
        virtual const char* className() const { return "OldLodRecord"; }
        virtual int classOpcode() const { return OLD_LOD_OP; }
        virtual int sizeofData() const { return sizeof(SOldLOD); }
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

