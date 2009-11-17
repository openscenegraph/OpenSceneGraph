/**********************************************************************
 *
 *    FILE:           SpecularHighlights.cpp
 *
 *    DESCRIPTION:    Read/Write osgFX::SpecularHighlights in binary format to disk.
 *
 *    CREATED BY:     Liang Aibin
 *
 *    HISTORY:        Created 23.8.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "SpecularHighlights.h"
#include "Effect.h"

using namespace ive;

void SpecularHighlights::write(DataOutputStream* out){
    // Write SpecularHighlights's identification.
    out->writeInt(IVEANISOTROPICLIGHTING);
    // If the osg class is inherited by any other class we should also write this to file.
    osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
    if(effect){
        ((ive::Effect*)(effect))->write(out);
    }
    else
        out_THROW_EXCEPTION("SpecularHighlights::write(): Could not cast this osgFX::SpecularHighlights to an osgFX::Effect.");

    // Write SpecularHighlights's properties.
    out->writeInt(getLightNumber());
    out->writeInt(getTextureUnit());
    out->writeVec4(getSpecularColor());
    out->writeFloat(getSpecularExponent());
}

void SpecularHighlights::read(DataInputStream* in){
    // Peek on SpecularHighlights's identification.
    int id = in->peekInt();
    if(id == IVEANISOTROPICLIGHTING){
        // Read SpecularHighlights's identification.
        id = in->readInt();

        // If the osg class is inherited by any other class we should also read this from file.
        osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
        if(effect){
            ((ive::Effect*)(effect))->read(in);
        }
        else
            in_THROW_EXCEPTION("SpecularHighlights::read(): Could not cast this osgFX::SpecularHighlights to an osgFX::Effect.");

        // Read SpecularHighlights's properties
        setLightNumber(in->readInt());
        setTextureUnit(in->readInt());
        setSpecularColor(in->readVec4());
        setSpecularExponent(in->readFloat());
    }
    else{
        in_THROW_EXCEPTION("SpecularHighlights::read(): Expected SpecularHighlights identification.");
    }
}
