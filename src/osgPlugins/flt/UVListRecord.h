// UVListRecord.h

#ifndef __FLT_UV_LIST_RECORD_H
#define __FLT_UV_LIST_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

struct SVertexUV {
    float32x2 coords;
    void endian() {
    ENDIAN( coords );
    };
};

struct SMorphUV {
    float32x2 coords0;
    float32x2 coords100;
    void endian() {
    ENDIAN( coords0 );
    ENDIAN( coords100 );
    };
};

struct SUVList
{
    SRecHeader    RecHeader;
    uint32        layers;

    union {
    SVertexUV vertex[1];
    SMorphUV  morph[1];
    } coords;
};

class UVListRecord : public AncillaryRecord
{
    public:

        UVListRecord();

        virtual Record* clone() const { return new UVListRecord(); }
        virtual const char* className() const { return "UVListRecord"; }
        virtual int classOpcode() const { return UV_LIST_OP; }
        virtual size_t sizeofData() const { return sizeof(SUVList); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~UVListRecord();

        virtual void endian();
};


}; // end namespace flt

#endif


