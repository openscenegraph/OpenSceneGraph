#ifndef IVE_GEOMETRY
#define IVE_GEOMETRY 1

#include <osg/Geometry>
#include "ReadWrite.h"

namespace ive{
class Geometry : public ReadWrite, public osg::Geometry{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
