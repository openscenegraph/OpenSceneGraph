/**********************************************************************
 *
 *  FILE:           Depth.cpp
 *
 *  DESCRIPTION:    Read/Write osg::Depth in binary format to disk.
 *
 *  CREATED BY:     Botorabi AT gmx DOT net
 *
 *  HISTORY:        Created 14.12.2005
 *
 **********************************************************************/

#include "Exception.h"
#include "Depth.h"
#include "Object.h"

using namespace ive;

void Depth::write(DataOutputStream* out){
    // Write Depth's identification.
    out->writeInt(IVEDEPTH);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object*  obj = dynamic_cast<osg::Object*>(this);
    if(obj){
        ((ive::Object*)(obj))->write(out);
    }
    else
        out_THROW_EXCEPTION("Depth::write(): Could not cast this osg::Depth to an osg::Object.");
    // Write Depth's properties.
    out->writeInt(getFunction());
    out->writeBool(getWriteMask());
    out->writeFloat(getZNear());
    out->writeFloat(getZFar());
}

void Depth::read(DataInputStream* in){
    // Peek on Depth's identification.
    int id = in->peekInt();
    if(id == IVEDEPTH){
        // Read Depth's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if(obj){
            ((ive::Object*)(obj))->read(in);
        }
        else
            in_THROW_EXCEPTION("Depth::read(): Could not cast this osg::Depth to an osg::Object.");
        // Read CullFace's properties
        setFunction((osg::Depth::Function)in->readInt());
        setWriteMask(in->readBool());
        setZNear(in->readFloat());
        setZFar(in->readFloat());
    }
    else{
        in_THROW_EXCEPTION("Depth::read(): Expected Depth identification.");
    }
}

