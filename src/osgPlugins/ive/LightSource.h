#ifndef IVE_LIGHTSOURCE
#define IVE_LIGHTSOURCE 1

#include <osg/LightSource>
#include "ReadWrite.h"

namespace ive{
class LightSource : public osg::LightSource {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
