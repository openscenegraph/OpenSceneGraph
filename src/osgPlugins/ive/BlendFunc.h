#ifndef IVE_BLENDFUNC
#define IVE_BLENDFUNC 1

#include <osg/BlendFunc>
#include "ReadWrite.h"

namespace ive{
class BlendFunc : public osg::BlendFunc, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
