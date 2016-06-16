#ifndef IVE_TEXMAT
#define IVE_TEXMAT 1

#include <osg/TexMat>
#include "ReadWrite.h"

namespace ive{
class TexMat : public osg::TexMat {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
