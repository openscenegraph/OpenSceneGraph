/**********************************************************************
 *
 *    FILE:           PolygonStipple.cpp
 *
 *    DESCRIPTION:    Read/Write osg::PolygonStipple in binary format to disk.
 *
 *    CREATED BY:     Copied from LineStipple
 *                    and modified by Luc Frauciel
 *
 *    HISTORY:        Created 21.3.2009
 *
 *    Copyright 2008 VR-C
 **********************************************************************/

#include "Exception.h"
#include "PolygonStipple.h"
#include "Object.h"

using namespace ive;

void PolygonStipple::write(DataOutputStream* out){
    // Write CullFace's identification.
    out->writeInt(IVEPOLYGONSTIPPLE);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object* obj = dynamic_cast<osg::Object*>(this);
    if (obj) {
        ((ive::Object*)(obj))->write(out);
    }
    else
        out_THROW_EXCEPTION("PolygonStipple::write(): Could not cast this osg::PolygonStipple to an osg::Object.");
    // Write PolygonStipple's properties.
    out->writeUByteArray(new osg::UByteArray(128,const_cast<GLubyte*>(getMask())));
}

void PolygonStipple::read(DataInputStream* in){
    // Peek on LineStipple's identification.
    int id = in->peekInt();
    if (id == IVEPOLYGONSTIPPLE) {
        // Read PolygonStipple's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if (obj) {
            ((ive::Object*)(obj))->read(in);
        }
        else
            in_THROW_EXCEPTION("PolygonStipple::read(): Could not cast this osg::PolygonStipple to an osg::Object.");
        // Read PolygonStipple's properties
        setMask((GLubyte *)in->readUByteArray()->getDataPointer());
    }
    else{
        in_THROW_EXCEPTION("PolygonStipple::read(): Expected PolygonStipple identification.");
    }
}
