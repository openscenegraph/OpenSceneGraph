#ifndef IVE_POLYGONMODE
#define IVE_POLYGONMODE 1

#include <osg/PolygonMode>
#include "ReadWrite.h"

namespace ive{
class PolygonMode : public osg::PolygonMode, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif


