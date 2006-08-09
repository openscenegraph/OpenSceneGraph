//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include "Record.h"
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

namespace flt {

// Color from ColorPool.
osg::Vec4 getColorFromPool(int index, const ColorPool* colorPool)
{
    osg::Vec4 color(1,1,1,1);
    if (colorPool)
        color = colorPool->getColor(index);
    return color;
}


// Vertex flags
enum Flags
{
    START_HARD_EDGE = (0x8000 >> 0),
    NORMAL_FROZEN   = (0x8000 >> 1),
    NO_COLOR        = (0x8000 >> 2),
    PACKED_COLOR    = (0x8000 >> 3)
};


class VertexC : public Record
{
    public:

        VertexC() {}

        META_Record(VertexC)

        virtual ~VertexC() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            /*int colorNameIndex =*/ in.readInt16();
            uint16 flags = in.readUInt16();
            osg::Vec3d coord = in.readVec3d();
            osg::Vec4f packedColor = in.readColor32();
            int colorIndex = in.readInt32(-1);

            Vertex vertex;
            vertex.setCoord(coord*document.unitScale());

            // color
            if (flags & PACKED_COLOR)
                vertex.setColor(packedColor);                                             // Packed color
            else if (colorIndex >= 0)
                vertex.setColor(getColorFromPool(colorIndex, document.getColorPool()));   // Color from pool

            if (_parent.valid())
                _parent->addVertex(vertex);
        }
};

RegisterRecordProxy<VertexC> g_VertexC(VERTEX_C_OP);


class VertexCN : public Record
{
    public:

        VertexCN() {}

        META_Record(VertexCN)

    protected:

        virtual ~VertexCN() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            /*int colorNameIndex =*/ in.readInt16();
            uint16 flags = in.readUInt16();
            osg::Vec3d coord = in.readVec3d();
            osg::Vec3f normal = in.readVec3f();
            osg::Vec4f packedColor = in.readColor32();
            int colorIndex = in.readInt32(-1);

            Vertex vertex;
            vertex.setCoord(coord*document.unitScale());
            vertex.setNormal(normal);

            // color
            if (flags & PACKED_COLOR)
                vertex.setColor(packedColor);                                               // Packed color
            else if (colorIndex >= 0)
                vertex.setColor(getColorFromPool(colorIndex, document.getColorPool()));   // Color from pool

            if (_parent.valid())
                _parent->addVertex(vertex);
        }
};

RegisterRecordProxy<VertexCN> g_VertexCN(VERTEX_CN_OP);


class VertexCT : public Record
{
    public:

        VertexCT() {}

        META_Record(VertexCT)

    protected:

        virtual ~VertexCT() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            /*int colorNameIndex =*/ in.readInt16();
            uint16 flags = in.readUInt16();
            osg::Vec3d coord = in.readVec3d();
            osg::Vec2f uv = in.readVec2f();
            osg::Vec4f packedColor = in.readColor32();
            int colorIndex = in.readInt32(-1);

            Vertex vertex;
            vertex.setCoord(coord*document.unitScale());
            vertex.setUV(0,uv);

            // color
            if (flags & PACKED_COLOR)
                vertex.setColor(packedColor);                                               // Packed color
            else if (colorIndex >= 0)
                vertex.setColor(getColorFromPool(colorIndex, document.getColorPool()));   // Color from pool

            if (_parent.valid())
                _parent->addVertex(vertex);
        }
};

RegisterRecordProxy<VertexCT> g_VertexCT(VERTEX_CT_OP);


class VertexCNT : public Record
{
    public:

        VertexCNT() {}

        META_Record(VertexCNT)

    protected:

        virtual ~VertexCNT() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            /*int colorNameIndex =*/ in.readInt16();
            uint16 flags = in.readUInt16();
            osg::Vec3d coord = in.readVec3d();
            osg::Vec3f normal = in.readVec3f();
            osg::Vec2f uv = in.readVec2f();
            osg::Vec4f packedColor = in.readColor32();
            int colorIndex = in.readInt32(-1);

            Vertex vertex;
            vertex.setCoord(coord*document.unitScale());
            vertex.setNormal(normal);
            vertex.setUV(0,uv);
            

            if (!coord.valid())
            {
                osg::notify(osg::NOTICE)<<"Warning: data error detected in VertexCNT::readRecord coord="<<coord.x()<<" "<<coord.y()<<" "<<coord.z()<<std::endl;
            }

            if (!normal.valid())
            {
                osg::notify(osg::NOTICE)<<"Warning: data error detected in VertexCNT::readRecord normal="<<normal.x()<<" "<<normal.y()<<" "<<normal.z()<<std::endl;
            }

            if (!uv.valid())
            {
                osg::notify(osg::NOTICE)<<"Warning: data error detected in VertexCNT::readRecord uv="<<uv.x()<<" "<<uv.y()<<std::endl;
            }

            // color
            if (flags & PACKED_COLOR)
                vertex.setColor(packedColor);                                               // Packed color
            else if (colorIndex >= 0)
                vertex.setColor(getColorFromPool(colorIndex, document.getColorPool()));   // Color from pool

            if (_parent.valid())
                _parent->addVertex(vertex);
        }
};

RegisterRecordProxy<VertexCNT> g_VertexCNT(VERTEX_CNT_OP);


/** Absolut Vertex -
  * version < 13
  */
class AbsoluteVertex : public Record
{
    public:

        AbsoluteVertex() {}

        META_Record(AbsoluteVertex)

    protected:

        virtual ~AbsoluteVertex() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            int32 x = in.readInt32();
            int32 y = in.readInt32();
            int32 z = in.readInt32();

            Vertex vertex;

            // coord
            vertex.setCoord(osg::Vec3(x,y,z) * document.unitScale());

            // optional texture coordinates
            if (in.tellg() < in.getEndOfRecord())
            {
                osg::Vec2f uv = in.readVec2f();
                vertex.setUV(0,uv);
            }

            if (_parent.valid())
                _parent->addVertex(vertex);
        }
};

RegisterRecordProxy<AbsoluteVertex> g_AbsoluteVertex(OLD_ABSOLUTE_VERTEX_OP);


/** Shaded Vertex
  * version < 13
  */
class ShadedVertex : public Record
{
    public:

        ShadedVertex() {}

        META_Record(ShadedVertex)

    protected:

        virtual ~ShadedVertex() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            int32 x = in.readInt32();
            int32 y = in.readInt32();
            int32 z = in.readInt32();
            /*uint8 edgeFlag =*/ in.readUInt8();
            /*uint8 shadingFlag =*/ in.readUInt8();
            int colorIndex = (int)in.readInt16();

            Vertex vertex;

            // coord
            vertex.setCoord(osg::Vec3(x,y,z) * document.unitScale());

            // color
            if (colorIndex >= 0)
                vertex.setColor(getColorFromPool(colorIndex, document.getColorPool()));   // Color from pool

            // optional texture coordinates
            if (in.tellg() < in.getEndOfRecord())
            {
                osg::Vec2f uv = in.readVec2f();
                vertex.setUV(0,uv);
            }

            if (_parent.valid())
                _parent->addVertex(vertex);
        }
};

RegisterRecordProxy<ShadedVertex> g_ShadedVertex(OLD_SHADED_VERTEX_OP);


/** Normal Vertex
  * version < 13
  */
class NormalVertex : public Record
{
    public:

        NormalVertex() {}

        META_Record(NormalVertex)

    protected:

        virtual ~NormalVertex() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            int32 x = in.readInt32();
            int32 y = in.readInt32();
            int32 z = in.readInt32();
            /*uint8 edgeFlag =*/ in.readUInt8();
            /*uint8 shadingFlag =*/ in.readUInt8();
            int colorIndex = (int)in.readInt16();
            osg::Vec3f normal = in.readVec3d();

            Vertex vertex;
            vertex.setCoord(osg::Vec3(x,y,z) * document.unitScale());
            vertex.setNormal(normal / (float)(1L<<30));

            // color
            if (colorIndex >= 0)
                vertex.setColor(getColorFromPool(colorIndex, document.getColorPool()));   // Color from pool

            // optional texture coordinates
            if (in.tellg() < in.getEndOfRecord())
            {
                osg::Vec2f uv = in.readVec2f();
                vertex.setUV(0,uv);
            }

            if (_parent.valid())
                _parent->addVertex(vertex);
        }
};

RegisterRecordProxy<NormalVertex> g_NormalVertex(OLD_NORMAL_VERTEX_OP);

} // end namespace
