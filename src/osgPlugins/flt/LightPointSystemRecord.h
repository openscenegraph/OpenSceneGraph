
#ifndef __FLT_LIGHT_POINT_SYSTEM_RECORD_H
#define __FLT_LIGHT_POINT_SYSTEM_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct SLightPointSystem
{
    SRecHeader  RecHeader;
    char        ident[8];
    float32     intensity;      // Child light point node intensity
    int32       animationState; // Animation state: 0 -- On
                                //                  1 -- Off
                                //                  3 -- Random
    int32       flags;          // Flag bits: 0 -- Enable
                                //            1-31 -- Spare
};


class LightPointSystemRecord : public PrimNodeRecord
{
    public:

        LightPointSystemRecord();

        virtual Record* clone() const { return new LightPointSystemRecord(); }
        virtual const char* className() const { return "LightPointSystemRecord"; }
        virtual int classOpcode() const { return LIGHT_PT_SYSTEM_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }

        virtual SLightPointSystem* getData() const { return (SLightPointSystem*)_pData; }

    protected:

        virtual ~LightPointSystemRecord();

        virtual void endian();
};


}; // end namespace flt


#endif
