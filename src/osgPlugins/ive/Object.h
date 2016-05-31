#ifndef IVE_OBJECT
#define IVE_OBJECT 1


#include <osg/Object>
#include "ReadWrite.h"

namespace ive{

class Object : public osg::Object{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* out);
};

}

#endif
