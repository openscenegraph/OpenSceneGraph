#ifndef IVE_CULLFACE
#define IVE_CULLFACE 1

#include <osg/CullFace>
#include "ReadWrite.h"

namespace ive{
class CullFace : public osg::CullFace, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
