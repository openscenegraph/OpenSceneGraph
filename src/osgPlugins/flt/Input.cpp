// Input.cpp

#include <string>
// #include <malloc.h>
#include <assert.h>

#include <osg/Notify>
#include <osgDB/FileUtils>

#include "Input.h"
#include "Record.h"
#include "Registry.h"

using namespace flt;

FileInput::FileInput()
{
    _init();
}


FileInput::~FileInput()
{
    close();
}


void FileInput::_init()
{
    _lRecOffset = 0L;
    _file = NULL;
    _eof = true;
}


size_t FileInput::_read(void *buffer, size_t size)
{
    if (_eof) return 0;

    size_t nItemsRead = ::fread(buffer, size, 1, _file);
    if (nItemsRead != 1)
        _eof = true;

    return nItemsRead;
}


bool FileInput::eof()
{
    return _eof;
}


bool FileInput::open(const std::string& fileName)
{
    _file=::fopen( fileName.c_str(), "rb");
    if (_file == NULL) return false;
    _eof = false;
    return true;
}


void FileInput::close()
{
    if (_file) ::fclose(_file);
    _init();
}


bool FileInput::rewindLast()
{
    if (_file == NULL) return false;
    return (fseek(_file, _lRecOffset, SEEK_SET) == 0);
}


long FileInput::offset()
{
    return _lRecOffset;
}


// read opcode and size

bool FileInput::_readHeader(SRecHeader* pHdr)
{
    int nItemsRead;

                                 // Save file position for rewind operation
    _lRecOffset = ::ftell( _file );

    // Read record header (4 bytes)
    nItemsRead = _read(pHdr, sizeof(SRecHeader));
    if (nItemsRead != 1)
        return false;

    if (isLittleEndianMachine())
        pHdr->endian();

    if ((unsigned)pHdr->length() < sizeof(SRecHeader))
        return false;

    return true;
}


bool FileInput::_readBody(SRecHeader* pData)
{
    // Read record body
    int nBodySize = pData->length() - sizeof(SRecHeader);
    if (nBodySize > 0)
    {
        int nItemsRead = _read(pData+1, nBodySize);
        if (nItemsRead != 1)
            return false;
    }

    return true;
}


bool FileInput::_readContinuedBody(char* pData, int nBytes)
{
    // Read record body
    if (nBytes > 0)
    {
        int nItemsRead = _read(pData, nBytes);
        if (nItemsRead != 1)
            return false;
    }

    return true;
}


SRecHeader* FileInput::readRecord()
{
    SRecHeader hdr;
    SRecHeader* pData;

    if (!_readHeader(&hdr))
        return NULL;

    // Allocate buffer for record (including header)
    // This buffer is extended later in Record::cloneRecord()
    // if defined struct is bigger than read.
    pData = (SRecHeader*)::malloc(hdr.length());
    if (pData == NULL)
        return NULL;

    *pData = hdr;

    // Some records contains only the header
    if (hdr.length() == sizeof(SRecHeader))
        return pData;

    if (!_readBody(pData))
        return NULL;


    // Add support for OpenFlight 15.7 (1570) continuation records
    //

    // Record and FaceRecord both want to rewindLast, so save and restore
    //   the current file offset.
    const long lRecOffsetSave = _lRecOffset;

    int nTotalLen = hdr.length();
    // From spec, in practice only these three records can be continued:
    bool bContinuationPossible = (
        (hdr.opcode()==COLOR_NAME_PALETTE_OP) ||
        (hdr.opcode()==EXTENSION_OP) ||
        (hdr.opcode()==LOCAL_VERTEX_POOL_OP) );

    while (bContinuationPossible)
    {
        SRecHeader hdr2;
        if (_readHeader( &hdr2 ))
        {
            if (hdr2.opcode() == CONTINUATION_OP)
            {
                int nNewChunkLen = hdr2.length() - 4;
                size_t nNewLen = nTotalLen + nNewChunkLen;
                pData = (SRecHeader*)::realloc( (void*)pData, nNewLen );
                if (pData == NULL)
                    return NULL;

                if (!_readContinuedBody( ((char*)pData) + nTotalLen, nNewChunkLen ))
                    return NULL;
                nTotalLen = (int)nNewLen;
            }
            else
            {
                // Not a continuation record. Rewind, then exit loop.
                rewindLast();
                bContinuationPossible = false;
            }
        }
        else
            // Probably EOF
            bContinuationPossible = false;
    }

    _lRecOffset = lRecOffsetSave;

    //
    // END support for continuation records


    return pData;
}


Record* Input::readCreateRecord(FltFile* pFltFile)
{
    SRecHeader* pData = readRecord();

    if (pData == NULL) return NULL;

    // find matching record prototype class
    Record* pProto = Registry::instance()->getPrototype(pData->opcode());
    if (pProto == NULL)
        pProto = Registry::instance()->getPrototype(0);

    if (pProto == NULL)
    {
        // Should not be possible to end up here!
        osg::notify(osg::INFO) << "UnknownRecord not in registry!" << std::endl;
        ::free(pData);
        return NULL;
    }

    // clone protoype
    Record* pRec = pProto->cloneRecord(pData);
    if (pRec == NULL)
    {
        osg::notify(osg::INFO) << "Can't clone record!" << std::endl;
        ::free(pData);
        return NULL;
    }

    pRec->_pFltFile = pFltFile;

    #if 0
    osg::notify(osg::ALWAYS) << "class   = " << pRec->className();
    osg::notify(osg::ALWAYS) << " op     = " << pRec->getOpcode();
    osg::notify(osg::ALWAYS) << " name   = " << pRec->getName();
    osg::notify(osg::ALWAYS) << " offset = " << offset() << std::endl;
    #endif

    // Perform any post-read initializations.
    pRec->postReadInit();

    return pRec;
}
