// OldVertexRecords.h

#ifndef __FLT_OLD_VERTEX_RECORDS_H
#define __FLT_OLD_VERTEX_RECORDS_H

#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

#include <map>
#include <iostream>


namespace flt {


////////////////////////////////////////////////////////////////////
//
//                       OldVertexRecord
//
////////////////////////////////////////////////////////////////////

struct SOldVertex
{
    SRecHeader  RecHeader;
    int32       v[3];
    float32x2   t;          // optional texture u,v
                            // check record size.
};


class OldVertexRecord : public PrimNodeRecord
{
    public:
        OldVertexRecord();
        virtual Record* clone() const { return new OldVertexRecord(); }
        virtual const char* className() const { return "OldVertexRecord"; }
        virtual int classOpcode() const { return OLD_VERTEX_OP; }
        virtual size_t sizeofData() const { return sizeof(SOldVertex); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SOldVertex* getData() const { return (SOldVertex*)_pData; }
//        friend std::ostream& operator << (std::ostream& output, const OldVertexRecord& rec);

    protected:
        virtual ~OldVertexRecord();

        virtual void endian();
        virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);
};


////////////////////////////////////////////////////////////////////
//
//                       OldVertexColorRecord
//
////////////////////////////////////////////////////////////////////


struct SOldVertexColor
{
    SRecHeader    RecHeader;
    int32       v[3];
    uint8       edge_flag;          // Hard edge flag
    uint8       shading_flag;       // Don’t touch normal when shading flag.
    uint16      color_index;        // Vertex color.
    float32x2   t;                  // optional texture u,v
                                    // check record size.
};


class OldVertexColorRecord : public PrimNodeRecord
{
    public:
        OldVertexColorRecord();
        virtual Record* clone() const { return new OldVertexColorRecord(); }
        virtual const char* className() const { return "OldVertexColorRecord"; }
        virtual int classOpcode() const { return OLD_VERTEX_COLOR_OP; }
        virtual size_t sizeofData() const { return sizeof(SOldVertexColor); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SOldVertexColor* getData() const { return (SOldVertexColor*)_pData; }
//        friend std::ostream& operator << (std::ostream& output, const OldVertexColorRecord& rec);

    protected:
        virtual ~OldVertexColorRecord();

        virtual void endian();
        virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);
};


////////////////////////////////////////////////////////////////////
//
//                     OldVertexColorNormalRecord
//
////////////////////////////////////////////////////////////////////

struct SOldVertexColorNormal
{
    SRecHeader    RecHeader;
    int32       v[3];
    uint8       edge_flag;          // Hard edge flag
    uint8       shading_flag;       // Don’t touch normal when shading flag.
    uint16      color_index;        // Vertex color.
    int32       n[3];               // Normal scaled  2**30
    float32x2   t;                  // optional texture u,v
                                    // check record size.
};


class OldVertexColorNormalRecord : public PrimNodeRecord
{
    public:
        OldVertexColorNormalRecord();
        virtual Record* clone() const { return new OldVertexColorNormalRecord(); }
        virtual const char* className() const { return "OldVertexColorNormalRecord"; }
        virtual int classOpcode() const { return OLD_VERTEX_COLOR_NORMAL_OP; }
        virtual size_t sizeofData() const { return sizeof(SOldVertexColorNormal); }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);
        virtual SOldVertexColorNormal* getData() const { return (SOldVertexColorNormal*)_pData; }
//        friend std::ostream& operator << (std::ostream& output, const OldVertexColorNormalRecord& rec);

    protected:
        virtual ~OldVertexColorNormalRecord();

        virtual void endian();
        virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);
};


}; // end namespace flt

#endif // __FLT_VERTEX_POOL_RECORDS_H
