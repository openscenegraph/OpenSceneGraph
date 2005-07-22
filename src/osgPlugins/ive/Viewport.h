#ifndef IVE_VIEWPORT
#define IVE_VIEWPORT 1

#include <osg/Viewport>
#include "ReadWrite.h"

namespace ive{
class Viewport : public osg::Viewport, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
