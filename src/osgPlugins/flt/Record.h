#ifndef __FLT_RECORD_H
#define __FLT_RECORD_H

#include <string>
#include <vector>

#include <osg/Referenced>
#include <osg/ref_ptr>

#include "FltRecords.h"

#include <iostream>


namespace flt {

//class Record;
class Input;
class FltFile;
class RecordVisitor;
class FltFile;
class PrimNodeRecord;
class FaceRecord;


////////////////////////////////////////////////////////////////////
//
//                          Record
//
////////////////////////////////////////////////////////////////////

class Record : public osg::Referenced
{
    public:

        // used for debugging the number of records created and deleted.
        static int s_numAllocatedRecords;

        Record();

        virtual Record* clone() const = 0;
        Record* cloneRecord(SRecHeader* pData);

        virtual const char* className() const { return "Record"; } //const = 0;
        virtual int classOpcode() const { return 0; } //const = 0;
        virtual size_t sizeofData() const { return 0; }  //const = 0;
        virtual void accept(RecordVisitor& rv);
        virtual void traverse(RecordVisitor&) {}

        virtual const std::string getName( void ) const { return std::string("?"); }

        virtual bool isPrimaryNode() const { return false; }
        virtual bool isControlRecord() const { return false; }
        virtual bool isAncillaryRecord() const { return false; }
        virtual void endian() {}

        int getRecordType() const;

        SRecHeader*     getData() const;
        void*           getBody() const;
        int             getOpcode() const;
        size_t          getSize() const;
        size_t          getHeaderLength() const;
        size_t          getBodyLength() const;
        bool            isOfType(int op) const;
        Record*         getParent() const { return _pParent; }
        FltFile*        getFltFile()  { return _pFltFile; }
        int             getFlightVersion() const;

        virtual void postReadInit();

        friend std::ostream& operator << (std::ostream& output, const Record& rec);

    protected:

        /** disallow creation of Records on the stack.*/
        virtual ~Record();

        /** Template Method local read and write methods */
        virtual bool readLocalData(Input& /*fr*/) { return false; }
//      virtual bool writeLocalData(Output& fw) { return false; }

        SRecHeader*     _pData;
        Record*         _pParent;
        FltFile*        _pFltFile;

        friend class Input;
        friend class FltFile;
        friend class PrimNodeRecord;

    private:

        /** disallow copy */
        Record& operator = (const Record&) { return *this;} 
        Record(const Record&) : osg::Referenced() {}

};



inline
SRecHeader* Record::getData() const
{
    return _pData;
}


inline
void* Record::getBody() const
{
    return (_pData) ? _pData+1 : NULL;
}


inline
int Record::getOpcode() const
{
    return (_pData) ? _pData->opcode() : 0;
}

inline
size_t Record::getSize() const
{
    return (_pData) ? _pData->length() : 0;
}

inline
size_t Record::getHeaderLength() const
{
    return sizeof(SRecHeader);
}

inline
size_t Record::getBodyLength() const
{
    return getSize() - getHeaderLength();
}

inline
bool Record::isOfType(int op) const
{
    return (op == getOpcode());
}



////////////////////////////////////////////////////////////////////
//
//                          PrimNodeRecord
//
////////////////////////////////////////////////////////////////////


class PrimNodeRecord : public Record
{
    public:
        PrimNodeRecord();
        virtual bool isPrimaryNode() const { return true; }
        virtual void accept(RecordVisitor& rv);
        virtual void traverse(RecordVisitor& rv);

        void addChild( Record* child );
        void removeChild( Record* child );
        void removeAllChildren();
        int  getNumChildren( void )       { return _children.size(); }
        Record* getChild( int i )     { return _children[i].get(); }

    protected:

        /** disallow creation of PrimNodeRecords on the stack.*/
        virtual ~PrimNodeRecord();

        virtual bool readLocalData(Input& fr);

    private:
        bool readExtensionLevel(Input& fr);
        bool readLevel(Input& fr);
        Record* readRecord(Input& fr);

        typedef std::vector<osg::ref_ptr<Record> > ChildList;
        ChildList _children;

        friend class FaceRecord;
};


////////////////////////////////////////////////////////////////////
//
//                          ControlRecord
//
////////////////////////////////////////////////////////////////////

class ControlRecord : public Record
{
    public:

//      ControlRecord();
        virtual bool isControlRecord() const { return true; }
        virtual void accept(RecordVisitor& rv);
//      virtual void traverse(RecordVisitor& rv);

        
    protected:

        /** disallow creation of ControlRecord on the stack.*/
//      virtual ~ControlRecord();

        virtual bool readLocalData(Input& /*fr*/) { return true; }
};


////////////////////////////////////////////////////////////////////
//
//                          AncillaryRecord
//
////////////////////////////////////////////////////////////////////

class AncillaryRecord : public Record
{
    public:

//      AncillaryRecord();
        virtual bool isAncillaryRecord() const { return true; }
        virtual void accept(RecordVisitor& rv);
//      virtual void traverse(RecordVisitor& rv);

    protected:

        /** disallow creation of AncillaryRecord on the stack.*/
//      virtual ~AncillaryRecord();

        virtual bool readLocalData(Input& /*fr*/) { return true; }
};

////////////////////////////////////////////////////////////////////



}; // end namespace flt

#endif // __FLT_RECORD_H



