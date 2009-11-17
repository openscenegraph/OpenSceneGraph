/**********************************************************************
 *
 *    FILE:           Scribe.cpp
 *
 *    DESCRIPTION:    Read/Write osgFX::Scribe in binary format to disk.
 *
 *    CREATED BY:     Liang Aibin
 *
 *    HISTORY:        Created 23.8.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "Scribe.h"
#include "Effect.h"

using namespace ive;

void Scribe::write(DataOutputStream* out){
    // Write Scribe's identification.
    out->writeInt(IVESCRIBE);
    // If the osg class is inherited by any other class we should also write this to file.
    osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
    if(effect){
        ((ive::Effect*)(effect))->write(out);
    }
    else
        out_THROW_EXCEPTION("Scribe::write(): Could not cast this osgFX::Scribe to an osgFX::Effect.");

    // Write Scribe's properties.
    out->writeVec4(getWireframeColor());
    out->writeFloat(getWireframeLineWidth());
}

void Scribe::read(DataInputStream* in){
    // Peek on Scribe's identification.
    int id = in->peekInt();
    if(id == IVESCRIBE){
        // Read Scribe's identification.
        id = in->readInt();

        // If the osg class is inherited by any other class we should also read this from file.
        osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
        if(effect){
            ((ive::Effect*)(effect))->read(in);
        }
        else
            in_THROW_EXCEPTION("Scribe::read(): Could not cast this osgFX::Scribe to an osgFX::Effect.");

        // Read Scribe's properties
        setWireframeColor(in->readVec4());
        setWireframeLineWidth(in->readFloat());
    }
    else{
        in_THROW_EXCEPTION("Scribe::read(): Expected Scribe identification.");
    }
}
