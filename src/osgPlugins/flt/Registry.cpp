
#include "osg/Node"
#include "osg/Group"
#include "osg/Output"
#include <osg/Notify>

#include <algorithm>
#include <set>

#include "Record.h"
#include "Input.h"
#include "Registry.h"

#ifdef OSG_USE_IO_DOT_H
//#include <ostream.h>
#else
#include <iostream>
using namespace std;
#endif

#include <stdio.h>


using namespace flt;

// definition of the Registry
Registry::Registry()
{
    osg::notify(osg::INFO) << "Constructing flt record flt::Registry\n";
}


Registry::~Registry()
{
    
    osg::notify(osg::INFO) << "Destructing flt flt::Registry"<< endl;

    // note, do not need to unrefence records as the combination of
    // std::vector and ref_ptr will do it automatical on destruction.
}


Registry* Registry::instance()
{
    static Registry s_nodeFactory;
    return &s_nodeFactory;
}


void Registry::addPrototype(Record* rec)
{
    if (rec==0L) return;

    osg::notify(osg::INFO) << "flt::Registry::addPrototype("<< rec->className()<<")\n";

    RecordProtoList::iterator pitr = std::find(_recordProtoList.begin(),_recordProtoList.end(),rec);
    if (pitr==_recordProtoList.end())
    {
        _recordProtoList.push_back(rec);
    }
    else
    {
        osg::notify(osg::INFO) << "    failed - flt::Registry::addPrototype() - prototype already exists"<<")\n";
    }

    rec->ref();
}


void Registry::removePrototype(Record* rec)
{
    if (rec==0L) return;

    osg::notify(osg::INFO) << "flt::Registry::removePrototype("<<rec->className()<<")\n";

    RecordProtoList::iterator pitr = std::find(_recordProtoList.begin(),_recordProtoList.end(),rec);
    if (pitr!=_recordProtoList.end())
    {
        _recordProtoList.erase(pitr);
    }

}


Registry::RecordProtoList::iterator Registry::getRecordProtoItr(const int opcode)
{
    for(RecordProtoList::iterator itr=_recordProtoList.begin();
        itr!=_recordProtoList.end();
        ++itr)
    {
        if ((*itr)->classOpcode() == opcode)
            return itr;
    }

    return _recordProtoList.end();
}



Record* Registry::getRecordProto(const int opcode)
{
    RecordProtoList::iterator itr = getRecordProtoItr(opcode);
    if (itr==_recordProtoList.end())
        return NULL;

    return itr->get();
}

