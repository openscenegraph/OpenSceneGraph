/**********************************************************************
 *
 *    FILE:           LineStipple.cpp
 *
 *    DESCRIPTION:    Read/Write osg::LineStipple in binary format to disk.
 *
 *    CREATED BY:     Copied from LineWidth
 *                    and modified by Andrew Bettison
 *
 *    HISTORY:        Created 12.6.2008
 *
 *    Copyright 2008 VR-C
 **********************************************************************/

#include "Exception.h"
#include "LineStipple.h"
#include "Object.h"

using namespace ive;

void LineStipple::write(DataOutputStream* out){
    // Write CullFace's identification.
    out->writeInt(IVELINESTIPPLE);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object* obj = dynamic_cast<osg::Object*>(this);
    if (obj) {
        ((ive::Object*)(obj))->write(out);
    }
    else
        throw Exception("LineStipple::write(): Could not cast this osg::LineStipple to an osg::Object.");
    // Write LineStipple's properties.
    out->writeUShort(getPattern());
    out->writeInt(getFactor());
}

void LineStipple::read(DataInputStream* in){
    // Peek on LineStipple's identification.
    int id = in->peekInt();
    if (id == IVELINESTIPPLE) {
        // Read LineStipple's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if (obj) {
            ((ive::Object*)(obj))->read(in);
        }
        else
            throw Exception("LineStipple::read(): Could not cast this osg::LineStipple to an osg::Object.");
        // Read LineStipple's properties
        setPattern(in->readUShort());
        setFactor(in->readInt());
    }
    else{
        throw Exception("LineStipple::read(): Expected LineStipple identification.");
    }
}
