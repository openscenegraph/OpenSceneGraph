#ifndef IVE_DRAWARRAYS
#define IVE_DRAWARRAYS 1

#include <osg/PrimitiveSet>
#include "ReadWrite.h"

namespace ive{
class IVE_EXPORT DrawArrays : public ReadWrite, public osg::DrawArrays{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
