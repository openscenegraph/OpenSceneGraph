#ifndef IVE_LOD
#define IVE_LOD 1

#include <osg/LOD>
#include "ReadWrite.h"

namespace ive{
class LOD : public osg::LOD, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
