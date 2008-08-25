#ifndef IVE_ANISOTROPICLIGHTING
#define IVE_ANISOTROPICLIGHTING 1

#include <osgFX/AnisotropicLighting>
#include "ReadWrite.h"

namespace ive{
class AnisotropicLighting : public osgFX::AnisotropicLighting, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
