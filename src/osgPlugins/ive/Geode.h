#ifndef IVE_GEODE
#define IVE_GEODE 1

#include <osg/Geode>
#include "ReadWrite.h"

namespace ive{
class Geode : public osg::Geode{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
