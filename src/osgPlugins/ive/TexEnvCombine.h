#ifndef IVE_TEXENVCOMBINE
#define IVE_TEXENVCOMBINE 1

#include <osg/TexEnvCombine>
#include "ReadWrite.h"

namespace ive{
class IVE_EXPORT TexEnvCombine : public osg::TexEnvCombine, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
