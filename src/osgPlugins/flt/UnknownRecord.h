// UnknownRecord.h

#ifndef __FLT_UNKNOWN_RECORD_H
#define __FLT_UNKNOWN_RECORD_H

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


////////////////////////////////////////////////////////////////////

class UnknownRecord : public AncillaryRecord
{
    public:

        UnknownRecord();
        virtual ~UnknownRecord();

        virtual Record* clone() const { return new UnknownRecord(); }
        virtual const char* className() const { return "UnknownRecord"; }
        virtual int classOpcode() const { return 0; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:


};

}; // end namespace flt

#endif

