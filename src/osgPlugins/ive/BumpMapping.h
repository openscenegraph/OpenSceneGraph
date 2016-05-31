#ifndef IVE_BUMPMAPPING
#define IVE_BUMPMAPPING 1

#include <osgFX/BumpMapping>
#include "ReadWrite.h"

namespace ive{
class BumpMapping : public osgFX::BumpMapping {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
