#ifndef IVE_SHADER
#define IVE_SHADER 1

#include <osg/Shader>
#include "ReadWrite.h"

namespace ive{
class Shader : public osg::Shader, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
