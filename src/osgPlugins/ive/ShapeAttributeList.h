#ifndef IVE_SHAPEATTRIBUTELIST
#define IVE_SHAPEATTRIBUTELIST 1

#include <osgSim/ShapeAttribute>
#include "ReadWrite.h"

namespace ive{
class ShapeAttributeList : public osgSim::ShapeAttributeList {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);

    void write(DataOutputStream* out, const osgSim::ShapeAttribute& sa);
    void read(DataInputStream* in, osgSim::ShapeAttribute& sa);
};
}

#endif
