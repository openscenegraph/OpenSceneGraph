#ifndef IVE_ALPHAFUNC
#define IVE_ALPHAFUNC 1

#include <osg/AlphaFunc>
#include "ReadWrite.h"

namespace ive{
  class AlphaFunc : public osg::AlphaFunc, public ReadWrite {
  public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
  };
}

#endif
