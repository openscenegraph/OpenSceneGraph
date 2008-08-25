/**********************************************************************
 *
 *    FILE:           BumpMapping.cpp
 *
 *    DESCRIPTION:    Read/Write osgFX::BumpMapping in binary format to disk.
 *
 *    CREATED BY:     Liang Aibin
 *
 *    HISTORY:        Created 23.8.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "BumpMapping.h"
#include "Effect.h"
#include "Texture2D.h"

using namespace ive;

void BumpMapping::write(DataOutputStream* out){
    // Write BumpMapping's identification.
    out->writeInt(IVEBUMPMAPPING);
    // If the osg class is inherited by any other class we should also write this to file.
    osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
    if(effect){
        ((ive::Effect*)(effect))->write(out);
    }
    else
        throw Exception("BumpMapping::write(): Could not cast this osgFX::BumpMapping to an osgFX::Effect.");

    // Write BumpMapping's properties.
    out->writeInt(getLightNumber());
    out->writeInt(getDiffuseTextureUnit());
    out->writeInt(getNormalMapTextureUnit());

    osg::Texture2D *tex=getOverrideDiffuseTexture();
    ((ive::Texture2D*)(tex))->write(out);

    tex=getOverrideNormalMapTexture();
    ((ive::Texture2D*)(tex))->write(out);
}

void BumpMapping::read(DataInputStream* in){
    // Peek on BumpMapping's identification.
    int id = in->peekInt();
    if(id == IVEBUMPMAPPING){
        // Read BumpMapping's identification.
        id = in->readInt();

        // If the osg class is inherited by any other class we should also read this from file.
        osgFX::Effect*  effect = dynamic_cast<osgFX::Effect*>(this);
        if(effect){
            ((ive::Effect*)(effect))->read(in);
        }
        else
            throw Exception("BumpMapping::read(): Could not cast this osgFX::BumpMapping to an osgFX::Effect.");

        // Read BumpMapping's properties
        setLightNumber(in->readInt());
        setDiffuseTextureUnit(in->readInt());
        setNormalMapTextureUnit(in->readInt());

        osg::Texture2D *tex=new osg::Texture2D;
        ((ive::Texture2D*)(tex))->read(in);
        setOverrideDiffuseTexture(tex);

        tex=new osg::Texture2D;
        ((ive::Texture2D*)(tex))->read(in);
        setOverrideNormalMapTexture(tex);
    }
    else{
        throw Exception("BumpMapping::read(): Expected BumpMapping identification.");
    }
}
