#ifndef IVE_LIGHT
#define IVE_LIGHT 1

#include <osg/Light>
#include "ReadWrite.h"

namespace ive{
class Light : public osg::Light, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
