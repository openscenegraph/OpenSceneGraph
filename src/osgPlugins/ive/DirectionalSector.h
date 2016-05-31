#ifndef IVE_DIRECTIONALSECTOR
#define IVE_DIRECTIONALSECTOR 1

#include <osgSim/Sector>
#include "ReadWrite.h"

namespace ive{
class DirectionalSector : public osgSim::DirectionalSector {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
