// DofRecord.h



#ifndef __FLT_DOF_RECORD_H
#define __FLT_DOF_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

struct SRange
{
    float64    _dfMin;            // Minimum value with respect to the local coord system
    float64    _dfMax;            // Maximum value with respect to the local coordsystem
    float64    _dfCurrent;        // Current value with respect to the local coord system
    float64    _dfIncrement;    // Increment

    inline float64 minRange()   { return _dfMin; }
    inline float64 maxRange()   { return _dfMax; }
    inline float64 current()    { return _dfCurrent; }
    inline float64 increment()  { return _dfIncrement; }
    void endian() {
        ENDIAN( _dfMin );
        ENDIAN( _dfMax );
        ENDIAN( _dfCurrent );
        ENDIAN( _dfIncrement );
    }
};


typedef struct DegreeOfFreedomTag
{
    SRecHeader    RecHeader;
    char    szIdent[8];                // 7 char ASCII ID; 0 terminates
    int32    diReserved;                // Reserved
    float64x3    OriginLocalDOF;    // Origin (x,y,z) of the DOF's local coordinate system
    float64x3    PointOnXaxis;    // Point (x,y,z) on the x-axis of the DOF's local coord system
    float64x3    PointInXYplane;    // Point (x,y,z) in xy plane of the DOF's local coord system
    SRange    dfZ;                    // Legal z values with respect to the local coord system
    SRange    dfY;                    // Legal y values with respect to the local coord system
    SRange    dfX;                    // Legal x values with respect to the local coord system
    SRange    dfPitch;                // Legal pitch values (rotation about the x-axis)
    SRange    dfRoll;                    // Legal roll values( rotation about the y-axis)
    SRange    dfYaw;                    // Legal yaw values (rotation about the z-axis)
    SRange    dfZscale;                // Legal z scale values (about local origin)
    SRange    dfYscale;                // Legal y scale values about local origin)
    SRange    dfXscale;                // Legal x scale values (about local origin)
    uint32    dwFlags;                // Flags, bits from left to right (see OF doc)
} SDegreeOfFreedom;



class DofRecord : public PrimNodeRecord
{
    public:
        DofRecord();

        virtual Record* clone() const { return new DofRecord(); }
        virtual const char* className() const { return "DofRecord"; }
        virtual int classOpcode() const { return DOF_OP; }
        virtual size_t sizeofData() const { return sizeof(SDegreeOfFreedom); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SDegreeOfFreedom* getData() const { return (SDegreeOfFreedom*)_pData; }
        virtual const std::string getName( void ) const { return std::string(getData()->szIdent); }

    protected:
        virtual ~DofRecord();

        virtual void endian();

//      virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);

};


}; // end namespace flt

#endif

