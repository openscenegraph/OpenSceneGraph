/**********************************************************************
 *
 *	FILE:			ClipPlane.cpp
 *
 *	DESCRIPTION:	Read/Write osg::ClipPlane (partially) in binary format to disk.
 *
 *	CREATED BY:		Stanislav Blinov
 *
 *	HISTORY:		Created 7.09.2004
 *
 *	Copyright 2004 OtherSide
 **********************************************************************/

#include "Exception.h"
#include "ClipPlane.h"
#include "Object.h"

using namespace ive;

void ClipPlane::write(DataOutputStream* out){

  // write ClipPlane's identification
  out->writeInt(IVECLIPPLANE);

  // if the osg class is inherited by any other class we should also write this to file
  osg::Object*  obj = dynamic_cast<osg::Object*>(this);
  if(obj)
    ((ive::Object*)(obj))->write(out);
  else
    throw Exception("ClipPlane::write(): Could not cast this osg::ClipPlane to an osg::Object.");

  // write ClipPlane's properties
  
  
  
  
  double plane[4];
  getClipPlane(plane);
  
  out->writeDouble(plane[0]);
  out->writeDouble(plane[1]);
  out->writeDouble(plane[2]);
  out->writeDouble(plane[3]);
  
  out->writeUInt(getClipPlaneNum());

}

void ClipPlane::read(DataInputStream* in){

  // peek on ClipPlane's identification
  int id = in->peekInt();
  if(id == IVECLIPPLANE)
    {
      // read ClipPlane's identification
      id = in->readInt();

      // if the osg class is inherited by any other class we should also read this from file
      osg::Object*  obj = dynamic_cast<osg::Object*>(this);
      if(obj)
        ((ive::Object*)(obj))->read(in);
      else
        throw Exception("ClipPlane::read(): Could not cast this osg::ClipPlane to an osg::Object.");

      // Read ClipPlane's properties
      double plane[4];

      plane[0] = in->readDouble();
      plane[1] = in->readDouble();
      plane[2] = in->readDouble();
      plane[3] = in->readDouble();

      setClipPlane(plane);

      setClipPlaneNum(in->readUInt());
    }
  else{
    throw Exception("ClipPlane::read(): Expected ClipPlane identification.");
  }
}
