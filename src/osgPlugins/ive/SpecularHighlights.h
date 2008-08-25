#ifndef IVE_SPECULARHIGHLIGHTS
#define IVE_SPECULARHIGHLIGHTS 1

#include <osgFX/SpecularHighlights>
#include "ReadWrite.h"

namespace ive{
class SpecularHighlights : public osgFX::SpecularHighlights, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
