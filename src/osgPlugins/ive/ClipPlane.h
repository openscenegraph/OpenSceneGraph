#ifndef IVE_ClipPlane
#define IVE_ClipPlane 1

#include <osg/ClipPlane>
#include "ReadWrite.h"

namespace ive{
  class ClipPlane : public osg::ClipPlane, public ReadWrite {
  public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
  };
}

#endif
