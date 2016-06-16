#ifndef IVE_DRAWELEMENTSUSHORT
#define IVE_DRAWELEMENTSUSHORT 1

#include <osg/PrimitiveSet>
#include "ReadWrite.h"

namespace ive{
class DrawElementsUShort : public osg::DrawElementsUShort {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
