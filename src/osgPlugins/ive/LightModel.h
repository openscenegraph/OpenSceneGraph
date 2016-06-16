#ifndef IVE_LIGHTMODEL
#define IVE_LIGHTMODEL 1

#include <osg/LightModel>
#include "ReadWrite.h"

namespace ive{
  class LightModel : public osg::LightModel {
  public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
  };
}

#endif
