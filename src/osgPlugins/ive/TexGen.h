#ifndef IVE_TEXGEN
#define IVE_TEXGEN 1

#include <osg/TexGen>
#include "ReadWrite.h"

namespace ive{
class TexGen : public osg::TexGen, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
