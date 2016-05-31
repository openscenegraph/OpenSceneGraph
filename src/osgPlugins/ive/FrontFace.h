#ifndef IVE_FRONTFACE
#define IVE_FRONTFACE 1

#include <osg/FrontFace>
#include "ReadWrite.h"

namespace ive{
class FrontFace : public osg::FrontFace {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif

