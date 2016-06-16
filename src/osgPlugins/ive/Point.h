#ifndef IVE_POINT
#define IVE_POINT 1

#include <osg/Point>
#include "ReadWrite.h"

namespace ive{
class Point : public osg::Point {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
