/**********************************************************************
 *
 *    FILE:           Cartoon.cpp
 *
 *    DESCRIPTION:    Read/Write osgFX::Cartoon in binary format to disk.
 *
 *    CREATED BY:     Liang Aibin
 *
 *    HISTORY:        Created 23.8.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "Cartoon.h"
#include "Effect.h"

using namespace ive;

void Cartoon::write(DataOutputStream* out){
    // Write Cartoon's identification.
    out->writeInt(IVECARTOON);
    // If the osg class is inherited by any other class we should also write this to file.
    osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
    if(effect){
        ((ive::Effect*)(effect))->write(out);
    }
    else
        out_THROW_EXCEPTION("Cartoon::write(): Could not cast this osgFX::Cartoon to an osgFX::Effect.");

    // Write Cartoon's properties.
    out->writeVec4(getOutlineColor());
    out->writeFloat(getOutlineLineWidth());
    out->writeInt(getLightNumber());
}

void Cartoon::read(DataInputStream* in){
    // Peek on Cartoon's identification.
    int id = in->peekInt();
    if(id == IVECARTOON){
        // Read Cartoon's identification.
        id = in->readInt();

        // If the osg class is inherited by any other class we should also read this from file.
        osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
        if(effect){
            ((ive::Effect*)(effect))->read(in);
        }
        else
            in_THROW_EXCEPTION("Cartoon::read(): Could not cast this osgFX::Cartoon to an osgFX::Effect.");

        // Read Cartoon's properties
        setOutlineColor(in->readVec4());
        setOutlineLineWidth(in->readFloat());
        setLightNumber(in->readInt());
    }
    else{
        in_THROW_EXCEPTION("Cartoon::read(): Expected Cartoon identification.");
    }
}
