
#include <osg/Node>
#include <osg/Group>
//#include <osg/Output>
#include <osg/Notify>

#include <algorithm>
#include <set>

#include "Record.h"
#include "Input.h"
#include "FltFile.h"
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

///////////////////////////////////////////////////////////////////


void Registry::addTexture(osg::Texture* texture)
{
    if (texture==0L) return;

    osg::notify(osg::INFO) << "flt::Registry::addTexture("<< texture->className()<<")\n";

    TextureList::iterator pitr = std::find(_textureList.begin(),_textureList.end(),texture);
    if (pitr==_textureList.end())
        _textureList.push_back(texture);
    else
        osg::notify(osg::INFO) << "failed - flt::Registry::addTexture() - texture already exists"<<")\n";
}


void Registry::removeTexture(osg::Texture* texture)
{
    if (texture==0L) return;

    osg::notify(osg::INFO) << "flt::Registry::removeTexture("<< texture->className()<<")\n";

    TextureList::iterator itr = std::find(_textureList.begin(),_textureList.end(),texture);
    if (itr!=_textureList.end())
    {
        _textureList.erase(itr);
    }
}


Registry::TextureList::iterator Registry::getTextureItr(const std::string name)
{
    for(TextureList::iterator itr=_textureList.begin();
        itr!=_textureList.end();
        ++itr)
    {
        osg::Image* image = (*itr)->getImage();
        if (image && name == image->getFileName())
            return itr;
    }

    return _textureList.end();
}


osg::Texture* Registry::getTexture(const std::string name)
{
    TextureList::iterator itr = getTextureItr(name);
    if (itr==_textureList.end())
        return NULL;

    return *itr;
}


FltFile* Registry::getFltFile(const std::string& name)
{
    FltFileMap::iterator itr = _fltFileMap.find(name);
    if (itr != _fltFileMap.end())
        return (*itr).second.get();
    else
        return NULL;
}

void Registry::addFltFile(const std::string& name, FltFile* file)
{
    _fltFileMap[name] = file;
}

