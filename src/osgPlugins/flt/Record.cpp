// Record.cpp

#include <algorithm>
#include <stdio.h>
#include <string.h>

#include <osg/Notify>

#include "flt.h"
#include "Registry.h"
#include "Record.h"
#include "FltRecords.h"
#include "UnknownRecord.h"
#include "opcodes.h"
#include "Input.h"
#include "RecordVisitor.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                          Record
//
////////////////////////////////////////////////////////////////////

int Record::s_numAllocatedRecords=0;
        
Record::Record()
{
    _pData = NULL;
    _pParent = NULL;
    _pFltFile = NULL;
    
    ++s_numAllocatedRecords;
    
    flt::Registry::instance()->addRecordForFutureDelete(this);
}


Record::~Record()
{
    --s_numAllocatedRecords;

    if (_pData) ::free(_pData);
}


int Record::getFlightVersion() const
{
    if (_pFltFile)
        return _pFltFile->getFlightVersion();

    return -1;
}


Record* Record::cloneRecord(SRecHeader* pData)
{
    Record* pRec = clone();

    if (pRec->sizeofData() > pData->length())
        pData = (SRecHeader*)::realloc(pData, pRec->sizeofData());

    pRec->_pData = (SRecHeader*)pData;

    return pRec;
}


void Record::accept(RecordVisitor& rv)
{
    rv.apply(*this);
}


void Record::postReadInit()
{
  // These two lines used to be the last thing in Input::readCreateRecord().
  // Some records need a "post read" function called regardless of 
  // the endian of the machine, so I moved it to here.
  if ( flt::isLittleEndianMachine() ) // From Intel with love  :-(
      this->endian();
}


/*
void Record::ascend(RecordVisitor& rv)
{
    std::for_each(_parents.begin(),_parents.end(),RecordAcceptOp(rv));
}
*/

std::ostream& operator << (std::ostream& output, const Record& rec)
{
    output << rec.className()
        << " op=" << rec.getOpcode()
        << " size=" << rec.getSize();
    return output;               // to enable cascading
}


////////////////////////////////////////////////////////////////////
//
//                          PrimNodeRecord
//
////////////////////////////////////////////////////////////////////

PrimNodeRecord::PrimNodeRecord()
{
}


// virtual
PrimNodeRecord::~PrimNodeRecord()
{
//    removeAllChildren();
}


// virtual
void PrimNodeRecord::accept(RecordVisitor& rv)
{
    rv.apply(*this);
}


// virtual
void PrimNodeRecord::traverse(RecordVisitor& rv)
{
    for (ChildList::iterator itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        (*itr)->accept(rv);
    }
}


void PrimNodeRecord::addChild( Record *child )
{
    if (child==NULL) return;

#if ENSURE_CHILD_IS_UNIQUE    
    ChildList::iterator itr = std::find(_children.begin(),_children.end(),child);
    if (itr==_children.end())
    {
#endif    
        // note ref_ptr<> automatically handles incrementing child's reference count.
        _children.push_back(child);
        child->_pParent = this;
        
#if ENSURE_CHILD_IS_UNIQUE
    }
    else
    {
        std::cout<<"Trying to add non unique child, ignoring"<<std::endl;
    }
#endif
}


void PrimNodeRecord::removeChild( Record *child )
{
    if (child==NULL) return;

    ChildList::iterator itr = std::find(_children.begin(),_children.end(),child);
    if (itr!=_children.end())
    {
        // note ref_ptr<> automatically handles decrementing child's reference count.
        _children.erase(itr);
    }
}


void PrimNodeRecord::removeAllChildren()
{
    _children.clear();
}


bool PrimNodeRecord::readExtensionLevel(Input& fr)
{
    int extensionState = 1;
    Record* pRec;

    while ((pRec=fr.readCreateRecord(_pFltFile)))
    {
        if (pRec->isOfType(PUSH_EXTENSION_OP))
        {
            ++extensionState;
        }
        else if (pRec->isOfType(POP_EXTENSION_OP))
        {
            --extensionState;
            if ( !extensionState )  // PUSH'es and POP's have cancelled out
            {
                return true;
            }
        }
    }
    return false;
}


// Read from PUSH to POP
bool PrimNodeRecord::readLevel(Input& fr)
{
    Record* pRec;

    pRec = readRecord(fr);

    while (pRec && !pRec->isOfType(POP_LEVEL_OP))
    {
        if (!pRec->isPrimaryNode())
        {
            osg::notify(osg::WARN) << "Non primary record found as child. op="
                                   << pRec->getOpcode() << std::endl;
            return false;
        }

        addChild(pRec);

        if (!((PrimNodeRecord*)pRec)->readLocalData(fr))
            return false;

        pRec = readRecord(fr);
    }

    return true;
}

// Read next record, including extension record
Record* PrimNodeRecord::readRecord(Input& fr)
{
    Record* pRec = fr.readCreateRecord(_pFltFile);

    while (pRec && (pRec->getOpcode() == PUSH_EXTENSION_OP))
    {
        readExtensionLevel(fr);
        pRec=fr.readCreateRecord(_pFltFile);
    }
    return pRec;
}

#include "UVListRecord.h"
#include <stdio.h>

// virtual
bool PrimNodeRecord::readLocalData(Input& fr)
{
    Record* pRec;

    pRec = readRecord(fr);

    // Get ancillary records (if any)
    while (pRec && pRec->isAncillaryRecord())
    {
        addChild(pRec);
    if ( pRec->getOpcode() == 52 ) {
        CERR << "Multitexture added to " << this << "(opcode: "
        << getOpcode() << ")\n";
    }
    if ( pRec->getOpcode() == 53 ) {
        CERR << "UV list added to " << this << "(opcode: "
        << getOpcode() << ")\n";
        UVListRecord* mtr =
        dynamic_cast<UVListRecord*>(pRec);
        assert( mtr );
        assert( mtr->isAncillaryRecord() );
        SUVList* mt =
        reinterpret_cast<SUVList*>(mtr->getData());
        assert( mt );
    }
        pRec = readRecord(fr);
    }

    if (pRec == NULL)
        return false;

    // The next record should be PUSH or primary
    switch (pRec->getOpcode())
    {
        case PUSH_LEVEL_OP:
            readLevel(fr);
            break;

        default:
            fr.rewindLast();
    }

    return true;
}


////////////////////////////////////////////////////////////////////
//
//                          ControlRecord
//
////////////////////////////////////////////////////////////////////

// virtual
void ControlRecord::accept(RecordVisitor& rv)
{
    rv.apply(*this);
}


////////////////////////////////////////////////////////////////////
//
//                          AncillaryRecord
//
////////////////////////////////////////////////////////////////////

// virtual
void AncillaryRecord::accept(RecordVisitor& rv)
{
    rv.apply(*this);
}
