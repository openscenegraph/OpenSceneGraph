/**********************************************************************
 *
 *    FILE:            Multisample.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Multisample in binary format to disk.
 *
 *    CREATED BY:     Nikolaus Hanekamp
 *
 *
 *    HISTORY:        Created 15.06.2007
 *
 **********************************************************************/

#include "Exception.h"
#include "Multisample.h"
#include "Object.h"

using namespace ive;

void Multisample::write(DataOutputStream* out){
    // Write CullFace's identification.
    out->writeInt(IVEMULTISAMPLE);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object*  obj = dynamic_cast<osg::Object*>(this);
    if(obj){
        ((ive::Object*)(obj))->write(out);
    }
    else
        out_THROW_EXCEPTION("Multisample::write(): Could not cast this osg::Multisample to an osg::Object.");
    // Write Multisample's properties.
    out->writeFloat(getCoverage());
    out->writeBool(getInvert());
    out->writeInt(getHint());
}

void Multisample::read(DataInputStream* in){
    // Peek on Multisample's identification.
    int id = in->peekInt();
    if(id == IVEMULTISAMPLE){
        // Read Multisample's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if(obj){
            ((ive::Object*)(obj))->read(in);
        }
        else
            in_THROW_EXCEPTION("Multisample::read(): Could not cast this osg::Multisample to an osg::Object.");
        // Read Multisample's properties
        setCoverage(in->readFloat());
        setInvert(in->readBool());
        setHint((Mode) in->readInt());
    }
    else{
        in_THROW_EXCEPTION("Multisample::read(): Expected Multisample identification.");
    }
}
