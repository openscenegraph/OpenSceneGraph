// GroupRecord.h

#ifndef __FLT_GROUP_RECORD_H
#define __FLT_GROUP_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

struct SGroup
{
    SRecHeader    RecHeader;
    char    szIdent[8];            // 7 char ASCII ID; 0 terminates
    int16    iGroupRelPriority;    // Group relative priority
    int16    iSpare;                // Spare for fullword alignment
    uint32    dwFlags;            // Flags (bits, from left to right)
                                // 0 = Reserved
                                // 1 = Forward animation
                                // 2 = Cycling animation
                                // 3 = Bounding box follows
                                // 4 = Freeze bounding box
                                // 5 = Default parent
                                // 6 - Backward animation
                                // 7-31 Spare
    int16    iSpecialId_1;        // Special effects ID 1 - defined by real time
    int16    iSpecialId_2;        // Special effects ID 2 - defined by real time
    int16    iSignificance;        // Significance Flags
    uint8    swLayer;            // Layer Number
    uint8    swReserved[5];        // Reserved
	int32    iLoopCount;           // Animation loop count
	float32  fLoopDuration;        // Animation loop duration
	float32  fLastFrameDuration;   // Duration of last frame in animation
};


class GroupRecord : public PrimNodeRecord
{
    public:

    enum FlagBit {
            FORWARD_ANIM     =    0x40000000,
            SWING_ANIM       =    0x20000000,
            BOUND_BOX_FOLLOW =    0x10000000,
            FREEZE_BOUND_BOX =    0x08000000,
            DEFAULT_PARENT   =    0x04000000,
            BACKWARD_ANIM    =    0x02000000
        };

        GroupRecord();

        virtual Record* clone() const { return new GroupRecord(); }
        virtual const char* className() const { return "GroupRecord"; }
        virtual int classOpcode() const { return GROUP_OP; }
        virtual size_t sizeofData() const { return sizeof(SGroup); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SGroup* getData() const { return (SGroup*)_pData; }
        virtual const std::string getName( void ) const { return std::string(getData()->szIdent); }

        int childLODs();

    protected:
        virtual ~GroupRecord();

        virtual void endian();
};



}; // end namespace flt

#endif

