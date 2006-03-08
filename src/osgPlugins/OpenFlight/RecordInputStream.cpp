//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include "opcodes.h"
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

using namespace flt;
using namespace std;


RecordInputStream::RecordInputStream(std::istream* istream):
    DataInputStream(istream)
{

#if 0 //def _DEBUG
    ios::iostate mask = istream->exceptions();
    cout << "ios::badbit=" << ( mask & ios::badbit ) << std::endl;
    cout << "ios::failbit=" << ( mask & ios::failbit ) << std::endl;
    cout << "ios::eofbit=" << ( mask & ios::eofbit ) << std::endl;
#endif

    // dont't throw an exception on failbit
    istream->exceptions(istream->exceptions() & ~ios::failbit);
}


RecordInputStream::~RecordInputStream()
{
}


std::istream& RecordInputStream::read(std::istream::char_type *_Str, std::streamsize _Count) const
{
    // Bounds check
    istream::pos_type pos = _istream->tellg();
    if (pos+(istream::pos_type)_Count > _end)
    {
        _istream->setstate(ios::failbit); // end-of-record (EOR)
        return *_istream;
    }

    return _istream->read(_Str,_Count);
}


bool RecordInputStream::readRecord(Document& document)
{
    // Get current read position in stream.
    _start = _istream->tellg();

    // Get record header without bounds check.
    DataInputStream distream(_istream);
    uint16 opcode = distream.readUInt16();
    uint16 size = distream.readUInt16();

    // Correct endian error in Creator v2.5 gallery models.
    // Last pop level record in little-endian.
    const uint16 LITTLE_ENDIAN_POP_LEVEL_OP = 0x0B00;
    if (opcode==LITTLE_ENDIAN_POP_LEVEL_OP)
    {
        osg::notify(osg::INFO) << "Little endian pop-level record" << std::endl;
        opcode=POP_LEVEL_OP;
        size=4;
    }

    // Update end-of-record
    _end = _start + (std::istream::pos_type)size;

#if 0
    // TODO: Peek at next opcode looking for continuation record.
    _istream->seekg(_end, std::ios_base::beg);
    if (_istream->fail())
        return false;

    int16 nextOpcode = readUInt16();
    _istream->seekg(_start+(std::istream::pos_type)4, std::ios_base::beg);
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
        _istream->clear(_istream->rdstate() & ~std::ios::failbit);
    }
    else // prototype not found
    {
        osg::notify(osg::WARN) << "Unknown record, opcode=" << opcode << " size=" << size << std::endl;

        // Add to registry so we only have to see this error message once.
        Registry::instance()->addPrototype(opcode,new DummyRecord);
    }

    // Move to beginning of next record
    _istream->seekg(_end, std::ios_base::beg);

    return _istream->good();
}
