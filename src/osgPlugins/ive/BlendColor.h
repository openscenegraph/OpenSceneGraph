#ifndef IVE_BLENDCOLOR
#define IVE_BLENDCOLOR 1

#include <osg/BlendColor>
#include "ReadWrite.h"

namespace ive{
   class BlendColor : public osg::BlendColor, public ReadWrite {
       public:
           void write(DataOutputStream* out);
           void read(DataInputStream* in);
   };
}

#endif
