#ifndef IVE_PRIMITIVESET
#define IVE_PRIMITIVESET 1

#include <osg/PrimitiveSet>
#include "ReadWrite.h"

namespace ive{
class PrimitiveSet : public ReadWrite, public osg::PrimitiveSet{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
