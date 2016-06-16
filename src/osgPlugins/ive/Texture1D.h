#ifndef IVE_TEXTURE1D
#define IVE_TEXTURE1D 1

#include <osg/Texture1D>
#include "ReadWrite.h"

namespace ive{
class Texture1D : public osg::Texture1D {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
