// BoundingVolumeRecords.h

#ifndef __FLT_BOUNDING_VOLUME_RECORDS_H
#define __FLT_BOUNDING_VOLUME_RECORDS_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"


namespace flt {


////////////////////////////////////////////////////////////////////
//
//                    BoundingBoxRecord
//
////////////////////////////////////////////////////////////////////


typedef struct BoundingBoxTag
{
    SRecHeader    RecHeader;
#if 0
Int 4 Reserved
Double 8 x coordinate of lowest corner
Double 8 y coordinate of lowest corner
Double 8 z coordinate of lowest corner
Double 8 x coordinate of highest corner
Double 8 y coordinate of highest corner
Double 8 z coordinate of highest corner
#endif
} SBoundingBox;



class BoundingBoxRecord : public AncillaryRecord
{
    public:

        BoundingBoxRecord();

        virtual Record* clone() const { return new BoundingBoxRecord(); }
        virtual const char* className() const { return "BoundingBoxRecord"; }
        virtual int classOpcode() const { return BOUNDING_BOX_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~BoundingBoxRecord();

        virtual void endian();
};



////////////////////////////////////////////////////////////////////
//
//                    BoundingSphereRecord
//
////////////////////////////////////////////////////////////////////


typedef struct BoundingSphereTag
{
    SRecHeader    RecHeader;
#if 0
Unsigned Int 2 Length of the record
Int 4 Reserved
Double 8 Radius of the sphere
#endif
} SBoundingSphere;



class BoundingSphereRecord : public AncillaryRecord
{
    public:

        BoundingSphereRecord();

        virtual Record* clone() const { return new BoundingSphereRecord(); }
        virtual const char* className() const { return "BoundingSphereRecord"; }
        virtual int classOpcode() const { return BOUNDING_SPHERE_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~BoundingSphereRecord();

        virtual void endian();
};



////////////////////////////////////////////////////////////////////
//
//                    BoundingCylinderRecord
//
////////////////////////////////////////////////////////////////////


typedef struct BoundingCylinderTag
{
    SRecHeader    RecHeader;
#if 0
Int 4 Reserved
Double 8 Radius of the cylinder base
Double 8 Height of the cylinder
#endif
} SBoundingCylinder;



class BoundingCylinderRecord : public AncillaryRecord
{
    public:

        BoundingCylinderRecord();

        virtual Record* clone() const { return new BoundingCylinderRecord(); }
        virtual const char* className() const { return "BoundingCylinderRecord"; }
        virtual int classOpcode() const { return BOUNDING_CYLINDER_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~BoundingCylinderRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                    BoundingVolumeCenterRecord
//
////////////////////////////////////////////////////////////////////


typedef struct BoundingVolumeCenterTag
{
    SRecHeader    RecHeader;
#if 0
Int 4 Reserved
Double 8 x coordinate of center
Double 8 y coordinate of center
Double 8 z coordinate of center
#endif
} SBoundingVolumeCenter;



class BoundingVolumeCenterRecord : public AncillaryRecord
{
    public:

        BoundingVolumeCenterRecord();

        virtual Record* clone() const { return new BoundingVolumeCenterRecord(); }
        virtual const char* className() const { return "BoundingVolumeCenterRecord"; }
        virtual int classOpcode() const { return BOUNDING_VOLUME_CENTER_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~BoundingVolumeCenterRecord();

        virtual void endian();
};


////////////////////////////////////////////////////////////////////
//
//                    BoundingVolumeOrientationRecord
//
////////////////////////////////////////////////////////////////////


typedef struct BoundingVolumeOrientationTag
{
    SRecHeader    RecHeader;
#if 0
Int 2 Bounding Volume Orientation Opcode 109
Unsigned Int 2 Length of the record
Int 4 Reserved
Double 8 Yaw angle
Double 8 Pitch angle
Double 8 Roll angle
#endif
} SBoundingVolumeOrientation;



class BoundingVolumeOrientationRecord : public AncillaryRecord
{
    public:

        BoundingVolumeOrientationRecord();

        virtual Record* clone() const { return new BoundingVolumeOrientationRecord(); }
        virtual const char* className() const { return "BoundingVolumeOrientationRecord"; }
        virtual int classOpcode() const { return BOUNDING_VOLUME_ORIENTATION_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

    protected:

        virtual ~BoundingVolumeOrientationRecord();

        virtual void endian();
};


}; // end namespace flt

#endif



