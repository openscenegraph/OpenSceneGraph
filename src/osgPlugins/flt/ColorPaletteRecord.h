// ColorPaletteRecord.h

#ifndef __FLT_COLOR_PALETTE_RECORD_H
#define __FLT_COLOR_PALETTE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

////////////////////////////////////////////////////////////////////
//
//                    ColorPaletteRecord
//
////////////////////////////////////////////////////////////////////


struct SColorName
{
    uint16      swSize;         // Length of color name entry
    int16       Reserved1;
    int16       nIndex;         // Color entry index
    int16       Reserved2;
    char        szName[1];      // Color name string (variable length, up to 80 bytes)
};


struct SColorPalette
{
    SRecHeader  RecHeader;
    char        szReserved[128];    // Reserved
    color32     Colors[1024];        // Color 0 - 1023
    int32       nNames;
//  Followed by SColorName. SColorName is of valiable length!
};


struct SOldColorPalette
{
    SRecHeader    RecHeader;
    color48     Colors[32];            // Color 0 - 31
    color48     FixedColors[56];    // Fixed Intensity Color 0 - 55 (4096-> )
};


class ColorPaletteRecord : public AncillaryRecord
{
    public:
        ColorPaletteRecord();

        virtual Record* clone() const { return new ColorPaletteRecord(); }
        virtual const char* className() const { return "ColorPaletteRecord"; }
        virtual int classOpcode() const { return COLOR_PALETTE_OP; }
//      virtual size_t sizeofData() const { return sizeof(SColorPalette); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SColorPalette* getData() const { return (SColorPalette*)_pData; }

    protected:
        virtual ~ColorPaletteRecord();

        virtual void endian();
};

}; // end namespace flt

#endif // __FLT_COLOR_PALETTE_RECORD_H
