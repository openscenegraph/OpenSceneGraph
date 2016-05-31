#ifndef IVE_POLYGONOFFSET
#define IVE_POLYGONOFFSET 1

#include <osg/PolygonOffset>
#include "ReadWrite.h"

namespace ive{
class PolygonOffset : public osg::PolygonOffset {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
