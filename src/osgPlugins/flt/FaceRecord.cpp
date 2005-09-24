// FaceRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "FaceRecord.h"
#include "Input.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          FaceRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<FaceRecord> g_FaceProxy;

FaceRecord::FaceRecord()
{
}


// virtual
FaceRecord::~FaceRecord()
{
}


int FaceRecord::numberOfVertices()
{
    for (int n=0; n < getNumChildren(); n++)
    {
        Record* child = getChild(n);
        if (!child) continue;

        if (child->isOfType(VERTEX_LIST_OP))
        {
            return ((VertexListRecord*)child)->numberOfVertices();
        }
        else if (child->isOfType(MORPH_VERTEX_LIST_OP))
        {
            return ((MorphVertexListRecord*)child)->numberOfVertices();
        }

    }

    return 0;
}


int FaceRecord::getVertexPoolOffset(int index)
{
    for (int n=0; n < getNumChildren(); n++)
    {
        Record* child = getChild(n);
        if (!child) continue;

        if (child->isOfType(VERTEX_LIST_OP))
        {
            return ((VertexListRecord*)child)->getVertexPoolOffset(index);
        }
        else if (child->isOfType(MORPH_VERTEX_LIST_OP))
        {
            return ((MorphVertexListRecord*)child)->getVertexPoolOffset(index);
        }
    }

    return 0;
}


void FaceRecord::endian()
{
    SFace *pSFace = (SFace*)getData();

    ENDIAN( pSFace->diIRColor );
    ENDIAN( pSFace->iObjectRelPriority );
    ENDIAN( pSFace->wPrimaryNameIndex );
    ENDIAN( pSFace->wSecondaryNameIndex );
    ENDIAN( pSFace->iDetailTexturePattern );
    ENDIAN( pSFace->iTexturePattern );
    ENDIAN( pSFace->iMaterial );
    ENDIAN( pSFace->iSurfaceMaterial );
    ENDIAN( pSFace->iFeature );
    ENDIAN( pSFace->diIRMaterial );
    ENDIAN( pSFace->wTransparency );

    // Face record extended after version 13
    if (getFltFile()->getFlightVersion() > 13)
    {
        ENDIAN( pSFace->dwFlags );
        //  ENDIAN( pSFace->PrimaryPackedColor );
        //  ENDIAN( pSFace->SecondaryPackedColor );
        ENDIAN( pSFace->iTextureMapIndex );
        ENDIAN( pSFace->dwPrimaryColorIndex );
        ENDIAN( pSFace->dwAlternateColorIndex );
    }
}


// virtual
bool FaceRecord::readLocalData(Input& fr)
{
    if (!PrimNodeRecord::readLocalData(fr))
        return false;

    //
    // Check for subfaces
    //

    Record* pRec;

    if (!(pRec=fr.readCreateRecord(_pFltFile))) return false;

    if (pRec->getOpcode() != PUSH_SUBFACE_OP)
        return fr.rewindLast();

    while ((pRec=fr.readCreateRecord(_pFltFile)))
    {
        if (pRec->getOpcode()==POP_SUBFACE_OP) return true;

        if (pRec->isPrimaryNode())
        {
            addChild(pRec);

            if (!((PrimNodeRecord*)pRec)->readLocalData(fr))
                return false;
        }
    }

    return (pRec != NULL);
}


////////////////////////////////////////////////////////////////////
//
//                       VertexListRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<VertexListRecord> g_VertexListProxy;

VertexListRecord::VertexListRecord()
{
}


// virtual
VertexListRecord::~VertexListRecord()
{
}


int VertexListRecord::numberOfVertices()
{
    return getBodyLength()/4;
}


int VertexListRecord::getVertexPoolOffset(int index)
{
    SSingleVertexList *pSVertexList = (SSingleVertexList*)getData();

    if ((index >= 0) && (index < numberOfVertices()))
        return pSVertexList->diSOffset[index];

    return 0;
}


void VertexListRecord::endian()
{
    SSingleVertexList *pSVertexList = (SSingleVertexList*)getData();
    int  nNumberOfVertices = numberOfVertices();

    for(int i=0; i < nNumberOfVertices; i++)
    {
        ENDIAN( pSVertexList->diSOffset[i] );
    }
}


////////////////////////////////////////////////////////////////////
//
//                       MorphVertexListRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<MorphVertexListRecord> g_MorphVertexListRecordProxy;

MorphVertexListRecord::MorphVertexListRecord()
{
}


// virtual
MorphVertexListRecord::~MorphVertexListRecord()
{
}


int MorphVertexListRecord::numberOfVertices()
{
    return getBodyLength()/8;
}


int MorphVertexListRecord::getVertexPoolOffset(int index)
{
    MorphVertexListTag *pSMorphVertexList = (MorphVertexListTag*)getData();

    if ((index >= 0) && (index < numberOfVertices()))
    {
        return pSMorphVertexList->list[index].dwOffset0;
    }

    return 0;
}


void MorphVertexListRecord::endian()
{
    SMorphVertexList *pSMorpVertexList = (SMorphVertexList*)getData();
    int  nNumberOfVertices = numberOfVertices();

    for(int i=0; i < nNumberOfVertices; i++)
    {
        ENDIAN( pSMorpVertexList->list[i].dwOffset0 );
        ENDIAN( pSMorpVertexList->list[i].dwOffset100 );
    }
}


////////////////////////////////////////////////////////////////////
//
//                       VectorRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<VectorRecord> g_VectorProxy;

VectorRecord::VectorRecord()
{
}


// virtual
VectorRecord::~VectorRecord()
{
}


void VectorRecord::endian()
{
    SVector *pSVector = (SVector*)getData();

    pSVector->Vec.endian();
}
