/**********************************************************************
 *
 *    FILE:            LightModel.cpp
 *
 *    DESCRIPTION:    Read/Write osg::LightModel (partially) in binary format to disk.
 *
 *    CREATED BY:        Stanislav Blinov
 *
 *    HISTORY:        Created 7.09.2004
 *
 *    Copyright 2004 OtherSide
 **********************************************************************/

#include "Exception.h"
#include "LightModel.h"
#include "Object.h"

using namespace ive;

void LightModel::write(DataOutputStream* out){

  // write LightModel's identification
  out->writeInt(IVELIGHTMODEL);

  // if the osg class is inherited by any other class we should also write this to file
  osg::Object*  obj = dynamic_cast<osg::Object*>(this);
  if(obj)
    ((ive::Object*)(obj))->write(out);
  else
    out_THROW_EXCEPTION("LightModel::write(): Could not cast this osg::LightModel to an osg::Object.");

  // write LightModel's properties
  out->writeBool(getTwoSided());
  out->writeBool(getLocalViewer());
  out->writeVec4(getAmbientIntensity());
  out->writeInt( getColorControl());
}

void LightModel::read(DataInputStream* in){

  // peek on LightModel's identification
  int id = in->peekInt();
  if(id == IVELIGHTMODEL)
    {
      // read LightModel's identification
      id = in->readInt();

      // if the osg class is inherited by any other class we should also read this from file
      osg::Object*  obj = dynamic_cast<osg::Object*>(this);
      if(obj)
        ((ive::Object*)(obj))->read(in);
      else
        in_THROW_EXCEPTION("LightModel::read(): Could not cast this osg::LightModel to an osg::Object.");

      // Read LightModel's properties
      setTwoSided(in->readBool());
      setLocalViewer(in->readBool());
      setAmbientIntensity(in->readVec4());
      setColorControl((ColorControl)in->readInt());
    }
  else{
    in_THROW_EXCEPTION("LightModel::read(): Expected LightModel identification.");
  }
}
