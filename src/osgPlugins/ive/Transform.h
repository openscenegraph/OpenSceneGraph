#ifndef IVE_TRANSFORM
#define IVE_TRANSFORM 1

#include <osg/Transform>
#include "ReadWrite.h"

namespace ive{
class Transform : public osg::Transform, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
