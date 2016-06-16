#ifndef IVE_MULTISWITCH
#define IVE_MULTISWITCH 1

#include <osgSim/MultiSwitch>
#include "ReadWrite.h"

namespace ive{
class MultiSwitch : public osgSim::MultiSwitch {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
