//
// BSPRecord.h
//
// Author: Michael M. Morrison
//

#ifndef __FLT_BSP_RECORD_H
#define __FLT_BSP_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

struct SBSP
{
    SRecHeader    RecHeader;
    char    szIdent[8];         // 7 char ASCII ID; 0 terminates
    uint32  reserved;           // Reserved
    float64 planeA;             // Plane Equation A
    float64 planeB;             // Plane Equation B
    float64 planeC;             // Plane Equation C
    float64 planeD;             // Plane Equation D
};

class BSPRecord : public PrimNodeRecord
{
    public:
        BSPRecord();

        virtual Record* clone() const { return new BSPRecord(); }
        virtual const char* className() const { return "BSPRecord"; }
        virtual int classOpcode() const { return BSP_OP; }
        virtual size_t sizeofData() const { return sizeof(SBSP); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SBSP* getData() const { return (SBSP*)_pData; }

    protected:
        virtual ~BSPRecord();

        virtual void endian();
};



}; // end namespace flt

#endif

