#ifndef __FLT_INPUT_H
#define __FLT_INPUT_H

#include "Record.h"

#include <map>
#include <list>
#include <string>

#include "stdio.h"

namespace osg {
class Object;
class Image;
class Node;
};


namespace flt {


class Record;
class FltFile;


class Input
{
    public:

        Input() {}

        virtual SRecHeader* readRecord() = 0;
        virtual bool eof() = 0;
        virtual bool rewindLast() = 0;
        virtual long offset() = 0;

        Record* readCreateRecord(FltFile* pFltFile);

    protected:

        /** disallow creation of Objects on the stack.*/
        virtual ~Input() {}

    private:

        virtual bool _readHeader(SRecHeader* pHdr) = 0;
        virtual bool _readBody(SRecHeader* pData) = 0;
        virtual bool _readContinuedBody(char* pData, int nBytes) = 0;
};


/** Class for managing the reading of binary .flt files.*/
class FileInput : public Input
{
    public:

        FileInput();
        virtual ~FileInput();

        bool open(const std::string& fileName);
        void close();
        virtual bool eof();
        virtual bool rewindLast();
        virtual long offset();

        virtual SRecHeader* readRecord();

    private:
        virtual bool _readHeader(SRecHeader* pHdr);
        virtual bool _readBody(SRecHeader* pData);
        virtual bool _readContinuedBody(char* pData, int nBytes);
        void _init();
        size_t _read(void *buffer, size_t size);

        FILE*   _file;
        bool    _eof;
        long    _lRecOffset;
};



class MemInput : public Input
{
    public:

        MemInput();
        virtual ~MemInput();

        bool open(SRecHeader* pHdr);
        void close();
        virtual bool eof();
        virtual bool rewindLast();
        virtual long offset();

        virtual SRecHeader* readRecord();

    private:
        virtual bool _readHeader(SRecHeader* pHdr);
        virtual bool _readBody(SRecHeader* pData);
        virtual bool _readContinuedBody(char* pData, int nBytes);
        void _init();
        size_t _read(void *buffer, size_t size);

        bool    _eof;
        long    _lRecOffset;
};



}; // end namespace flt

#endif // __FLT_INPUT_H
