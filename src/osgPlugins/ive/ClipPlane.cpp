/**********************************************************************
 *
 *    FILE:            ClipPlane.cpp
 *
 *    DESCRIPTION:    Read/Write osg::ClipPlane (partially) in binary format to disk.
 *
 *    CREATED BY:        Stanislav Blinov
 *
 *    HISTORY:        Created 7.09.2004
 *
 *    Copyright 2004 OtherSide
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
    out_THROW_EXCEPTION("ClipPlane::write(): Could not cast this osg::ClipPlane to an osg::Object.");

  // write ClipPlane's properties

  out->writeVec4d(getClipPlane());

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
        in_THROW_EXCEPTION("ClipPlane::read(): Could not cast this osg::ClipPlane to an osg::Object.");

      setClipPlane(in->readVec4d());

      setClipPlaneNum(in->readUInt());
    }
  else{
    in_THROW_EXCEPTION("ClipPlane::read(): Expected ClipPlane identification.");
  }
}
