#ifndef IVE_EFFECT
#define IVE_EFFECT 1

#include <osgFX/Effect>
#include "ReadWrite.h"

namespace ive{
class Effect : public osgFX::Effect, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
