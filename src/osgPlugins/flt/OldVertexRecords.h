// OldVertexRecords.h

#ifndef __FLT_OLD_VERTEX_RECORDS_H
#define __FLT_OLD_VERTEX_RECORDS_H

#include <map>

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

#ifdef OSG_USE_IO_DOT_H
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif


namespace flt {


////////////////////////////////////////////////////////////////////
//
//                       OldVertexRecord
//
////////////////////////////////////////////////////////////////////

typedef struct OldVertexTag
{
	SRecHeader  RecHeader;
    int32       v[3];
} SOldVertex;


class OldVertexRecord : public PrimNodeRecord
{
    public:
        OldVertexRecord();
        virtual Record* clone() const { return new OldVertexRecord(); }
        virtual const char* className() const { return "OldVertexRecord"; }
        virtual int classOpcode() const { return OLD_VERTEX_OP; }
        virtual int sizeofData() const { return sizeof(SOldVertex); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SOldVertex* getData() const { return (SOldVertex*)_pData; }
//    	friend ostream& operator << (ostream& output, const OldVertexRecord& rec);

    protected:
        virtual ~OldVertexRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                       OldVertexColorRecord
//
////////////////////////////////////////////////////////////////////


typedef struct OldVertexColorTag
{
	SRecHeader	RecHeader;
    int32       v[3];
/*
    float64_t   x;
    float64_t   y;
    float64_t   z;
    uint16_t    color_name_index;
    uint16_t    flags;
    uint32_t    packed_color;
    uint32_t    color_index;
*/
} SOldVertexColor;


class OldVertexColorRecord : public PrimNodeRecord
{
    public:
        OldVertexColorRecord();
        virtual Record* clone() const { return new OldVertexColorRecord(); }
        virtual const char* className() const { return "OldVertexColorRecord"; }
        virtual int classOpcode() const { return OLD_VERTEX_COLOR_OP; }
        virtual int sizeofData() const { return sizeof(SOldVertexColor); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SOldVertexColor* getData() const { return (SOldVertexColor*)_pData; }
//    	friend ostream& operator << (ostream& output, const OldVertexColorRecord& rec);

    protected:
        virtual ~OldVertexColorRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                     OldVertexColorNormalRecord
//
////////////////////////////////////////////////////////////////////

typedef struct OldVertexColorNormalTag
{
	SRecHeader	RecHeader;
    int32       Coord[3];
	uint16	    swColor;			// Color Name Index
	uint16	    swFlags;			// Flags (bits, from left to right)
    int16       Normal[3];
} SOldVertexColorNormal;


class OldVertexColorNormalRecord : public PrimNodeRecord
{
    public:
        OldVertexColorNormalRecord();
        virtual Record* clone() const { return new OldVertexColorNormalRecord(); }
        virtual const char* className() const { return "OldVertexColorNormalRecord"; }
        virtual int classOpcode() const { return OLD_VERTEX_COLOR_NORMAL_OP; }
        virtual int sizeofData() const { return sizeof(SOldVertexColorNormal); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SOldVertexColorNormal* getData() const { return (SOldVertexColorNormal*)_pData; }
//    	friend ostream& operator << (ostream& output, const OldVertexColorNormalRecord& rec);

    protected:
        virtual ~OldVertexColorNormalRecord();

        virtual void endian();
};


}; // end namespace flt

#endif // __FLT_VERTEX_POOL_RECORDS_H
