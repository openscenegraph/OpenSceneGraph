#ifndef IVE_AZIMELEVATIONSECTOR
#define IVE_AZIMELEVATIONSECTOR 1

#include <osgSim/Sector>
#include "ReadWrite.h"

namespace ive{
class AzimElevationSector : public osgSim::AzimElevationSector, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
