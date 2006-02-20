#ifndef IVE_AUTOTRANSFORM
#define IVE_AUTOTRANSFORM 1

#include <osg/AutoTransform>
#include "ReadWrite.h"

namespace ive{
class AutoTransform : public osg::AutoTransform, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
