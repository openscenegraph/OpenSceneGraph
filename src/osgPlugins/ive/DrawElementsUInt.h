#ifndef IVE_DRAWELEMENTSUINT
#define IVE_DRAWELEMENTSUINT 1

#include <osg/PrimitiveSet>
#include "ReadWrite.h"

namespace ive{
class DrawElementsUInt : public osg::DrawElementsUInt, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
