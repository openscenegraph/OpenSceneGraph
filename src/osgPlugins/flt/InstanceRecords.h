// InstanceRecords.h


#ifndef __FLT_INSTANCE_RECORDS_H
#define __FLT_INSTANCE_RECORDS_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


////////////////////////////////////////////////////////////////////
//
//                          InstanceDefinitionRecord
//
////////////////////////////////////////////////////////////////////

typedef struct InstanceDefinitionTag
{
    SRecHeader    RecHeader;
    int16         iSpare;
    int16         iInstDefNumber;
}SInstanceDefinition;


class InstanceDefinitionRecord : public PrimNodeRecord
{
    public:
        InstanceDefinitionRecord();

        virtual Record* clone() const { return new InstanceDefinitionRecord(); }
        virtual const char* className() const { return "InstanceDefinitionRecord"; }
        virtual int classOpcode() const { return INSTANCE_DEFINITION_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SInstanceDefinition* getData() const { return (SInstanceDefinition*)_pData; }

    protected:
        virtual ~InstanceDefinitionRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                          InstanceReferenceRecord
//
////////////////////////////////////////////////////////////////////


typedef struct InstanceReferenceTag
{
    SRecHeader    RecHeader;
    int16         iSpare;
    int16         iInstDefNumber;
}SInstanceReference;



class InstanceReferenceRecord : public PrimNodeRecord
{
    public:
        InstanceReferenceRecord();

        virtual Record* clone() const { return new InstanceReferenceRecord(); }
        virtual const char* className() const { return "InstanceReferenceRecord"; }
        virtual int classOpcode() const { return INSTANCE_REFERENCE_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SInstanceReference* getData() const { return (SInstanceReference*)_pData; }

    protected:
        virtual ~InstanceReferenceRecord();

        virtual void endian();
};


}; // end namespace flt

#endif

