// TransformationRecords.h

#ifndef __FLT_TRANSFORMATION_RECORDS_H
#define __FLT_TRANSFORMATION_RECORDS_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {



////////////////////////////////////////////////////////////////////
//
//                           MatrixRecord
//
////////////////////////////////////////////////////////////////////

typedef struct MatrixTag
{
    SRecHeader    RecHeader;
    float32        sfMat[4][4];    // 4x4 Single Precision Matrix
} SMatrix;                      // row major order


class MatrixRecord : public AncillaryRecord
{
    public:
        MatrixRecord();

        virtual Record* clone() const { return new MatrixRecord(); }
        virtual const char* className() const { return "MatrixRecord"; }
        virtual int classOpcode() const { return MATRIX_OP; }
        virtual size_t sizeofData() const { return sizeof(SMatrix); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SMatrix* getData() const { return (SMatrix*)_pData; }

    protected:
        virtual ~MatrixRecord();

        virtual void endian();
};

////////////////////////////////////////////////////////////////////
//
//                           TranslateRecord
//
////////////////////////////////////////////////////////////////////


typedef struct TranslateTag
{
    SRecHeader  RecHeader;
    int32       diReserved;
    float64x3   From;           // reference FROM point
    float64x3   Delta;          // Delta to translate node by
} STranslate;


class TranslateRecord : public AncillaryRecord
{
    public:
        TranslateRecord();

        virtual Record* clone() const { return new TranslateRecord(); }
        virtual const char* className() const { return "TranslateRecord"; }
        virtual int classOpcode() const { return TRANSLATE_OP; }
        virtual size_t sizeofData() const { return sizeof(STranslate); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual STranslate* getData() const { return (STranslate*)_pData; }

    protected:
        virtual ~TranslateRecord();

        virtual void endian();
};
#if 0
////////////////////////////////////////////////////////////////////
//
//                           RotatAboutEdgeRecord
//
////////////////////////////////////////////////////////////////////

typedef struct RotatAboutEdgeTag
{
    SRecHeader        RecHeader;
    int32           diReserved;
    float64x3   Point1;         // first point on edge
    float64x3   Point2;         // second point on edge
    float32         sfAngle;        // Angle by which to rotate
} SRotatAboutEdge;


class RotatAboutEdgeRecord : public AncillaryRecord
{
    public:
        RotatAboutEdgeRecord();

        virtual Record* clone() const { return new RotatAboutEdgeRecord(); }
        virtual const char* className() const { return "RotatAboutEdgeRecord"; }
        virtual int classOpcode() const { return ROTATE_ABOUT_EDGE_OP; }
        virtual size_t sizeofData() const { return sizeof(SRotatAboutEdge); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SRotatAboutEdge* getData() const { return (SRotatAboutEdge*)_pData; }

    protected:
        virtual ~RotatAboutEdgeRecord();

        virtual void endian();
};



////////////////////////////////////////////////////////////////////
//
//                           OldTranslateRecord
//
////////////////////////////////////////////////////////////////////


struct SOldTranslate
{
    SRecHeader  RecHeader;
    int32       diReserved;
    float64x3   From;           // reference FROM point
    float64x3   Delta;          // Delta to translate node by
};


class OldTranslateRecord : public AncillaryRecord
{
    public:
        OldTranslateRecord();

        virtual Record* clone() const { return new OldTranslateRecord(); }
        virtual const char* className() const { return "OldTranslateRecord"; }
        virtual int classOpcode() const { return OLD_TRANSLATE_OP; }
        virtual size_t sizeofData() const { return sizeof(SOldTranslate); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SOldTranslate* getData() const { return (SOldTranslate*)_pData; }

    protected:
        virtual ~OldTranslateRecord();

        virtual void endian();
};

////////////////////////////////////////////////////////////////////
//
//                           ScaleRecord
//
////////////////////////////////////////////////////////////////////

struct SScale
{
    SRecHeader        RecHeader;
    int32           Reserved;
    float64x3       center;
    float32x3       scale;
};

class ScaleRecord : public AncillaryRecord
{
    public:
        ScaleRecord();

        virtual Record* clone() const { return new ScaleRecord(); }
        virtual const char* className() const { return "ScaleRecord"; }
        virtual int classOpcode() const { return SCALE_OP; }
        virtual size_t sizeofData() const { return sizeof(SScale); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

//      virtual SGroup* getData() const { return (SGroup*)_pData; }

    protected:
        virtual ~ScaleRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                           RotatAboutPointRecord
//
////////////////////////////////////////////////////////////////////

typedef struct RotatAboutPointTag
{
    SRecHeader        RecHeader;
    // TODO
} SRotatAboutPoint;

class RotatAboutPointRecord : public AncillaryRecord
{
    public:
        RotatAboutPointRecord();

        virtual Record* clone() const { return new RotatAboutPointRecord(); }
        virtual const char* className() const { return "RotatAboutPointRecord"; }
        virtual int classOpcode() const { return ROTATE_ABOUT_POINT_OP; }
        virtual size_t sizeofData() const { return sizeof(SRotatAboutPoint); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

//      virtual SGroup* getData() const { return (SGroup*)_pData; }

    protected:
        virtual ~RotatAboutPointRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                           RotatScaleToPointRecord
//
////////////////////////////////////////////////////////////////////

typedef struct RotatScaleToPointTag
{
    SRecHeader        RecHeader;
    // TODO
} SRotatScaleToPoint;

class RotatScaleToPointRecord : public AncillaryRecord
{
    public:
        RotatScaleToPointRecord();

        virtual Record* clone() const { return new RotatScaleToPointRecord(); }
        virtual const char* className() const { return "RotatScaleToPointRecord"; }
        virtual int classOpcode() const { return ROTATE_SCALE_TO_POINT_OP; }
        virtual size_t sizeofData() const { return sizeof(SRotatScaleToPoint); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

//      virtual SGroup* getData() const { return (SGroup*)_pData; }

    protected:
        virtual ~RotatScaleToPointRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                           PutTransformRecord
//
////////////////////////////////////////////////////////////////////

typedef struct PutTransformTag    //follows normally a matrix record to
{                                //make up the transformation
    SRecHeader    RecHeader;
    int32        tmp1;            //mismatch with spec!
    float64x3    FromOrigin;
    float64x3    FromAlign;
    float64x3    FromTrack;
    float64x3    ToOrigin;        //mismatch !!
    float64x3    ToAlign;
    float64x3    ToTrack;
} SPutTransform;


class PutTransformRecord : public AncillaryRecord
{
    public:
        PutTransformRecord();

        virtual Record* clone() const { return new PutTransformRecord(); }
        virtual const char* className() const { return "PutTransformRecord"; }
        virtual int classOpcode() const { return PUT_TRANSFORM_OP; }
        virtual size_t sizeofData() const { return sizeof(SPutTransform); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        virtual SPutTransform* getData() const { return (SPutTransform*)_pData; }

    protected:
        virtual ~PutTransformRecord();

        virtual void endian();
};
#endif

////////////////////////////////////////////////////////////////////
//
//                           GeneralMatrixRecord
//
////////////////////////////////////////////////////////////////////

struct SGeneralMatrix
{
    SRecHeader  RecHeader;
    float32     sfMat[4][4];    // 4x4 Single Precision Matrix
};            // row major order


class GeneralMatrixRecord : public AncillaryRecord
{
    public:
        GeneralMatrixRecord();

        virtual Record* clone() const { return new GeneralMatrixRecord(); }
        virtual const char* className() const { return "GeneralMatrixRecord"; }
        virtual int classOpcode() const { return GENERAL_MATRIX_OP; }
        virtual size_t sizeofData() const { return sizeof(SGeneralMatrix); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

		virtual SGeneralMatrix* getData() const { return (SGeneralMatrix*)_pData; }
//      virtual SGroup* getData() const { return (SGroup*)_pData; }
        
    protected:
        virtual ~GeneralMatrixRecord();

        virtual void endian();
};


}; // end namespace flt

#endif // __FLT_TRANSFORMATION_RECORDS_H
