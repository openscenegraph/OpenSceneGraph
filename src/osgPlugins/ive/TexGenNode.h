#ifndef IVE_TEXGENNODE
#define IVE_TEXGENNODE 1

#include <osg/TexGenNode>
#include "ReadWrite.h"

namespace ive{
class TexGenNode : public osg::TexGenNode {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
