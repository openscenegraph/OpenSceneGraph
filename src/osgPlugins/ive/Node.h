#ifndef IVE_NODE
#define IVE_NODE 1

#include <osg/Node>
#include "ReadWrite.h"
#include <iostream>

namespace ive{
class IVE_EXPORT Node :  public osg::Node, public ReadWrite{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
