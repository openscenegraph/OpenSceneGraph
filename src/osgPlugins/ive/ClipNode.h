#ifndef IVE_ClipNode
#define IVE_ClipNode 1

#include <osg/ClipNode>
#include "ReadWrite.h"

namespace ive{
class ClipNode : public osg::ClipNode, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
