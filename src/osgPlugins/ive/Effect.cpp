/**********************************************************************
 *
 *    FILE:           Effect.cpp
 *
 *    DESCRIPTION:    Read/Write osgFX::Effect in binary format to disk.
 *
 *    CREATED BY:     Liang Aibin
 *
 *    HISTORY:        Created 23.8.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "Effect.h"
#include "Group.h"

using namespace ive;

void Effect::write(DataOutputStream* out){
    // Write Effect's identification.
    out->writeInt(IVEEFFECT);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Group*  group = dynamic_cast<osg::Group*>(this);
    if(group){
        ((ive::Group*)(group))->write(out);
    }
    else
        throw Exception("Effect::write(): Could not cast this osgFX::Effect to an osg::Group.");

    // Write Effect's properties.
    out->writeBool(getEnabled());
    
    out->writeInt(getSelectedTechnique());
}

void Effect::read(DataInputStream* in){
    // Peek on Effect's identification.
    int id = in->peekInt();
    if(id == IVEEFFECT){
        // Read Effect's identification.
        id = in->readInt();

        // If the osg class is inherited by any other class we should also read this from file.
        osg::Group*  group = dynamic_cast<osg::Group*>(this);
        if(group){
            ((ive::Group*)(group))->read(in);
        }
        else
            throw Exception("Effect::read(): Could not cast this osgFX::Effect to an osg::Group.");

        // Read Effect's properties
        setEnabled(in->readBool());
        
        selectTechnique(in->readInt());
    }
    else{
        throw Exception("Effect::read(): Expected Effect identification.");
    }
}
