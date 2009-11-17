/**********************************************************************
 *
 *    FILE:            AlphaFunc.cpp
 *
 *    DESCRIPTION:    Read/Write osg::AlphaFunc in binary format to disk.
 *
 *    CREATED BY:        Pavlo Moloshtan
 *
 *    HISTORY:        Created 30.11.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "AlphaFunc.h"
#include "Object.h"

using namespace ive;

void AlphaFunc::write(DataOutputStream* out){

  // write AlphaFunc's identification
  out->writeInt(IVEALPHAFUNC);

  // if the osg class is inherited by any other class we should also write this to file
  osg::Object*  obj = dynamic_cast<osg::Object*>(this);
  if(obj)
    ((ive::Object*)(obj))->write(out);
  else
    out_THROW_EXCEPTION("AlphaFunc::write(): Could not cast this osg::AlphaFunc to an osg::Object.");

  // write AlphaFunc's properties
  out->writeInt(getFunction());
  out->writeFloat(getReferenceValue());
}

void AlphaFunc::read(DataInputStream* in){

  // peek on AlphaFunc's identification
  int id = in->peekInt();
  if(id == IVEALPHAFUNC)
    {
      // read AlphaFunc's identification
      id = in->readInt();

      // if the osg class is inherited by any other class we should also read this from file
      osg::Object*  obj = dynamic_cast<osg::Object*>(this);
      if(obj)
        ((ive::Object*)(obj))->read(in);
      else
        in_THROW_EXCEPTION("AlphaFunc::read(): Could not cast this osg::AlphaFunc to an osg::Object.");

      // Read AlphaFunc's properties
      osg::AlphaFunc::ComparisonFunction comparison_funtion = osg::AlphaFunc::ComparisonFunction(in->readInt());
      float reference_value = in->readFloat();
      setFunction(comparison_funtion, reference_value);
    }
  else{
    in_THROW_EXCEPTION("AlphaFunc::read(): Expected AlphaFunc identification.");
  }
}
