// TexturePaletteRecord.h

#ifndef __FLT_TEXTURE_PALETTE_RECORD_H
#define __FLT_TEXTURE_PALETTE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct STexturePalette
{
    SRecHeader    RecHeader;
    char     szFilename[200];    // Filename of texture pattern
    int32    diIndex;            // Pattern index
    int32    diX;                // x location in texture palette
    int32    diY;                // y location in texture palette
};


// Version 10, 12, 13
struct SOldTexturePalette
{
    SRecHeader    RecHeader;
    char     szFilename[80];     // Filename of texture pattern
    int32    diIndex;            // Pattern index
    int32    diX;                // x location in texture palette
    int32    diY;                // y location in texture palette
};


class TexturePaletteRecord : public AncillaryRecord
{
    public:

        TexturePaletteRecord();

        virtual Record* clone() const { return new TexturePaletteRecord(); }
        virtual const char* className() const { return "TexturePaletteRecord"; }
        virtual int classOpcode() const { return TEXTURE_PALETTE_OP; }
        virtual size_t sizeofData() const { return sizeof(STexturePalette); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~TexturePaletteRecord();

        virtual void endian();
};


}; // end namespace flt

#endif


