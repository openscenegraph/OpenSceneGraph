// ControlRecord.h

#ifndef __FLT_CONTROL_RECORD_H
#define __FLT_CONTROL_RECORD_H

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {

////////////////////////////////////////////////////////////////////
//
//                    PushLevelRecord
//
////////////////////////////////////////////////////////////////////


class PushLevelRecord : public ControlRecord
{
    public:
        PushLevelRecord() {}

        virtual Record* clone() const { return new PushLevelRecord(); }
        virtual const char* className() const { return "PushLevelRecord"; }
        virtual int classOpcode() const { return PUSH_LEVEL_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:
        virtual ~PushLevelRecord() {}

};


class PopLevelRecord : public ControlRecord
{
    public:
        PopLevelRecord() {}

        virtual Record* clone() const { return new PopLevelRecord(); }
        virtual const char* className() const { return "PopLevelRecord"; }
        virtual int classOpcode() const { return POP_LEVEL_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:
        virtual ~PopLevelRecord() {}

};


class PushSubfaceRecord : public ControlRecord
{
    public:
        PushSubfaceRecord() {}

        virtual Record* clone() const { return new PushSubfaceRecord(); }
        virtual const char* className() const { return "PushSubfaceRecord"; }
        virtual int classOpcode() const { return PUSH_SUBFACE_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:
        virtual ~PushSubfaceRecord() {}

};


class PopSubfaceRecord : public ControlRecord
{
    public:
        PopSubfaceRecord() {}

        virtual Record* clone() const { return new PopSubfaceRecord(); }
        virtual const char* className() const { return "PopSubfaceRecord"; }
        virtual int classOpcode() const { return POP_SUBFACE_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:
        virtual ~PopSubfaceRecord() {}

};


class PushExtensionRecord : public ControlRecord
{
    public:
        PushExtensionRecord() {}

        virtual Record* clone() const { return new PushExtensionRecord(); }
        virtual const char* className() const { return "PushExtensionRecord"; }
        virtual int classOpcode() const { return PUSH_EXTENSION_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:
        virtual ~PushExtensionRecord() {}

};


class PopExtensionRecord : public ControlRecord
{
    public:
        PopExtensionRecord() {}

        virtual Record* clone() const { return new PopExtensionRecord(); }
        virtual const char* className() const { return "PopExtensionRecord"; }
        virtual int classOpcode() const { return POP_EXTENSION_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:
        virtual ~PopExtensionRecord() {}

};


}; // end namespace flt

#endif // __FLT_CONTROL_RECORD_H

