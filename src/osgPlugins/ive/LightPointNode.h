#ifndef IVE_LIGHTPOINTNODE
#define IVE_LIGHTPOINTNODE 1

#include <osgSim/LightPointNode>
#include "ReadWrite.h"

namespace ive{
class LightPointNode : public osgSim::LightPointNode, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
