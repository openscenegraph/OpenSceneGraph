#ifndef IVE_OCCLUDERNODE
#define IVE_OCCLUDERNODE 1

#include <osg/OccluderNode>

#include "ReadWrite.h"

namespace ive{
class OccluderNode : public osg::OccluderNode {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
