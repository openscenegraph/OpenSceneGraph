#ifndef IVE_DRAWARRAYS
#define IVE_DRAWARRAYS 1

#include <osg/PrimitiveSet>
#include "ReadWrite.h"

namespace ive{
class DrawArrays : public ReadWrite, public osg::DrawArrays{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
