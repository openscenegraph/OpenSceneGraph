#ifndef IVE_LINEWIDTH
#define IVE_LINEWIDTH 1

#include <osg/LineWidth>
#include "ReadWrite.h"

namespace ive{
class LineWidth : public osg::LineWidth, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
