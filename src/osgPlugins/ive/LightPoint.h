#ifndef IVE_LIGHTPOINT
#define IVE_LIGHTPOINT 1

#include <osgSim/LightPoint>
#include "ReadWrite.h"

namespace ive{
class LightPoint : public osgSim::LightPoint, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
        virtual ~LightPoint() {}
};
}

#endif
