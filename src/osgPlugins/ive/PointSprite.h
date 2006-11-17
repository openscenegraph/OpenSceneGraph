#ifndef IVE_POINTSPRITE
#define IVE_POINTSPRITE 1

#include <osg/PointSprite>
#include "ReadWrite.h"

namespace ive{
class PointSprite : public osg::PointSprite, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif

