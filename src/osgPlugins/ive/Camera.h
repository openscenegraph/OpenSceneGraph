#ifndef IVE_CAMERA
#define IVE_CAMERA 1

#include <osg/Camera>
#include "ReadWrite.h"

namespace ive{
class Camera : public osg::Camera {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
