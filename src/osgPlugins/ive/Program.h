#ifndef IVE_PROGRAM
#define IVE_PROGRAM 1

#include <osg/Program>
#include "ReadWrite.h"

namespace ive{
class Program : public osg::Program {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
