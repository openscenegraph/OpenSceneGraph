#ifndef IVE_LOD
#define IVE_LOD 1

#include <osg/LOD>
#include "ReadWrite.h"

namespace ive{
class IVE_EXPORT LOD : public osg::LOD, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
