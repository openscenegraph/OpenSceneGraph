#ifndef IVE_SCISSOR
#define IVE_SCISSOR 1

#include <osg/Scissor>
#include "ReadWrite.h"

namespace ive{
class Scissor : public osg::Scissor, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
