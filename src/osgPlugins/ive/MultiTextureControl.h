#ifndef IVE_MULTITEXTYRECONTROL
#define IVE_MULTITEXTYRECONTROL 1

#include <osgFX/MultiTextureControl>
#include "ReadWrite.h"

namespace ive{
class MultiTextureControl : public osgFX::MultiTextureControl {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
