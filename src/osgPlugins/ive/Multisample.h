#ifndef IVE_MULTISAMPLE
#define IVE_MULTISAMPLE 1

#include <osg/Multisample>
#include "ReadWrite.h"

namespace ive{
class Multisample : public osg::Multisample {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif // IVE_VERTEX_PROGRAM
