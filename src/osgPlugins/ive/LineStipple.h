#ifndef IVE_LINESTIPPLE
#define IVE_LINESTIPPLE 1

#include <osg/LineStipple>
#include "ReadWrite.h"

namespace ive{
class LineStipple : public osg::LineStipple {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
