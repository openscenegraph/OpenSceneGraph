#ifndef IVE_FRAGMENT_PROGRAM
#define IVE_FRAGMENT_PROGRAM 1

#include <osg/FragmentProgram>
#include "ReadWrite.h"

namespace ive{
class FragmentProgram : public osg::FragmentProgram {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif // IVE_FRAGMENT_PROGRAM
