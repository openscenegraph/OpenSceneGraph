// TransformationRecords.cpp

#include "flt.h"
#include "Registry.h"
#include "TransformationRecords.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                           MatrixRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<MatrixRecord> g_MatrixProxy;

MatrixRecord::MatrixRecord()
{
}


// virtual
MatrixRecord::~MatrixRecord()
{
}


// virtual
void MatrixRecord::endian()
{
    SMatrix* pSMatrix = (SMatrix*)getData();

    if (pSMatrix)
    {
        for(int i=0;i<4;++i)
        {
            for(int j=0;j<4;++j)
            {
                ENDIAN( pSMatrix->sfMat[i][j] );
            }
        }
    }
}

////////////////////////////////////////////////////////////////////
//
//                           TranslateRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<TranslateRecord> g_TranslateProxy;

TranslateRecord::TranslateRecord()
{
}


// virtual
TranslateRecord::~TranslateRecord()
{
}


// virtual
void TranslateRecord::endian()
{
   STranslate *pSTranslate = (STranslate*)getData() ;
   if ( pSTranslate)
   {
      pSTranslate->From.endian() ;
      pSTranslate->Delta.endian() ;
   }
}


#if 0
////////////////////////////////////////////////////////////////////
//
//                           RotatAboutEdgeRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<RotatAboutEdgeRecord> g_RotatAboutEdgeProxy;

RotatAboutEdgeRecord::RotatAboutEdgeRecord()
{
}


// virtual
RotatAboutEdgeRecord::~RotatAboutEdgeRecord()
{
}


// virtual
void RotatAboutEdgeRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                           ScaleRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<ScaleRecord> g_ScaleProxy;

ScaleRecord::ScaleRecord()
{
}


// virtual
ScaleRecord::~ScaleRecord()
{
}


// virtual
void ScaleRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                           RotatAboutPointRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<RotatAboutPointRecord> g_RotatAboutPointProxy;

RotatAboutPointRecord::RotatAboutPointRecord()
{
}


// virtual
RotatAboutPointRecord::~RotatAboutPointRecord()
{
}


// virtual
void RotatAboutPointRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                       RotatScaleToPointRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<RotatScaleToPointRecord> g_RotatScaleToPointProxy;

RotatScaleToPointRecord::RotatScaleToPointRecord()
{
}


// virtual
RotatScaleToPointRecord::~RotatScaleToPointRecord()
{
}


// virtual
void RotatScaleToPointRecord::endian()
{
}


////////////////////////////////////////////////////////////////////
//
//                         PutTransformRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<PutTransformRecord> g_PutTransformProxy;

PutTransformRecord::PutTransformRecord()
{
}


// virtual
PutTransformRecord::~PutTransformRecord()
{
}


// virtual
void PutTransformRecord::endian()
{
    SPutTransform *pSPutTransform = (SPutTransform*)getData();

    ENDIAN( pSPutTransform->tmp1 );
    pSPutTransform->FromOrigin.endian();
    pSPutTransform->FromAlign.endian();
    pSPutTransform->FromTrack.endian();
    pSPutTransform->ToOrigin.endian();
    pSPutTransform->ToAlign.endian();
    pSPutTransform->ToTrack.endian();
}
#endif

////////////////////////////////////////////////////////////////////
//
//                         GeneralMatrixRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<GeneralMatrixRecord> g_GeneralMatrixProxy;

GeneralMatrixRecord::GeneralMatrixRecord()
{
}


// virtual
GeneralMatrixRecord::~GeneralMatrixRecord()
{
}


// virtual
void GeneralMatrixRecord::endian()
{
    SGeneralMatrix* pSMatrix = (SGeneralMatrix*)getData();

    if (pSMatrix)
    {
        for(int i=0;i<4;++i)
        {
            for(int j=0;j<4;++j)
            {
                ENDIAN( pSMatrix->sfMat[i][j] );
            }
        }
    }
}
