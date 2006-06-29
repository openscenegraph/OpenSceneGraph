//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <iostream>
#include "opcodes.h"
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

using namespace flt;
using namespace std;


RecordInputStream::RecordInputStream(std::streambuf* sb):
    DataInputStream(sb),
    _recordSize(0),
    _recordOffset(0)
{}


std::istream& RecordInputStream::vread(char_type *str, std::streamsize count)
{
    if ((_recordSize>0) && (_recordOffset+count > _recordSize))
    {
        setstate(ios::failbit); // end-of-record (EOR)
        return *this;
    }

    _recordOffset += count;
    return DataInputStream::vread(str,count);
}


std::istream& RecordInputStream::vforward(std::istream::off_type off)
{
    if ((_recordSize>0) && (_recordOffset+off > _recordSize))
    {
        setstate(ios::failbit); // end-of-record (EOR)
        return *this;
    }

    _recordOffset += off;
    return DataInputStream::vforward(off);
}


bool RecordInputStream::readRecord(Document& document)
{
    // Get current read position in stream.
    _start = tellg();
    _recordOffset = 0;

    // Get record header without bounds check.
    _recordSize = 0;               // disable boundary check
    uint16 opcode = readUInt16();
    int size = (int)readUInt16();

    // Correct endian error in Creator v2.5 gallery models.
    // Last pop level record in little-endian.
    const uint16 LITTLE_ENDIAN_POP_LEVEL_OP = 0x0B00;
    if (opcode==LITTLE_ENDIAN_POP_LEVEL_OP)
    {
        osg::notify(osg::INFO) << "Little endian pop-level record" << std::endl;
        opcode=POP_LEVEL_OP;
        size=4;
    }

    _recordSize = size;

    // Update end-of-record
    _end = _start + (std::istream::pos_type)size;

#if 0
    // TODO: Peek at next opcode looking for continuation record.
    seekg(_end, std::ios_base::beg);
    if (_istream->fail())
        return false;

    int16 nextOpcode = readUInt16();
    seekg(_start+(std::istream::pos_type)4, std::ios_base::beg);
    if (nextOpcode == CONTINUATION_OP)
    {

    }
#endif

    // Get prototype record
    Record* prototype = Registry::instance()->getPrototype((int)opcode);

    if (prototype)
    {
#if 0 //def _DEBUG
        {
            for (int i=0; i<document.level(); i++)
                cout << "   ";
            cout << "opcode=" << opcode << " size=" << size;
            if (prototype) std::cout << " " << typeid(*prototype).name();
            cout << endl;
        }
#endif

        {
            // Create from prototype.
            osg::ref_ptr<Record> record = prototype->cloneType();

            // Read record
            record->read(*this,document);
        }

        // Clear failbit, it's used for end-of-record testing.
        clear(rdstate() & ~std::ios::failbit);
    }
    else // prototype not found
    {
        osg::notify(osg::WARN) << "Unknown record, opcode=" << opcode << " size=" << size << std::endl;

        // Add to registry so we only have to see this error message once.
        Registry::instance()->addPrototype(opcode,new DummyRecord);
    }

    // Move to beginning of next record
    seekg(_end, std::ios_base::beg);

    return good();
}
