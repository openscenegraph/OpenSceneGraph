#ifndef IVE_ELLIPSOIDMODEL
#define IVE_ELLIPSOIDMODEL 1

#include <osg/CoordinateSystemNode>
#include "ReadWrite.h"

namespace ive{
class EllipsoidModel : public osg::EllipsoidModel, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
