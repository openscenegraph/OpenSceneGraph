#ifndef IVE_COORDINATESYSTEMNODE
#define IVE_COORDINATESYSTEMNODE 1

#include <osg/CoordinateSystemNode>
#include "ReadWrite.h"

namespace ive{
class CoordinateSystemNode : public osg::CoordinateSystemNode {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
