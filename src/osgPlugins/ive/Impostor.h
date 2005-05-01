#ifndef IVE_IMPOSTOR
#define IVE_IMPOSTOR 1

#include <osgSim/Impostor>
#include "ReadWrite.h"

namespace ive{
class Impostor : public osgSim::Impostor, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
