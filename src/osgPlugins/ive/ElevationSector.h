#ifndef IVE_ELEVATIONSECTOR
#define IVE_ELEVATIONSECTOR 1

#include <osgSim/Sector>
#include "ReadWrite.h"

namespace ive{
class ElevationSector : public osgSim::ElevationSector {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
