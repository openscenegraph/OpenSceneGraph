// ExtensionRecord.h

#ifndef __FLT_EXTENSION_RECORD_H
#define __FLT_EXTENSION_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

namespace flt {
class Input;
};


namespace flt {


////////////////////////////////////////////////////////////////////
//
//                          ExtensionRecord
//
////////////////////////////////////////////////////////////////////

typedef struct ExtensionTag
{
    SRecHeader    RecHeader;

    char    szIdent[8];     // 7 char ASCII ID; 0 terminates
    char    site[8];        // Site ID - Unique site name
    char    reserved;       // Reserved
    char    revision;       // Revision - site specific
    uint16  code;           // Record code - site specific
//  char    n/a;            // Extended data - site specific
}SExtension;


class ExtensionRecord : public PrimNodeRecord
{
    public:
        ExtensionRecord();

        virtual Record* clone() const { return new ExtensionRecord(); }
        virtual const char* className() const { return "ExtensionRecord"; }
        virtual int classOpcode() const { return EXTENSION_OP; }
        virtual size_t sizeofData() const { return sizeof(SExtension); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:
        virtual ~ExtensionRecord();

        virtual void endian();
};


}; // end namespace flt

#endif
