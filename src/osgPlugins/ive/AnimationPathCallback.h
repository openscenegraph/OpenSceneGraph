#ifndef IVE_ANIMATIONPATHCALLBACK
#define IVE_ANIMATIONPATHCALLBACK 1

#include <osg/AnimationPath>
#include "ReadWrite.h"

namespace ive{
class AnimationPathCallback : public osg::AnimationPathCallback, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
