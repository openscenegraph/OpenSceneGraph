#ifndef IVE_SEQUENCE
#define IVE_SEQUENCE 1

#include <osg/Sequence>
#include "ReadWrite.h"

namespace ive{
class Sequence : public osg::Sequence, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
