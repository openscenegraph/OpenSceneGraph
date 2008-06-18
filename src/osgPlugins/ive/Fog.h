#ifndef IVE_FOG
#define IVE_FOG 1

#include <osg/Fog>
#include "ReadWrite.h"

namespace ive{
  class Fog : public osg::Fog, public ReadWrite {
  public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
  };
}

#endif
