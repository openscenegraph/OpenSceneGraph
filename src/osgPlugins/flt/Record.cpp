// Record.cpp

#include <algorithm>
#include <stdio.h>
#include <malloc.h>

#include <osg/Notify>

#include "export.h"
#include "flt.h"
#include "Registry.h"
#include "Record.h"
#include "FltRecords.h"
#include "UnknownRecord.h"
#include "opcodes.h"
#include "Input.h"
#include "RecordVisitor.h"

#ifdef OSG_USE_IO_DOT_H
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif


using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                          Record
//
////////////////////////////////////////////////////////////////////



Record::Record()
{
    _pData = NULL;
    _pParent = NULL;
}


Record::~Record()
{
    if (_pData) ::free(_pData);
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

/*
void Record::ascend(RecordVisitor& rv)
{
    std::for_each(_parents.begin(),_parents.end(),RecordAcceptOp(rv));
}
*/


ostream& operator << (ostream& output, const Record& rec)
{
	output << rec.className()
           << " op=" << rec.getOpcode()
           << " size=" << rec.getSize();
	return output; 	// to enable cascading
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
    removeAllChildren();
}


// virtual
void PrimNodeRecord::accept(RecordVisitor& rv)
{
    rv.apply(*this);
}


// virtual
void PrimNodeRecord::traverse(RecordVisitor& rv)
{
    for(ChildList::iterator itr=_children.begin();
                            itr!=_children.end();
                            ++itr)
    {
        (*itr)->accept(rv);
    }
}


void PrimNodeRecord::addChild( Record *child )
{
    if (child==NULL) return;

    ChildList::iterator itr = std::find(_children.begin(),_children.end(),child);
    if (itr==_children.end())
    {

        // add child to group.
        _children.push_back(child);

        // register as parent of child.
#if 0
//      child->_parents.push_back(this);
#else
        child->_pParent = this;
#endif
    }
}


void PrimNodeRecord::removeChild( Record *child )
{
    if (child==NULL) return;

    ChildList::iterator itr = std::find(_children.begin(),_children.end(),child);
    if (itr!=_children.end())
    {
        _children.erase(itr);

//      ParentList::iterator pitr = std::find(child->_parents.begin(),child->_parents.end(),child);
//      if (pitr!=child->_parents.end()) child->_parents.erase(pitr);
    }
}


void PrimNodeRecord::removeAllChildren()
{
    _children.erase(_children.begin(),_children.end());
}


bool PrimNodeRecord::readExtensions(Input& fr)
{
    Record* pRec;

    while ((pRec=readNextRecord(fr)))
    {
        if (pRec->isOfType(POP_EXTENSION_OP)) return true;
        // ignore extensions for now
    }
    return false;
}


bool PrimNodeRecord::readLevel(Input& fr)
{
    Record* pRec;

    while ((pRec=readNextRecord(fr)))
    {
        if (pRec->isOfType(POP_LEVEL_OP)) return true;
        if (!pRec->isPrimaryNode())
        {
            osg::notify(osg::WARN) << "Non primary record found as child. op=";
            osg::notify(osg::WARN) << pRec->getOpcode() << endl;
            return false;
        }
        addChild(pRec);

        if (!((PrimNodeRecord*)pRec)->readLocalData(fr))
            return false;
    }
    return false;
}


Record* PrimNodeRecord::readNextRecord(Input& fr)
{
    Record* pRec;
    
    while ((pRec=fr.readCreateRecord()))
    {
        switch (pRec->getOpcode())
        {
        case PUSH_EXTENSION_OP:
            readExtensions(fr);
            break;
        case PUSH_LEVEL_OP:
            readLevel(fr);
            break;
        default:
            return pRec;
        }
    }
    return pRec;
}


// virtual
bool PrimNodeRecord::readLocalData(Input& fr)
{
    Record* pRec;

    while ((pRec=readNextRecord(fr)))
    {
//      if (pRec->isOfType(PUSH_LEVEL_OP))
//          return true;
        if (!pRec->isAncillaryRecord())
            return fr.rewindLast();

        addChild(pRec);
    }
    return false;
}


/*
// virtual
bool PrimNodeRecord::readLocalData(Input& fr)
{
    Record* pRec;
    int until_op = 0;

    //
    // read ancillary records
    //

    while (pRec=fr.readCreateRecord())
    {
        // read extension records
        if (pRec->isOfType(PUSH_EXTENSION_OP))
        {
            while (pRec=fr.readCreateRecord())
            {
                if (pRec->isOfType(POP_EXTENSION_OP))
                {
                    pRec=fr.readCreateRecord();
                    break;
                }
            };
        }




        if (!pRec->isAncillaryRecord())
            break;

        addChild(pRec);
    };
//    if (pRec == NULL) return false;

    if (pRec == NULL) return false;


    if (pRec->getOpcode() != PUSH_LEVEL_OP)
    {
//      if (pRec->isPrimaryNode())
            return fr.rewindLast();

//      osg::notify(osg::INFO) << "Missing PUSH_LEVEL_OP" << endl;
//      return false;
    }

    //
    // read primary node records
    //

    while (pRec=fr.readCreateRecord())
    {
        if (pRec->getOpcode() == POP_LEVEL_OP) return true;

        if (pRec->isPrimaryNode())
        {
            addChild(pRec);
    
            if (!((PrimNodeRecord*)pRec)->readLocalData(fr))
                return false;
        }
    }

    if (pRec == NULL)
        return false;

    return true;
}
*/


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



