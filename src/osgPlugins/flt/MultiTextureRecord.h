// MultiTextureRecord.h

#ifndef __FLT_MULTI_TEXTURE_RECORD_H
#define __FLT_MULTI TEXTURE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

struct STextureLayer {
    uint16        texture;
    uint16        effect;
    uint16        mapping;
    uint16        data;
    void endian() {
    ENDIAN( texture );
    ENDIAN( effect );
    ENDIAN( mapping );
    ENDIAN( data );
    };
};

struct SMultiTexture
{
    SRecHeader    RecHeader;
    uint32        layers;

    STextureLayer data[1];
};

class MultiTextureRecord : public AncillaryRecord
{
    public:

        MultiTextureRecord();

        virtual Record* clone() const { return new MultiTextureRecord(); }
        virtual const char* className() const { return "MultiTextureRecord"; }
        virtual int classOpcode() const { return MULTI_TEXTURE_OP; }
        virtual size_t sizeofData() const { return sizeof(SMultiTexture); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }

    protected:

        virtual ~MultiTextureRecord();

        virtual void endian();
};


}; // end namespace flt

#endif


