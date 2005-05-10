#ifndef IVE_UNIFORM
#define IVE_UNIFORM 1

#include <osg/Uniform>
#include "ReadWrite.h"

namespace ive{
class Uniform : public osg::Uniform, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
