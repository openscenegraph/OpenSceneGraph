#ifndef IVE_MATERIAL
#define IVE_MATERIAL 1

#include <osg/Material>
#include "ReadWrite.h"

namespace ive{
class Material : public osg::Material, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
