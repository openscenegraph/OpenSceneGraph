#ifndef IVE_MATRIXTRANSFORM
#define IVE_MATRIXTRANSFORM 1

#include <osg/MatrixTransform>
#include "ReadWrite.h"

namespace ive{
class MatrixTransform : public osg::MatrixTransform {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
