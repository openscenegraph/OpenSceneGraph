#ifndef IVE_MATRIXTRANSFORM
#define IVE_MATRIXTRANSFORM 1

#include <osg/MatrixTransform>
#include "ReadWrite.h"

namespace ive{
class IVE_EXPORT MatrixTransform : public osg::MatrixTransform, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
