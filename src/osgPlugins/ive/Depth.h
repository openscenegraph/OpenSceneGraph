#ifndef IVE_DEPTH
#define IVE_DEPTH 1

#include <osg/Depth>
#include "ReadWrite.h"

namespace ive{
class Depth : public osg::Depth, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
