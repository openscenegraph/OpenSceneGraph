#ifndef IVE_TEXENV
#define IVE_TEXENV 1

#include <osg/TexEnv>
#include "ReadWrite.h"

namespace ive{
class TexEnv : public osg::TexEnv, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
