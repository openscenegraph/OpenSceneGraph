// VertexPoolRecords.cpp

#include "flt.h"
#include "Registry.h"
#include "VertexPoolRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       VertexPaletteRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<VertexPaletteRecord> g_VertexPaletteProxy;

VertexPaletteRecord::VertexPaletteRecord()
{
}


// virtual
VertexPaletteRecord::~VertexPaletteRecord()
{
}


// virtual
void VertexPaletteRecord::endian()
{
    SVertexTableHeader *pVertexTableHeader = (SVertexTableHeader*)getData();

    ENDIAN( pVertexTableHeader->diVertexTableLength );
}


std::ostream& operator << (std::ostream& output, const VertexPaletteRecord& rec)
{
    output << rec.className();
    return output;               // to enable cascading
}


////////////////////////////////////////////////////////////////////
//
//                       VertexRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<VertexRecord> g_VertexProxy;

VertexRecord::VertexRecord()
{
}


// virtual
VertexRecord::~VertexRecord()
{
}


// virtual
void VertexRecord::endian()
{
    SVertex *pVertex = (SVertex*)getData();

    ENDIAN( pVertex->swColor );
    ENDIAN( pVertex->swFlags );
    pVertex->Coord.endian();
    ENDIAN( pVertex->dwVertexColorIndex );
}


std::ostream& operator << (std::ostream& output, const VertexRecord& rec)
{
    output << rec.className() << " "
        << rec.getData()->swFlags << " "
        << rec.getData()->Coord;
    return output;               // to enable cascading
}


////////////////////////////////////////////////////////////////////
//
//                       NormalVertexRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<NormalVertexRecord> g_NormalVertexProxy;

NormalVertexRecord::NormalVertexRecord()
{
}


// virtual
NormalVertexRecord::~NormalVertexRecord()
{
}


// virtual
void NormalVertexRecord::endian()
{
    SNormalVertex *pVertex = (SNormalVertex*)getData();

    ENDIAN( pVertex->swColor );
    ENDIAN( pVertex->swFlags );
    pVertex->Coord.endian();
    pVertex->Normal.endian();
    //    ENDIAN( pVertex->PackedColor );
    ENDIAN( pVertex->dwVertexColorIndex );
}


std::ostream& operator << (std::ostream& output, const NormalVertexRecord& rec)
{
    output << rec.className() << " "
        << rec.getData()->swFlags << " "
        << rec.getData()->Coord;
    return output;               // to enable cascading
}


////////////////////////////////////////////////////////////////////
//
//                     TextureVertexRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<TextureVertexRecord> g_TextureVertexProxy;

TextureVertexRecord::TextureVertexRecord()
{
}


// virtual
TextureVertexRecord::~TextureVertexRecord()
{
}


// virtual
void TextureVertexRecord::endian()
{
    STextureVertex *pVertex = (STextureVertex*)getData();

    ENDIAN( pVertex->swColor );
    ENDIAN( pVertex->swFlags );
    pVertex->Coord.endian();
    pVertex->Texture.endian();
    //    ENDIAN( pVertex->PackedColor );
    ENDIAN( pVertex->dwVertexColorIndex );
}


std::ostream& operator << (std::ostream& output, const TextureVertexRecord& rec)
{
    output << rec.className() << " "
        << rec.getData()->swFlags << " "
        << rec.getData()->Coord;
    return output;               // to enable cascading
}


////////////////////////////////////////////////////////////////////
//
//                    NormalTextureVertexRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<NormalTextureVertexRecord> g_NormalTextureVertexProxy;

NormalTextureVertexRecord::NormalTextureVertexRecord()
{
}


// virtual
NormalTextureVertexRecord::~NormalTextureVertexRecord()
{
}


// virtual
void NormalTextureVertexRecord::endian()
{
    SNormalTextureVertex *pVertex = (SNormalTextureVertex*)getData();

    ENDIAN( pVertex->swColor );
    ENDIAN( pVertex->swFlags );
    pVertex->Coord.endian();
    pVertex->Normal.endian();
    pVertex->Texture.endian();
    //    ENDIAN( pVertex->PackedColor );
    ENDIAN( pVertex->dwVertexColorIndex );
}


std::ostream& operator << (std::ostream& output, const NormalTextureVertexRecord& rec)
{
    output << rec.className() << " "
        << rec.getData()->swFlags << " "
        << rec.getData()->Coord;
    return output;               // to enable cascading
}
