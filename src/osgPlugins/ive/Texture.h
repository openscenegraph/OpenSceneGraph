#ifndef IVE_TEXTURE
#define IVE_TEXTURE 1

#include <osg/Texture>
#include "ReadWrite.h"

namespace ive{
class Texture : public osg::Texture, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
