#ifndef IVE_CLUSTERCULLINGCALLBACK
#define IVE_CLUSTERCULLINGCALLBACK 1

#include <osg/ClusterCullingCallback>
#include "ReadWrite.h"

namespace ive{
class ClusterCullingCallback : public osg::ClusterCullingCallback {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
