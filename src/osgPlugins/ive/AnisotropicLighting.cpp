/**********************************************************************
 *
 *    FILE:           AnisotropicLighting.cpp
 *
 *    DESCRIPTION:    Read/Write osgFX::AnisotropicLighting in binary format to disk.
 *
 *    CREATED BY:     Liang Aibin
 *
 *    HISTORY:        Created 23.8.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "AnisotropicLighting.h"
#include "Effect.h"

using namespace ive;

void AnisotropicLighting::write(DataOutputStream* out){
    // Write AnisotropicLighting's identification.
    out->writeInt(IVEANISOTROPICLIGHTING);
    // If the osg class is inherited by any other class we should also write this to file.
    osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
    if(effect){
        ((ive::Effect*)(effect))->write(out);
    }
    else
        out_THROW_EXCEPTION("AnisotropicLighting::write(): Could not cast this osgFX::AnisotropicLighting to an osgFX::Effect.");

    // Write AnisotropicLighting's properties.
    out->writeImage(getLightingMap());
    out->writeInt(getLightNumber());
}

void AnisotropicLighting::read(DataInputStream* in){
    // Peek on AnisotropicLighting's identification.
    int id = in->peekInt();
    if(id == IVEANISOTROPICLIGHTING){
        // Read AnisotropicLighting's identification.
        id = in->readInt();

        // If the osg class is inherited by any other class we should also read this from file.
        osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
        if(effect){
            ((ive::Effect*)(effect))->read(in);
        }
        else
            in_THROW_EXCEPTION("AnisotropicLighting::read(): Could not cast this osgFX::AnisotropicLighting to an osgFX::Effect.");

        // Read AnisotropicLighting's properties
        setLightingMap(in->readImage());
        setLightNumber(in->readInt());
    }
    else{
        in_THROW_EXCEPTION("AnisotropicLighting::read(): Expected AnisotropicLighting identification.");
    }
}
