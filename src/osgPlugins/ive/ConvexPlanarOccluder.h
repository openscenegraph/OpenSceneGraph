#ifndef IVE_CONVEXPLANAROCCLUDER
#define IVE_CONVEXPLANAROCCLUDER 1

#include <osg/ConvexPlanarOccluder>
#include "ReadWrite.h"

namespace ive{
class ConvexPlanarOccluder : public osg::ConvexPlanarOccluder, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
