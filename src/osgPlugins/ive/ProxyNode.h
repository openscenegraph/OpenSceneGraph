#ifndef IVE_PROXYNODE
#define IVE_PROXYNODE 1

#include <osg/ProxyNode>
#include "ReadWrite.h"

namespace ive{
class ProxyNode : public osg::ProxyNode, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
