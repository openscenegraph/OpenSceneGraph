// LightSourceRecord.h

#ifndef __FLT_LIGHT_SOURCE_RECORD_H
#define __FLT_LIGHT_SOURCE_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {



////////////////////////////////////////////////////////////////////
//
//                          LightSourceRecord
//
////////////////////////////////////////////////////////////////////


typedef struct LightSourceTag
{
    SRecHeader    RecHeader;
    char        szIdent[8];        // 7 char ASCII ID; 0 terminates
    int32        diReserved_1;
    int32        diIndex;        //index into lightpalette
    int32        diReserved_2;
    uint32        dwFlags;        //bits from left to right
                                //0=enabled
                                //1=global
                                //2=reserve
                                //3=export
                                //4=reserved
                                //5-31 spare
    int32        diReserved_3;
    float64x3   Coord;        // x,y,z coordinate
    float32        sfYaw;
    float32        sfPitch;
} SLightSource;


class LightSourceRecord : public PrimNodeRecord
{
    public:

        LightSourceRecord();

        virtual Record* clone() const { return new LightSourceRecord(); }
        virtual const char* className() const { return "LightSourceRecord"; }
        virtual int classOpcode() const { return LIGHT_SOURCE_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~LightSourceRecord();

        virtual void endian();
};



}; // end namespace flt

#endif

