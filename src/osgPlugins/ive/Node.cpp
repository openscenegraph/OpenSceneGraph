/**********************************************************************
 *
 *	FILE:			Node.cpp
 *
 *	DESCRIPTION:	Read/Write osg::Node in binary format to disk.
 *
 *	CREATED BY:		Rune Schmidt Jensen
 *
 *	HISTORY:		Created 10.03.2003
 *
 *	Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "Node.h"
#include "MatrixTransform.h"
#include "Group.h"
#include "Object.h"
#include "StateSet.h"
#include "AnimationPathCallback.h"

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
		throw Exception("Node::write(): Could not cast this osg::Node to an osg::Object.");


	// Write osg::node properties.

	// Write Name
	out->writeString(getName());
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
	out->writeInt((int) getStateSet());
	if(getStateSet())
		out->writeStateSet(getStateSet());


	// Write UpdateCallback if any
	osg::NodeCallback* nc = getUpdateCallback();
	if(nc && dynamic_cast<osg::AnimationPathCallback*>(nc)){
		out->writeInt((int)nc);
		((ive::AnimationPathCallback*)(nc))->write(out);
	}
	else
		out->writeInt(0x0);
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
			throw Exception("Node::read(): Could not cast this osg::Node to an osg::Object.");
		// Read name
		setName(in->readString());
		// Read Culling active
		setCullingActive(in->readBool());
		// Read descriptions
		int nDesc = in->readInt();
		if(nDesc!=0){
			for(int i=0;i<nDesc;i++)
				 addDescription(in->readString());
		}
		// Read StateSet if any
		if(in->readInt()){
			setStateSet(in->readStateSet());
		}
		// Read UpdateCallback if any
		if(in->readInt()){
			osg::AnimationPathCallback* nc = new osg::AnimationPathCallback();
			((ive::AnimationPathCallback*)(nc))->read(in);
			setUpdateCallback(nc);
		}
	}
	else{
		throw Exception("Node::read(): Expected Node identification");
	}
}