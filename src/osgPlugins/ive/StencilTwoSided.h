#ifndef IVE_STENCIL_TWO_SIDED
#define IVE_STENCIL_TWO_SIDED 1

#include <osg/StencilTwoSided>
#include "ReadWrite.h"

namespace ive{
class StencilTwoSided : public osg::StencilTwoSided {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
