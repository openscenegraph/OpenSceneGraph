#ifndef IVE_IMPOSTOR
#define IVE_IMPOSTOR 1

#include <osg/Impostor>
#include "ReadWrite.h"

namespace ive{
class Impostor : public osg::Impostor, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
