#ifndef IVE_SCRIBE
#define IVE_SCRIBE 1

#include <osgFX/Scribe>
#include "ReadWrite.h"

namespace ive{
class Scribe : public osgFX::Scribe, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
