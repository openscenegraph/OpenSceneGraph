#ifndef IVE_DRAWARRAYLENGTHS
#define IVE_DRAWARRAYLENGTHS 1

#include <osg/PrimitiveSet>
#include "ReadWrite.h"

namespace ive{
class DrawArrayLengths : public osg::DrawArrayLengths {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
