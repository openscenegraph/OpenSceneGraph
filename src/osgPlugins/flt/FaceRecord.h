// FaceRecord.h

#ifndef __FLT_FACE_RECORD_H
#define __FLT_FACE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {



////////////////////////////////////////////////////////////////////
//
//                          FaceRecord
//
////////////////////////////////////////////////////////////////////


struct SFace
{
    SRecHeader    RecHeader;
    char    szIdent[8];                 // 7 char ASCII ID; 0 terminates
    int32    diIRColor;                 // IR Color Code
    int16    iObjectRelPriority;        // Polygon relative priority
    uint8    swDrawFlag;                // How to draw the polygon
                                        // = 0 Draw solid backfaced
                                        // = 1 Draw solid no backface
                                        // = 2 Draw closed wireframe (swapped in documentation)
                                        // = 3 Draw wireframe and not closed (swapped in documentation)
                                        // = 4 Surround with wireframe in alternate color
                                        // = 8 Omni-directional light
                                        // = 9 Unidirectional light
                                        // = 10 Bidirectional light
    uint8    swTexWhite;                // if TRUE, draw textured polygon white (see note 1 below)
    uint16   wPrimaryNameIndex;         // Color name index
    uint16   wSecondaryNameIndex;       // Alternate color name index
    uint8    swNotUsed;                 // Not used
    uint8    swTemplateTrans;           // Set template transparency
                                        // = 0 None
                                        // = 1 Fixed
                                        // = 2 Axis type rotate
                                        // = 4 Point rotate
    int16    iDetailTexturePattern;     // Detail texture pattern no. -1 if none
    int16    iTexturePattern;           // Texture pattern no. -1 if none
    int16    iMaterial;                 // Material code [0-63]. -1 if none
    int16    iSurfaceMaterial;          // Surface material code (for DFAD)
    int16    iFeature;                  // Feature ID (for DFAD)
    int32    diIRMaterial;              // IR Material codes
    uint16   wTransparency;             // Transparency
                                        // = 0 opaque
                                        // = 65535 for totally clear
    // version 11, 12 & 13 ends here!

    uint8    swInfluenceLODGen;         // LOD Generation Control
    uint8    swLinestyle;               // Linestyle Index
    uint32   dwFlags;                   // Flags (bits from left to right)
                                        // 0 = Terrain
                                        // 1 = No Color
                                        // 2 = No Alt Color
                                        // 3 = Packed color
                                        // 4 = Terrain culture cutout (footprint)
                                        // 5 = Hidden (not drawn)
                                        // 6 = Hidden (not drawn)
                                        // 7-31 Spare
    uint8    swLightMode;               // Lightmode
                                        // = 0 use face color, not illuminated
                                        // = 1 use vertex color, not illuminated
                                        // = 2 use face color and vertex normal
                                        // = 3 use vertex color and vertex normal

    uint8    Reserved1[7];              // Reserved
    color32  PrimaryPackedColor;        // Packed Color Primary (A, B, G, R)
    color32  SecondaryPackedColor;      // Packed Color Secondary (A, B, G, R)
    int16    iTextureMapIndex;          // Texture mapping index
    int16    iReserved2;
    uint32   dwPrimaryColorIndex;
    uint32   dwAlternateColorIndex;
    uint16   Reserved3[2];
};


class FaceRecord : public PrimNodeRecord
{
    public:

        enum DrawFlag {
            SOLID_BACKFACED = 0,
            SOLID_NO_BACKFACE = 1,
            WIREFRAME_CLOSED = 2,
            WIREFRAME_NOT_CLOSED = 3,
            SURROUND_ALTERNATE_COLOR = 4,
            OMNIDIRECTIONAL_LIGHT = 8,
            UNIDIRECTIONAL_LIGHT = 9,
            BIDIRECTIONAL_LIGHT = 10
        };

        enum LightMode {
            FACE_COLOR = 0,
            VERTEX_COLOR = 1,
            FACE_COLOR_LIGHTING = 2,
            VERTEX_COLOR_LIGHTING = 3
        };

        enum FlagBit {
            TERRAIN_BIT =       0x80000000,
            NO_COLOR_BIT =      0x40000000,
            NO_ALT_COLOR_BIT =  0x20000000,
            PACKED_COLOR_BIT =  0x10000000,
            FOOTPRINT_BIT =     0x08000000,
            HIDDEN_BIT =        0x04000000
        };

        FaceRecord();

        virtual Record* clone() const { return new FaceRecord(); }
        virtual const char* className() const { return "FaceRecord"; }
        virtual int classOpcode() const { return FACE_OP; }
        virtual size_t sizeofData() const { return sizeof(SFace); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SFace* getData() const { return (SFace*)_pData; }
        virtual const std::string getName( void ) const { return std::string(getData()->szIdent); }

        int numberOfVertices();
        int getVertexPoolOffset(int index);

    protected:

        virtual ~FaceRecord();

        virtual void endian();

        virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);
};



////////////////////////////////////////////////////////////////////
//
//                          VertexListRecord
//
////////////////////////////////////////////////////////////////////

typedef struct     SingleVertexListTag
{
    SRecHeader    RecHeader;
    int32    diSOffset[1];   // Byte offset to this vertex record in vertex table,
                            // the actual vertex of the face
} SSingleVertexList;



class VertexListRecord : public PrimNodeRecord
{
    public:
        VertexListRecord();

        virtual Record* clone() const { return new VertexListRecord(); }
        virtual const char* className() const { return "VertexListRecord"; }
        virtual int classOpcode() const { return VERTEX_LIST_OP; }
        virtual size_t sizeofData() const { return sizeof(SSingleVertexList); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SSingleVertexList* getData() const { return (SSingleVertexList*)_pData; }

        int numberOfVertices();
        int getVertexPoolOffset(int index);

    protected:
        virtual ~VertexListRecord();
        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                          MorphVertexListRecord
//
////////////////////////////////////////////////////////////////////

typedef struct     MorphVertexListTag
{
    struct VertexPair
    {
        uint32      dwOffset0;      // Byte offset into vertex palette of the 0% vertex
        uint32      dwOffset100;    // Byte offset into vertex palette of the 100% vertex
    };

    SRecHeader    RecHeader;
    VertexPair list[1];
} SMorphVertexList;



class MorphVertexListRecord : public PrimNodeRecord
{
    public:
        MorphVertexListRecord();

        virtual Record* clone() const { return new MorphVertexListRecord(); }
        virtual const char* className() const { return "MorphVertexListRecord"; }
        virtual int classOpcode() const { return MORPH_VERTEX_LIST_OP; }
        virtual size_t sizeofData() const { return sizeof(SMorphVertexList); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SMorphVertexList* getData() const { return (SMorphVertexList*)_pData; }

        int numberOfVertices();
        int getVertexPoolOffset(int index);

    protected:
        virtual ~MorphVertexListRecord();
        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                          VectorRecord
//
////////////////////////////////////////////////////////////////////

// A vector record is an ancillary record of the (pre V15.4) face node.
// Its only use is to provide the direction vector for old-style 
// unidirectional and bidirectional light point faces.

typedef struct     VectorTag
{
    SRecHeader    RecHeader;
    float32x3   Vec;
} SVector;



class VectorRecord : public AncillaryRecord
{
    public:
        VectorRecord();

        virtual Record* clone() const { return new VectorRecord(); }
        virtual const char* className() const { return "VectorRecord"; }
        virtual int classOpcode() const { return VECTOR_OP; }
        virtual size_t sizeofData() const { return sizeof(SVector); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SVector* getData() const { return (SVector*)_pData; }

    protected:
        virtual ~VectorRecord();

        virtual void endian();
};



}; // end namespace flt

#endif
