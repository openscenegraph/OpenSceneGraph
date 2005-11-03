#ifndef IVE_CAMERAVIEW
#define IVE_CAMERAVIEW 1

#include <osg/CameraView>
#include "ReadWrite.h"

namespace ive{
class CameraView : public osg::CameraView, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
