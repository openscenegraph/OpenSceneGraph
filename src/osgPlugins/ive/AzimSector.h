#ifndef IVE_AZIMSECTOR
#define IVE_AZIMSECTOR 1

#include <osgSim/Sector>
#include "ReadWrite.h"

namespace ive{
class AzimSector : public osgSim::AzimSector, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
