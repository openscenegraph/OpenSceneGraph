//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <osg/MatrixTransform>
#include <osg/Texture2D>

#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

namespace flt {

/** Comment -
  */
class Comment : public Record
{
    public:

        Comment() {}

        META_Record(Comment)

    protected:

        virtual ~Comment() {}

        virtual void readRecord(RecordInputStream& in, Document& /*document*/)
        {
            std::streamsize size = in.getRecordSize();
            std::string comment = in.readString(size-4);

            if (_parent.valid())
                _parent->setComment(comment);
        }
};

RegisterRecordProxy<Comment>    g_Comment(COMMENT_OP);


/** LongID -
  */
class LongID : public Record
{
    public:

        LongID() {}

        META_Record(LongID)

    protected:

        virtual ~LongID() {}

        virtual void readRecord(RecordInputStream& in, Document& /*document*/)
        {
            std::streamsize size = in.getRecordSize();
            std::string id = in.readString(size-4);

            if (_parent.valid())
                _parent->setID(id);
        }
};

RegisterRecordProxy<LongID> g_LongID(LONG_ID_OP);


/** Matrix -
  */
class Matrix : public Record
{
    public:

        Matrix() {}

        META_Record(Matrix)

    protected:

        virtual ~Matrix() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            osg::Matrix matrix;
            for (int i=0; i<4; ++i)
            {
                for (int j=0; j<4; ++j)
                {
                    matrix(i,j) = in.readFloat32();
                }
            }

            // scale position.
            osg::Vec3 pos = matrix.getTrans();
            matrix *= osg::Matrix::translate(-pos);
            pos *= (float)document.unitScale();
            matrix *= osg::Matrix::translate(pos);

            if (_parent.valid())
                _parent->setMatrix(matrix);
        }
};

RegisterRecordProxy<Matrix> g_Matrix(MATRIX_OP);


/** Multitexture -
  */
class Multitexture : public Record
{
    public:

        Multitexture() {}

        META_Record(Multitexture)

    protected:

        virtual ~Multitexture() {}

        virtual void readRecord(RecordInputStream& in, Document& document)
        {
            osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

            uint32 mask = in.readUInt32();
            for (int layer=1; layer<8; layer++)
            {
                uint32 layerBit = 0x80000000u >> (layer-1);
                if (mask & layerBit)
                {
                    int16 textureIndex = in.readInt16();
                    /* int16 effectIndex =*/ in.readInt16();
                    /* int16 mappingIndex =*/ in.readInt16();
                    /* uint16 data=*/ in.readUInt16();

                    osg::ref_ptr<osg::StateSet> texturePoolStateset = document.getOrCreateTexturePool()->get(textureIndex);
                    if (stateset.valid() && texturePoolStateset.valid())
                    {
                        osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(texturePoolStateset->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
                        if (texture)
                            stateset->setTextureAttributeAndModes(layer,texture,osg::StateAttribute::ON);
                    }
                }
            }

            if (_parent.valid())
                _parent->setMultitexture(*stateset);
        }
};

RegisterRecordProxy<Multitexture> g_Multitexture(MULTITEXTURE_OP);


/** UVList - Texture coordinates used with multitexture.
    UVList is an ancillary to VertexList.
  */
class UVList : public Record
{
    public:

        UVList() {}

        META_Record(UVList)

    protected:

        virtual ~UVList() {}

        // count number of 1's in mask.
        int bitCount(uint32 mask)
        {
            int count = 0;
            while (mask)
            { 
                if (mask & 0x0001)
                    ++count; 
                mask >>= 1; 
            }
            return count;
        }

        virtual void readRecord(RecordInputStream& in, Document& /*document*/)
        {
            uint32 mask = in.readUInt32(0);

            int numLayers = bitCount(mask);
            int numVertices = (in.getRecordSize()-8) / (8 * numLayers);
            for (int n=0; n < numVertices; ++n)
            {
                for (unsigned int layer=1; layer<8; layer++)
                {
                    uint32 layerBit = 0x80000000u >> (layer-1);
                    if (mask & layerBit)
                    {
    			        float32	u = in.readFloat32();
    			        float32	v = in.readFloat32();

                        // Add texture coodinates to geometry.
                        if (_parent.valid())
                            _parent->addVertexUV(layer,osg::Vec2(u,v));
                    }
                }
            }
        }
};

RegisterRecordProxy<UVList> g_UVList(UV_LIST_OP);


/** Replicate -
  */
class Replicate : public Record
{
    public:

        Replicate() {}

        META_Record(Replicate)

    protected:

        virtual ~Replicate() {}

        virtual void readRecord(RecordInputStream& in, Document& /*document*/)
        {
            int16 replicate = in.readInt16();

            if (_parent.valid())
                _parent->setNumberOfReplications((int)replicate);
        }
};

RegisterRecordProxy<Replicate> g_Replicate(REPLICATE_OP);


// Prevent "unknown record" message for the following ancillary records:
RegisterRecordProxy<DummyRecord> g_OldTranslate(OLD_TRANSLATE2_OP);
RegisterRecordProxy<DummyRecord> g_OldRotateAboutPoint(OLD_ROTATE_ABOUT_POINT_OP);
RegisterRecordProxy<DummyRecord> g_OldRotateAboutEdge(OLD_ROTATE_ABOUT_EDGE_OP);
RegisterRecordProxy<DummyRecord> g_OldScale(OLD_SCALE_OP);
RegisterRecordProxy<DummyRecord> g_OldTranslate2(OLD_TRANSLATE_OP);
RegisterRecordProxy<DummyRecord> g_OldNonuniformScale(OLD_NONUNIFORM_SCALE_OP);
RegisterRecordProxy<DummyRecord> g_OldRotateAboutPoint2(OLD_ROTATE_ABOUT_POINT2_OP);
RegisterRecordProxy<DummyRecord> g_OldRotateScaleToPoint(OLD_ROTATE_SCALE_TO_POINT_OP);
RegisterRecordProxy<DummyRecord> g_OldPutTransform(OLD_PUT_TRANSFORM_OP);
RegisterRecordProxy<DummyRecord> g_OldBoundingBox(OLD_BOUNDING_BOX_OP);
RegisterRecordProxy<DummyRecord> g_IndexedString(INDEXED_STRING_OP);
RegisterRecordProxy<DummyRecord> g_RoadZone(ROAD_ZONE_OP);
RegisterRecordProxy<DummyRecord> g_RotateAboutEdge(ROTATE_ABOUT_EDGE_OP);
RegisterRecordProxy<DummyRecord> g_Translate(TRANSLATE_OP);
RegisterRecordProxy<DummyRecord> g_Scale(NONUNIFORM_SCALE_OP);
RegisterRecordProxy<DummyRecord> g_RotateAboutPoint(ROTATE_ABOUT_POINT_OP);
RegisterRecordProxy<DummyRecord> g_RotateScaleToPoint(ROTATE_SCALE_TO_POINT_OP);
RegisterRecordProxy<DummyRecord> g_PutTransform(PUT_TRANSFORM_OP);
RegisterRecordProxy<DummyRecord> g_GeneralMatrix(GENERAL_MATRIX_OP);
RegisterRecordProxy<DummyRecord> g_Vector(VECTOR_OP);
RegisterRecordProxy<DummyRecord> g_BoundingBox(BOUNDING_BOX_OP);
RegisterRecordProxy<DummyRecord> g_BoundingSphere(BOUNDING_SPHERE_OP);
RegisterRecordProxy<DummyRecord> g_BoundingCylinder(BOUNDING_CYLINDER_OP);
RegisterRecordProxy<DummyRecord> g_BoundingConvexHull(BOUNDING_CONVEX_HULL_OP);
RegisterRecordProxy<DummyRecord> g_BoundingHistogram(BOUNDING_HISTOGRAM);
RegisterRecordProxy<DummyRecord> g_BoundingVolumeCenter(BOUNDING_VOLUME_CENTER_OP);
RegisterRecordProxy<DummyRecord> g_BoundingVolumeOrientation(BOUNDING_VOLUME_ORIENTATION_OP);
RegisterRecordProxy<DummyRecord> g_HistogramBoundingVolume(HISTOGRAM_BOUNDING_VOLUME_OP);

} // end namespace

