/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
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
        OSG_WARN << "Not a record." << std::endl;
        return;
    }

    if (_recordProtoMap.find(opcode) != _recordProtoMap.end())
        OSG_WARN << "Registry already contains prototype for opcode " << opcode << "." << std::endl;

    _recordProtoMap[opcode] = prototype;
}

Record* Registry::getPrototype(int opcode)
{
    RecordProtoMap::iterator itr = _recordProtoMap.find(opcode);
    if (itr != _recordProtoMap.end())
        return (*itr).second.get();

    return NULL;
}
