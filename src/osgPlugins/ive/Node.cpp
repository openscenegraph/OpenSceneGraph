/**********************************************************************
 *
 *    FILE:            Node.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Node in binary format to disk.
 *
 *    CREATED BY:        Rune Schmidt Jensen
 *
 *    HISTORY:        Created 10.03.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "Node.h"
#include "MatrixTransform.h"
#include "Group.h"
#include "Object.h"
#include "StateSet.h"
#include "AnimationPathCallback.h"
#include "ClusterCullingCallback.h"
#include "VolumePropertyAdjustmentCallback.h"

using namespace ive;


void Node::write(DataOutputStream* out){

    // Write node identification.
    out->writeInt(IVENODE);

    // Write out any inherited classes.
    osg::Object*  obj = dynamic_cast<osg::Object*>(this);
    if(obj){
        ((ive::Object*)(obj))->write(out);
    }
    else
        out_THROW_EXCEPTION("Node::write(): Could not cast this osg::Node to an osg::Object.");


    // Write osg::node properties.
    if ( out->getVersion() < VERSION_0012 )
    {
        // Write Name
        out->writeString(getName());
    }
    // Write culling active
    out->writeBool( getCullingActive());
    // Write Descriptions
    int nDesc =  getDescriptions().size();
    out->writeInt(nDesc);
    if(nDesc!=0){
        std::vector<std::string> desc =  getDescriptions();
        for(int i=0;i<nDesc;i++)
            out->writeString(desc[i]);
    }
    // Write Stateset if any
    out->writeBool( getStateSet()!=0);
    if(getStateSet())
        out->writeStateSet(getStateSet());

    // Write UpdateCallback if any
    osg::AnimationPathCallback* nc = dynamic_cast<osg::AnimationPathCallback*>(getUpdateCallback());
    out->writeBool(nc!=0);
    if(nc)
        {
        ((ive::AnimationPathCallback*)(nc))->write(out);
    }

    if (out->getVersion() >= VERSION_0006)
    {
        osg::ClusterCullingCallback* ccc = dynamic_cast<osg::ClusterCullingCallback*>(getCullCallback());
        out->writeBool(ccc!=0);
        if(ccc)
        {
            ((ive::ClusterCullingCallback*)(ccc))->write(out);
        }
    }


    if (out->getVersion() >= VERSION_0039)
    {
        osgVolume::PropertyAdjustmentCallback* pac = dynamic_cast<osgVolume::PropertyAdjustmentCallback*>(getEventCallback());
        out->writeBool(pac!=0);
        if(pac)
        {
            ((ive::VolumePropertyAdjustmentCallback*)pac)->write(out);
        }
    }

    if (out->getVersion() >= VERSION_0010)
    {
        const osg::BoundingSphere& bs = getInitialBound();
        out->writeBool(bs.valid());
        if (bs.valid())
        {
            out->writeVec3(bs.center());
            out->writeFloat(bs.radius());
        }
    }

    // Write NodeMask
    out->writeUInt(getNodeMask());
}


void Node::read(DataInputStream* in){
    // Peak on the identification id.
    int id = in->peekInt();

    if(id == IVENODE){
        id = in->readInt();
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if(obj){
            ((ive::Object*)(obj))->read(in);
        }
        else
            in_THROW_EXCEPTION("Node::read(): Could not cast this osg::Node to an osg::Object.");

        if ( in->getVersion() < VERSION_0012 )
        {
            // Read name
            setName(in->readString());
        }
        // Read Culling active
        setCullingActive(in->readBool());
        // Read descriptions
        int nDesc = in->readInt();
        if(nDesc!=0){
            for(int i=0;i<nDesc;i++)
                 addDescription(in->readString());
        }
        // Read StateSet if any
        if(in->readBool())
        {
            setStateSet(in->readStateSet());
        }

        // Read UpdateCallback if any
        if(in->readBool())
        {
            osg::AnimationPathCallback* nc = new osg::AnimationPathCallback();
            ((ive::AnimationPathCallback*)(nc))->read(in);
            setUpdateCallback(nc);
        }

        if (in->getVersion() >= VERSION_0006)
        {
            if(in->readBool())
            {
                osg::ClusterCullingCallback* ccc = new osg::ClusterCullingCallback();
                ((ive::ClusterCullingCallback*)(ccc))->read(in);
                setCullCallback(ccc);
            }
        }

        if (in->getVersion() >= VERSION_0039)
        {
            if(in->readBool())
            {
                int pacID = in->peekInt();
                if (pacID==IVEVOLUMEPROPERTYADJUSTMENTCALLBACK)
                {
                    osgVolume::PropertyAdjustmentCallback* pac = new osgVolume::PropertyAdjustmentCallback();
                    ((ive::VolumePropertyAdjustmentCallback*)(pac))->read(in);
                    setEventCallback(pac);
                }
                else
                {
                    in_THROW_EXCEPTION("Unknown event callback identification in Node::read()");
                }

            }
        }

        if (in->getVersion() >= VERSION_0010)
        {
            if (in->readBool())
            {
                osg::BoundingSphere bs;
                bs.center() = in->readVec3();
                bs.radius() = in->readFloat();
                setInitialBound(bs);
            }
        }

        // Read NodeMask
        setNodeMask(in->readUInt());
    }
    else{
        in_THROW_EXCEPTION("Node::read(): Expected Node identification");
    }
}
