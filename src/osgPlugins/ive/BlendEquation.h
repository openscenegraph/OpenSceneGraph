#ifndef IVE_BLENDEQUATION
#define IVE_BLENDEQUATION 1

#include <osg/BlendEquation>
#include "ReadWrite.h"

namespace ive{
class BlendEquation : public osg::BlendEquation, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
