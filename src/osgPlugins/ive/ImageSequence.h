#ifndef IVE_IMAGESEQUENCE
#define IVE_IMAGESEQUENCE 1

#include <osg/ImageSequence>
#include "ReadWrite.h"

namespace ive{
class ImageSequence : public osg::ImageSequence {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
