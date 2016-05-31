#ifndef IVE_CONESECTOR
#define IVE_CONESECTOR 1

#include <osgSim/Sector>
#include "ReadWrite.h"

namespace ive{
class ConeSector : public osgSim::ConeSector {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
