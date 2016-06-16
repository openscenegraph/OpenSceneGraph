#ifndef IVE_POSITIONATTITUDETRANSFORM
#define IVE_POSITIONATTITUDETRANSFORM 1

#include <osg/PositionAttitudeTransform>
#include "ReadWrite.h"

namespace ive{
class PositionAttitudeTransform : public osg::PositionAttitudeTransform {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
