#ifndef IVE_TEXTURE3D
#define IVE_TEXTURE3D 1

#include <osg/Texture3D>
#include "ReadWrite.h"

namespace ive{
class Texture3D : public osg::Texture3D, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
