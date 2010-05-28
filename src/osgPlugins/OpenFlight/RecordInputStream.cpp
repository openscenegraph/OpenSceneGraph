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

#include <iostream>
#include "Opcodes.h"
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

using namespace flt;
using namespace std;


RecordInputStream::RecordInputStream(std::streambuf* sb):
    DataInputStream(sb),
    _recordSize(0)
{}

bool RecordInputStream::readRecord(Document& document)
{
    opcode_type opcode = (opcode_type)readUInt16();
    size_type   size   = (size_type)readUInt16();

    return readRecordBody(opcode, size, document);
}

bool RecordInputStream::readRecordBody(opcode_type opcode, size_type size, Document& document)
{
    // Correct endian error in Creator v2.5 gallery models.
    // Last pop level record in little-endian.
    const uint16 LITTLE_ENDIAN_POP_LEVEL_OP = 0x0B00;
    if (opcode==LITTLE_ENDIAN_POP_LEVEL_OP)
    {
        OSG_INFO << "Little endian pop-level record" << std::endl;
        opcode=POP_LEVEL_OP;
        size=4;
    }

    _recordSize = size;

    // Get prototype record
    Record* prototype = Registry::instance()->getPrototype((int)opcode);

    if (prototype)
    {
#if 0 // for debugging
        {
            for (int i=0; i<document.level(); i++)
                cout << "   ";
            cout << "opcode=" << opcode << " size=" << size;
            if (prototype) std::cout << " " << typeid(*prototype).name();
            cout << endl;
        }
#endif
        // Create from prototype.
        osg::ref_ptr<Record> record = prototype->cloneType();

        // Read record
        record->read(*this,document);
    }
    else // prototype not found
    {
        OSG_WARN << "Unknown record, opcode=" << opcode << " size=" << size << std::endl;

        // Add to registry so we only have to see this error message once.
        Registry::instance()->addPrototype(opcode,new DummyRecord);
    }

    return good();
}
