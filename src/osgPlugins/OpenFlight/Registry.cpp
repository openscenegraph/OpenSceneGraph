//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <osg/Notify>
#include "Registry.h"

using namespace flt;

Registry::Registry()
{
}

Registry::~Registry()
{
}

Registry* Registry::instance()
{
    static Registry s_registry;
    return &s_registry;
}

void Registry::addPrototype(int opcode, Record* prototype)
{
    if (prototype==0L)
    {
        osg::notify(osg::WARN) << "Not a record." << std::endl;
        return;
    }

    if (_recordProtoMap.find(opcode) != _recordProtoMap.end())
        osg::notify(osg::WARN) << "Registry already contains prototype for opcode " << opcode << "." << std::endl;

    _recordProtoMap[opcode] = prototype;
}

Record* Registry::getPrototype(int opcode)
{
    RecordProtoMap::iterator itr = _recordProtoMap.find(opcode);
    if (itr != _recordProtoMap.end())
        return (*itr).second.get();

    return NULL;
}
