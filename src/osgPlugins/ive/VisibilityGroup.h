#ifndef IVE_VISIBILITYGROUP
#define IVE_VISIBILITYGROUP 1

#include <osgSim/VisibilityGroup>
#include "ReadWrite.h"

namespace ive{
class VisibilityGroup : public osgSim::VisibilityGroup {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
