#ifndef IVE_GROUP
#define IVE_GROUP 1

#include <osg/Group>
#include "ReadWrite.h"

namespace ive{
class IVE_EXPORT Group : public osg::Group, public ReadWrite{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* out);
};
}

#endif
