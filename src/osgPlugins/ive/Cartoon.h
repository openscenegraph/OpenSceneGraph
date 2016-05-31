#ifndef IVE_CARTOON
#define IVE_CARTOON 1

#include <osgFX/Cartoon>
#include "ReadWrite.h"

namespace ive{
class Cartoon : public osgFX::Cartoon {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
