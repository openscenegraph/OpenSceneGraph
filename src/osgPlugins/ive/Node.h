#ifndef IVE_NODE
#define IVE_NODE 1

#include <osg/Node>
#include "ReadWrite.h"
#include <iostream>

namespace ive{
class Node :  public osg::Node{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
