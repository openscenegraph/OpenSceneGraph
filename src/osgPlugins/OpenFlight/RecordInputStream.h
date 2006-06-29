//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_RECORDINPUTSTREAM_H
#define FLT_RECORDINPUTSTREAM_H 1

#include "Record.h"
#include "DataInputStream.h"

namespace flt {

class Document;

class RecordInputStream : public DataInputStream
{
    public:

        explicit RecordInputStream(std::streambuf* sb);

        bool readRecord(Document& data);

        inline std::istream::pos_type getStartOfRecord() const { return _start; }
        inline std::istream::pos_type getEndOfRecord() const { return _end; }
        inline std::streamsize getRecordSize() const { return _end-_start; }
        inline std::streamsize getRecordBodySize() const { return getRecordSize()-(std::streamsize)4; }

        inline void moveToStartOfRecord() { seekg(_start /*,std::ios_base::beg*/); }
        inline void setEndOfRecord(std::istream::pos_type pos) { _end=pos; }

    protected:

        virtual std::istream& vread(char_type *str, std::streamsize count);
        virtual std::istream& vforward(std::istream::off_type off);

        int _recordSize;
        int _recordOffset;
        std::istream::pos_type _start;      // start of record
        std::istream::pos_type _end;        // end of record
};

} // end namespace

#endif
