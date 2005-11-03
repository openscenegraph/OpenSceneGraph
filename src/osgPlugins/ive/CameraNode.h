#ifndef IVE_CAMERANODE
#define IVE_CAMERANODE 1

#include <osg/CameraNode>
#include "ReadWrite.h"

namespace ive{
class CameraNode : public osg::CameraNode, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
