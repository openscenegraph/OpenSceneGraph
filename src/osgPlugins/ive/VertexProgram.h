#ifndef IVE_VERTEX_PROGRAM
#define IVE_VERTEX_PROGRAM 1

#include <osg/VertexProgram>
#include "ReadWrite.h"

namespace ive{
class VertexProgram : public osg::VertexProgram, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif // IVE_VERTEX_PROGRAM
