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

#ifndef FLT_RECORDINPUTSTREAM_H
#define FLT_RECORDINPUTSTREAM_H 1

#include "Record.h"
#include "DataInputStream.h"

namespace flt {

class Document;

typedef int opcode_type;
typedef std::streamsize size_type;

class RecordInputStream : public DataInputStream
{
    public:

        explicit RecordInputStream(std::streambuf* sb);

        bool readRecord(Document&);
        bool readRecordBody(opcode_type, size_type, Document&);

        inline std::streamsize getRecordSize() const { return _recordSize; }
        inline std::streamsize getRecordBodySize() const { return _recordSize-(std::streamsize)4; }

    protected:

        std::streamsize _recordSize;
};

} // end namespace

#endif
