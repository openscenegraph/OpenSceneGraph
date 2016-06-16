#ifndef IVE_TEXTURE2D
#define IVE_TEXTURE2D 1

#include <osg/Texture2D>
#include "ReadWrite.h"

namespace ive{
class Texture2D : public osg::Texture2D {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
