#ifndef IVE_STATESET
#define IVE_STATESET 1

#include <osg/StateSet>
#include "ReadWrite.h"

namespace ive{
class StateSet : public ReadWrite, public osg::StateSet{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* out);
};
}

#endif
