#ifndef IVE_DRAWELEMENTSUBYTE
#define IVE_DRAWELEMENTSUBYTE 1

#include <osg/PrimitiveSet>
#include "ReadWrite.h"

namespace ive{
class DrawElementsUByte : public osg::DrawElementsUByte {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
