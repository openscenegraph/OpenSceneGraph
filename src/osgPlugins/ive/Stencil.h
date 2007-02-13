#ifndef IVE_STENCIL
#define IVE_STENCIL 1

#include <osg/Stencil>
#include "ReadWrite.h"

namespace ive{
class Stencil : public osg::Stencil, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
