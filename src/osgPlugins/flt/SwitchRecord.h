// SwitchRecord.h

#ifndef __FLT_SWITCH_RECORD_H
#define __FLT_SWITCH_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

struct SSwitch
{
    SRecHeader    RecHeader;
    char    szIdent[8];         // 7 char ASCII ID; 0 terminates
    uint8   reserved[4];        // Reserved
    uint32  nCurrentMask;       // Current mask
    int32   nMasks;             // Number of masks
    int32   nWordsInMask;       // Number of 32 bit words required for each mask
                                // (number of children / 32 + number of children modulo 32)
    uint32  aMask[1];           // Variable Mask words 
                                // (length = number of words per mask * number of masks * 4 bytes)
};



class SwitchRecord : public PrimNodeRecord
{
    public:
        SwitchRecord();

        virtual Record* clone() const { return new SwitchRecord(); }
        virtual const char* className() const { return "SwitchRecord"; }
        virtual int classOpcode() const { return SWITCH_OP; }
        virtual size_t sizeofData() const { return sizeof(SSwitch); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SSwitch* getData() const { return (SSwitch*)_pData; }

    protected:
        virtual ~SwitchRecord();

        virtual void endian();
};



}; // end namespace flt

#endif

