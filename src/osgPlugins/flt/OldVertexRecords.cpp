// OldVertexRecords.cpp

#include "flt.h"
#include "Registry.h"
#include "OldVertexRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       OldVertexRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<OldVertexRecord> g_OldVertexProxy;

OldVertexRecord::OldVertexRecord()
{
}


// virtual
OldVertexRecord::~OldVertexRecord()
{
}


// virtual
void OldVertexRecord::endian()
{
    SOldVertex *pVertex = (SOldVertex*)getData();

    ENDIAN( pVertex->v[0] );
    ENDIAN( pVertex->v[1] );
    ENDIAN( pVertex->v[2] );
    if (getSize() >= sizeofData())
        pVertex->t.endian();
}

// virtual
bool OldVertexRecord::readLocalData(Input&)
{
    return true;
}

/*
ostream& operator << (ostream& output, const OldVertexRecord& rec)
{
    output << rec.className() << " "
           << rec.getData()->swFlags << " "
           << rec.getData()->Coord;
    return output;     // to enable cascading
}
*/

////////////////////////////////////////////////////////////////////
//
//                       OldVertexColorRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<OldVertexColorRecord> g_OldVertexColorProxy;

OldVertexColorRecord::OldVertexColorRecord()
{
}


// virtual
OldVertexColorRecord::~OldVertexColorRecord()
{
}


// virtual
void OldVertexColorRecord::endian()
{
    SOldVertexColor *pVertex = (SOldVertexColor*)getData();

    ENDIAN( pVertex->v[0] );
    ENDIAN( pVertex->v[1] );
    ENDIAN( pVertex->v[2] );
    ENDIAN( pVertex->color_index );
    if (getSize() >= sizeofData())
        pVertex->t.endian();
}


// virtual
bool OldVertexColorRecord::readLocalData(Input&)
{
    return true;
}


/*
ostream& operator << (ostream& output, const OldVertexColorRecord& rec)
{
    output << rec.className() << " "
           << rec.getData()->swFlags << " "
           << rec.getData()->Coord;
    return output;     // to enable cascading
}
*/

////////////////////////////////////////////////////////////////////
//
//                     OldVertexColorNormalRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<OldVertexColorNormalRecord> g_OldVertexColorNormalProxy;

OldVertexColorNormalRecord::OldVertexColorNormalRecord()
{
}


// virtual
OldVertexColorNormalRecord::~OldVertexColorNormalRecord()
{
}


// virtual
void OldVertexColorNormalRecord::endian()
{
    SOldVertexColorNormal *pVertex = (SOldVertexColorNormal*)getData();

    ENDIAN( pVertex->v[0] );
    ENDIAN( pVertex->v[1] );
    ENDIAN( pVertex->v[2] );
    ENDIAN( pVertex->color_index );
    ENDIAN( pVertex->n[0] );
    ENDIAN( pVertex->n[1] );
    ENDIAN( pVertex->n[2] );
    if (getSize() >= sizeofData())
        pVertex->t.endian();
}


// virtual
bool OldVertexColorNormalRecord::readLocalData(Input&)
{
    return true;
}


/*
ostream& operator << (ostream& output, const OldVertexColorNormalRecord& rec)
{
    output << rec.className() << " "
           << rec.getData()->swFlags << " "
           << rec.getData()->Coord;
    return output;     // to enable cascading
}
*/
