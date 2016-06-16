#ifndef IVE_IMAGE
#define IVE_IMAGE 1

#include <osg/Image>
#include "ReadWrite.h"

namespace ive{
class Image : public osg::Image {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
