#ifndef IVE_COLORMASK
#define IVE_COLORMASK 1

#include <osg/ColorMask>
#include "ReadWrite.h"

namespace ive{
class ColorMask : public osg::ColorMask, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
