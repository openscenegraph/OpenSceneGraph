#ifndef IVE_SWITCH
#define IVE_SWITCH 1

#include <osg/Switch>
#include "ReadWrite.h"

namespace ive{
class Switch : public osg::Switch {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
