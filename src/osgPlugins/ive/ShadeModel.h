#ifndef IVE_SHADEMODEL
#define IVE_SHADEMODEL 1

#include <osg/ShadeModel>
#include "ReadWrite.h"

namespace ive{
class ShadeModel : public osg::ShadeModel, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
