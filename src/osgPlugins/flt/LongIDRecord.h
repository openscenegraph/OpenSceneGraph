// LongIDRecord.h

#ifndef __FLT_LONG_ID_RECORD_H
#define __FLT_LONG_ID_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct SLongID
{
    SRecHeader    RecHeader;
    char        szIdent[1];     // (Length - 4) ASCII ID of node
};



class LongIDRecord : public AncillaryRecord
{
    public:

        LongIDRecord();

        virtual Record* clone() const { return new LongIDRecord(); }
        virtual const char* className() const { return "LongIDRecord"; }
        virtual int classOpcode() const { return LONG_ID_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~LongIDRecord();

        virtual void endian();
};


}; // end namespace flt

#endif

