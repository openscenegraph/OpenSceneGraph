// ObjectRecord.h

#ifndef __FLT_OBJECT_RECORD_H
#define __FLT_OBJECT_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


struct SObject
{
    SRecHeader  RecHeader;
    char        szIdent[8];         // 7 char ASCII ID; 0 terminates
    uint32      dwFlags;            // Flags (bits from to right)
                                    //  0 = Don't display in daylight
                                    //  1 = Don't display at dusk
                                    //  2 = Don't display at night
                                    //  3 = Don't illuminate
                                    //  4 = Flat shaded
                                    //  5 = Group's shadow object
                                    //  6-31 Spare
    int16       iObjectRelPriority; // Object relative priority
    uint16      wTransparency;      // Transparency factor
                                    //  = 0 opaque
                                    //  = 65535 for totally clear
    int16       iSpecialId_1;       // Special effects ID 1 - defined by real time
    int16       iSpecialId_2;       // Special effects ID 2 - defined by real time
    int16       iSignificance;      // Significance
    int16       iSpare;             // Spare
};



class ObjectRecord : public PrimNodeRecord
{
    public:
        ObjectRecord();

        virtual Record* clone() const { return new ObjectRecord(); }
        virtual const char* className() const { return "ObjectRecord"; }
        virtual int classOpcode() const { return OBJECT_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SObject* getData() const { return (SObject*)_pData; }
        virtual const std::string getName( void ) const { return std::string(getData()->szIdent); }

    protected:
        virtual ~ObjectRecord();

        virtual void endian();

//      virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);

};


}; // end namespace flt

#endif
