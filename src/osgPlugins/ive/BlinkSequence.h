#ifndef IVE_BLINKSEQUENCE
#define IVE_BLINKSEQUENCE 1

#include <osgSim/BlinkSequence>
#include "ReadWrite.h"

namespace ive{
class BlinkSequence : public osgSim::BlinkSequence {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
