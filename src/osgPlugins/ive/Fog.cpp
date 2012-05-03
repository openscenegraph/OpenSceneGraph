/**********************************************************************
 *
 *    FILE:            Fog.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Fog in binary format to disk.
 *
 *    CREATED BY:        Liang Aibin
 *
 *    HISTORY:        Created 17.06.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "Fog.h"
#include "Object.h"

using namespace ive;

void Fog::write(DataOutputStream* out){

  // write Fog's identification
  out->writeInt(IVEFOG);

  // if the osg class is inherited by any other class we should also write this to file
  osg::Object*  obj = dynamic_cast<osg::Object*>(this);
  if(obj)
    ((ive::Object*)(obj))->write(out);
  else
    out_THROW_EXCEPTION("Fog::write(): Could not cast this osg::Fog to an osg::Object.");

  // write Fog's properties
  out->writeInt(getMode());
  out->writeFloat(getDensity());
  out->writeFloat(getStart());
  out->writeFloat(getEnd());
  out->writeVec4(getColor());
  out->writeInt(getFogCoordinateSource());
}

void Fog::read(DataInputStream* in){

  // peek on Fog's identification
  int id = in->peekInt();
  if(id == IVEFOG)
    {
      // read Fog's identification
      id = in->readInt();

      // if the osg class is inherited by any other class we should also read this from file
      osg::Object*  obj = dynamic_cast<osg::Object*>(this);
      if(obj)
        ((ive::Object*)(obj))->read(in);
      else
        in_THROW_EXCEPTION("Fog::read(): Could not cast this osg::Fog to an osg::Object.");

      // Read Fog's properties
      setMode(osg::Fog::Mode(in->readInt()));
      setDensity(in->readFloat());
      setStart(in->readFloat());
      setEnd(in->readFloat());
      setColor(in->readVec4());
      setFogCoordinateSource(in->readInt());
    }
  else{
    in_THROW_EXCEPTION("Fog::read(): Expected Fog identification.");
  }
}
