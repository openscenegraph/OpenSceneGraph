#ifndef IVE_ANIMATIONPATH
#define IVE_ANIMATIONPATH 1

#include <osg/AnimationPath>
#include "ReadWrite.h"

namespace ive{
class AnimationPath : public osg::AnimationPath, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
