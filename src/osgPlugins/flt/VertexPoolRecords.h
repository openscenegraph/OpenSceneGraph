// VertexPoolRecords.h

#ifndef __FLT_VERTEX_POOL_RECORDS_H
#define __FLT_VERTEX_POOL_RECORDS_H

#include <map>

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

#include <iostream>

namespace flt {

#define V_HARD_EDGE_BIT         BIT15
#define V_NORMAL_FROZEN_BIT     BIT14
#define V_NO_COLOR_BIT          BIT13
#define V_PACKED_COLOR_BIT      BIT12

////////////////////////////////////////////////////////////////////
//
//                    VertexPaletteRecord
//
////////////////////////////////////////////////////////////////////

typedef struct VertexTableHeaderTag    //Vertex Palette Header record
{
    SRecHeader    RecHeader;
    int32    diVertexTableLength;    //Length of this record plus pool
} SVertexTableHeader;



class VertexPaletteRecord : public AncillaryRecord
{
    public:
        VertexPaletteRecord();

        virtual Record* clone() const { return new VertexPaletteRecord(); }
        virtual const char* className() const { return "VertexPaletteRecord"; }
        virtual int classOpcode() const { return VERTEX_PALETTE_OP; }
        virtual size_t sizeofData() const { return sizeof(SVertexTableHeader); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        friend std::ostream& operator << (std::ostream& output, const VertexPaletteRecord& rec);

    protected:
        virtual ~VertexPaletteRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                       VertexRecord
//
////////////////////////////////////////////////////////////////////

struct SVertex                  // Vertex with Color Record Format
{
    SRecHeader  RecHeader;
    uint16      swColor;          // Color Name Index
    uint16      swFlags;          // Flags (bits, from left to right)
                                // 0 = Start Hard Edge
                                // 1 = Normal frozen
                                // 2 = no Vertex Color
                                // 3 = Packed Color
                                // 4-15 Spare
    float64x3   Coord;            // x,y,z coordinate
    color32     PackedColor;    // Packed color (A, B, G, R)
    uint32      dwVertexColorIndex;
};


class VertexRecord : public AncillaryRecord
{
    public:
        VertexRecord();
        virtual Record* clone() const { return new VertexRecord(); }
        virtual const char* className() const { return "VertexRecord"; }
        virtual int classOpcode() const { return VERTEX_C_OP; }
        virtual size_t sizeofData() const { return sizeof(SVertex); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SVertex* getData() const { return (SVertex*)_pData; }
        friend std::ostream& operator << (std::ostream& output, const VertexRecord& rec);

    protected:
        virtual ~VertexRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                       NormalVertexRecord
//
////////////////////////////////////////////////////////////////////


struct SNormalVertex                // Vertex with Normal Record Format
{
    SRecHeader  RecHeader;
    uint16      swColor;            // Color Name Index
    uint16      swFlags;            // Flags (bits, from left to right)
                                    // 0 = Start Hard Edge
                                    // 1 = Normal frozen
                                    // 2 = no Vertex Color
                                    // 3 = Packed Color
                                    // 4-15 Spare
    float64x3   Coord;              // x,y,z coordinate
    float32x3   Normal;             // Vertex normal
    color32     PackedColor;        // Packed color (A, B, G, R)
    uint32      dwVertexColorIndex;
};


class NormalVertexRecord : public AncillaryRecord
{
    public:
        NormalVertexRecord();
        virtual Record* clone() const { return new NormalVertexRecord(); }
        virtual const char* className() const { return "NormalVertexRecord"; }
        virtual int classOpcode() const { return VERTEX_CN_OP; }
        virtual size_t sizeofData() const { return sizeof(SNormalVertex); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SNormalVertex* getData() const { return (SNormalVertex*)_pData; }
        friend std::ostream& operator << (std::ostream& output, const NormalVertexRecord& rec);

    protected:
        virtual ~NormalVertexRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                     TextureVertexRecord
//
////////////////////////////////////////////////////////////////////

struct STextureVertex           // Vertex with Texture Record Format
{
    SRecHeader   RecHeader;
    uint16       swColor;       // Color Name Index
    uint16       swFlags;       // Flags (bits, from left to right)
                                // 0 = Start Hard Edge
                                // 1 = Normal frozen
                                // 2 = no Vertex Color
                                // 3 = Packed Color
                                // 4-15 Spare
    float64x3   Coord;          // x,y,z coordinate
    float32x2   Texture;        // Texture (u,v)
    color32     PackedColor;    // Packed color (A, B, G, R)
    uint32      dwVertexColorIndex;
};


class TextureVertexRecord : public AncillaryRecord
{
    public:
        TextureVertexRecord();
        virtual Record* clone() const { return new TextureVertexRecord(); }
        virtual const char* className() const { return "TextureVertexRecord"; }
        virtual int classOpcode() const { return VERTEX_CT_OP; }
        virtual size_t sizeofData() const { return sizeof(STextureVertex); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual STextureVertex* getData() const { return (STextureVertex*)_pData; }
        friend std::ostream& operator << (std::ostream& output, const TextureVertexRecord& rec);

    protected:
        virtual ~TextureVertexRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                    NormalTextureVertexRecord
//
////////////////////////////////////////////////////////////////////

struct SNormalTextureVertex     //Vertex with Normal and Texture Format
{
    SRecHeader  RecHeader;
    uint16      swColor;        // Color Name Index
    uint16      swFlags;        // Flags (bits, from left to right)
                                // 0 = Start Hard Edge
                                // 1 = Normal frozen
                                // 2 = no Vertex Color
                                // 3 = Packed Color
                                // 4-15 Spare
    float64x3   Coord;          // x,y,z coordinate
    float32x3   Normal;         // Vertex normal
    float32x2   Texture;        // Texture (u,v)
    color32     PackedColor;    // Packed color (A, B, G, R)
    uint32      dwVertexColorIndex;
};


class NormalTextureVertexRecord : public AncillaryRecord
{
    public:
        NormalTextureVertexRecord();
        virtual Record* clone() const { return new NormalTextureVertexRecord(); }
        virtual const char* className() const { return "NormalTextureVertexRecord"; }
        virtual int classOpcode() const { return VERTEX_CNT_OP; }
        virtual size_t sizeofData() const { return sizeof(SNormalTextureVertex); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SNormalTextureVertex* getData() const { return (SNormalTextureVertex*)_pData; }
        friend std::ostream& operator << (std::ostream& output, const NormalTextureVertexRecord& rec);

    protected:
        virtual ~NormalTextureVertexRecord();

        virtual void endian();
};


}; // end namespace flt

#endif // __FLT_VERTEX_POOL_RECORDS_H
