// RecordVisitor.h

#ifndef __FLT_RECORD_VISITOR_H
#define __FLT_RECORD_VISITOR_H

#include "Record.h"

namespace flt {

// Palette records
class ColorPaletteRecord;
class MaterialPaletteRecord;
class LightSourcePaletteRecord;
class TexturePaletteRecord;
class VertexPaletteRecord;
class VertexRecord;
class NormalVertexRecord;
class TextureVertexRecord;
class NormalTextureVertexRecord;

// Primary records
class HeaderRecord;
class GroupRecord;
class LodRecord;
class OldLodRecord;
class DofRecord;
class ObjectRecord;
class FaceRecord;
class MeshRecord;
class MeshPrimitiveRecord;
class VertexListRecord;
class MorphVertexListRecord;
class LightSourceRecord;
class LightPointRecord;
class LightPointIndexRecord;
class LightPointSystemRecord;
class SwitchRecord;
class ExtensionRecord;
class ExternalRecord;

// Ancillary records
class CommentRecord;
class LongIDRecord;
class VectorRecord;
class LocalVertexPoolRecord;
class OldVertexRecord;
class OldVertexColorRecord;
class OldVertexColorNormalRecord;

// Transformation records (ancillary)
class MatrixRecord;
class RotatAboutEdgeRecord;
class TranslateRecord;
class ScaleRecord;
class RotatAboutPointRecord;
class RotatScaleToPointRecord;
class PutTransformRecord;
class GeneralMatrixRecord;


/** Visitor for type safe operations on flt::Record's.
    Based on GOF's Visitor pattern.*/ 
class RecordVisitor
{
    public:

        enum TraversalMode {
            TRAVERSE_NONE,
//          TRAVERSE_PARENTS,
            TRAVERSE_ALL_CHILDREN,
            TRAVERSE_ACTIVE_CHILDREN,
            TRAVERSE_VISITOR
        };

        RecordVisitor(TraversalMode tm=TRAVERSE_NONE);
        virtual ~RecordVisitor();


        /** Set the traversal mode for Node::traverse() to use when 
            deciding which children of a node to traverse. If a
            NodeVisitor has been attached via setTraverseVisitor()
            and the new mode is not TRAVERSE_VISITOR then the attached
            visitor is detached. Default mode is TRAVERSE_NONE.*/
        void setTraverseMode(TraversalMode mode);
        /** Get the traversal mode.*/
        TraversalMode getTraverseMode() { return _traverseMode; }
        /** Set a visitor to handle traversal.
            Overides the traverse mode setting it to TRAVERSE_VISITOR.*/

        void setTraverseVisitor(RecordVisitor* rv);
        /** Get the traverse visitor, returns NULL if none is attached.*/
        RecordVisitor* getTraverseVisitor() { return _traverseVisitor; }

        /** Inline method for passing handling traversal of a nodes.
            If you intend to use the visitor for actively traversing 
            the scene graph then make sure the accept() methods call
            this method unless they handle traversal directly.*/
        void traverse(Record& rec)
        {
            if (_traverseVisitor) rec.accept(*_traverseVisitor);
//          else if (_traverseMode==TRAVERSE_PARENTS) rec.ascend(*this);
            else if (_traverseMode!=TRAVERSE_NONE) rec.traverse(*this);
        }

        virtual void apply(Record& rec)                     { traverse(rec);}
        virtual void apply(PrimNodeRecord& rec)             { apply((Record&)rec);}


        // Palette records
        virtual void apply(ColorPaletteRecord& rec)         { apply((Record&)rec); }
        virtual void apply(MaterialPaletteRecord& rec)      { apply((Record&)rec); }
        virtual void apply(LightSourcePaletteRecord& rec)   { apply((Record&)rec); }
        virtual void apply(TexturePaletteRecord& rec)       { apply((Record&)rec); }
        virtual void apply(VertexPaletteRecord& rec)        { apply((Record&)rec); }
        virtual void apply(VertexRecord& rec)               { apply((Record&)rec); }
        virtual void apply(NormalVertexRecord& rec)         { apply((Record&)rec); }
        virtual void apply(TextureVertexRecord& rec)        { apply((Record&)rec); }
        virtual void apply(NormalTextureVertexRecord& rec)  { apply((Record&)rec); }

        // Primary records
        virtual void apply(HeaderRecord& rec)               { apply((PrimNodeRecord&)rec); }
        virtual void apply(GroupRecord& rec)                { apply((PrimNodeRecord&)rec); }
        virtual void apply(LodRecord& rec)                  { apply((PrimNodeRecord&)rec); }
        virtual void apply(OldLodRecord& rec)               { apply((PrimNodeRecord&)rec); }
        virtual void apply(DofRecord& rec)                  { apply((PrimNodeRecord&)rec); }
        virtual void apply(ObjectRecord& rec)               { apply((PrimNodeRecord&)rec); }
        virtual void apply(FaceRecord& rec)                 { apply((PrimNodeRecord&)rec); }
        virtual void apply(MeshRecord& rec)                 { apply((PrimNodeRecord&)rec); }
        virtual void apply(MeshPrimitiveRecord& rec)        { apply((PrimNodeRecord&)rec); }
        virtual void apply(VertexListRecord& rec)           { apply((PrimNodeRecord&)rec); }
        virtual void apply(MorphVertexListRecord& rec)      { apply((PrimNodeRecord&)rec); }
        virtual void apply(LightSourceRecord& rec)          { apply((PrimNodeRecord&)rec); }
        virtual void apply(LightPointRecord& rec)           { apply((PrimNodeRecord&)rec); }
        virtual void apply(LightPointIndexRecord& rec)      { apply((PrimNodeRecord&)rec); }
        virtual void apply(LightPointSystemRecord& rec)     { apply((PrimNodeRecord&)rec); }
        virtual void apply(SwitchRecord& rec)               { apply((PrimNodeRecord&)rec); }
        virtual void apply(ExtensionRecord& rec)            { apply((PrimNodeRecord&)rec); }
        virtual void apply(ExternalRecord& rec)             { apply((PrimNodeRecord&)rec); }

        // Ancillary records
        virtual void apply(CommentRecord& rec)              { apply((Record&)rec); }
        virtual void apply(LongIDRecord& rec)               { apply((Record&)rec); }
        virtual void apply(VectorRecord& rec)               { apply((Record&)rec); }
        virtual void apply(LocalVertexPoolRecord& rec)      { apply((Record&)rec); }
        virtual void apply(OldVertexRecord& rec)            { apply((Record&)rec); }
        virtual void apply(OldVertexColorRecord& rec)       { apply((Record&)rec); }
        virtual void apply(OldVertexColorNormalRecord& rec) { apply((Record&)rec); }

        // Transformation records (ancillary)
        virtual void apply(MatrixRecord& rec)               { apply((Record&)rec); }
/*
        virtual void apply(RotatAboutEdgeRecord& rec)       { apply((Record&)rec); }
        virtual void apply(TranslateRecord& rec)            { apply((Record&)rec); }
        virtual void apply(ScaleRecord& rec)                { apply((Record&)rec); }
        virtual void apply(RotatAboutPointRecord& rec)      { apply((Record&)rec); }
        virtual void apply(RotatScaleToPointRecord& rec)    { apply((Record&)rec); }
        virtual void apply(PutTransformRecord& rec)         { apply((Record&)rec); }
*/
        virtual void apply(GeneralMatrixRecord& rec)        { apply((Record&)rec); }

    protected:

        RecordVisitor* _traverseVisitor;
        TraversalMode _traverseMode;

};


/** Convinience functor for assisting visiting of arrays of flt::Records's.*/ 
struct RecordAcceptOp
{
    RecordVisitor& _rv;
    RecordAcceptOp(RecordVisitor& rv):_rv(rv) {}
    void operator () (Record* rec) { rec->accept(_rv); }
};

};

#endif
