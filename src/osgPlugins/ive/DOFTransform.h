#ifndef IVE_DOFTRANSFORM
#define IVE_DOFTRANSFORM 1

#include <osgSim/DOFTransform>
#include "ReadWrite.h"

namespace ive{
class DOFTransform : public osgSim::DOFTransform, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
