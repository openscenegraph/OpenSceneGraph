/**********************************************************************
 *
 *    FILE:           AutoTransform.cpp
 *
 *    DESCRIPTION:    Read/Write osg::AutoTransform in binary format to disk.
 *
 *    CREATED BY:     Nathan Monteleone, based on PositionAttitudeTransform.cpp
 *
 *    HISTORY:        Created 02.3.2006
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "AutoTransform.h"
#include "Transform.h"

using namespace ive;

void AutoTransform::write(DataOutputStream* out){
    // Write AutoTransform's identification.
    out->writeInt(IVEAUTOTRANSFORM);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Transform*  trans = dynamic_cast<osg::Transform*>(this);
    if(trans){
        ((ive::Transform*)(trans))->write(out);
    }
    else
        throw Exception("AutoTransform::write(): Could not cast this osg::AutoTransform to an osg::Transform.");
    // Write AutoTransform's properties.

    out->writeVec3(getPosition());
    out->writeVec3(getPivotPoint());
    out->writeFloat(getAutoUpdateEyeMovementTolerance());
    
    out->writeInt(getAutoRotateMode());

    out->writeBool(getAutoScaleToScreen());

    out->writeQuat(getRotation());
    out->writeVec3(getScale());
}

void AutoTransform::read(DataInputStream* in){
    // Peek on AutoTransform's identification.
    int id = in->peekInt();
    if(id == IVEAUTOTRANSFORM){
        // Read AutoTransform's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Transform*  trans = dynamic_cast<osg::Transform*>(this);
        if(trans){
            ((ive::Transform*)(trans))->read(in);
        }
        else
            throw Exception("AutoTransform::read(): Could not cast this osg::AutoTransform to an osg::Transform.");
        // Read AutoTransform's properties

        setPosition(in->readVec3());
        setPivotPoint(in->readVec3());
        setAutoUpdateEyeMovementTolerance(in->readFloat());

        setAutoRotateMode(osg::AutoTransform::AutoRotateMode(in->readInt()));

        setAutoScaleToScreen(in->readBool());

        setRotation(in->readQuat());
        setScale(in->readVec3());
    }
    else{
        throw Exception("AutoTransform::read(): Expected AutoTransform identification.");
    }
}
