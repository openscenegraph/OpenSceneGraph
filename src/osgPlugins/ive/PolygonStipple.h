#ifndef IVE_POLYGONSTIPPLE
#define IVE_POLYGONSTIPPLE 1

#include <osg/PolygonStipple>
#include "ReadWrite.h"

namespace ive{
class PolygonStipple : public osg::PolygonStipple {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
