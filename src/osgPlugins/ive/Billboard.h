#ifndef IVE_BILLBOARD
#define IVE_BILLBOARD 1

#include <osg/Billboard>
#include "ReadWrite.h"

namespace ive{
class IVE_EXPORT Billboard : public osg::Billboard, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
