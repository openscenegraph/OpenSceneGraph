#ifndef IVE_DRAWABLE
#define IVE_DRAWABLE 1

#include <osg/Drawable>
#include "ReadWrite.h"

namespace ive{
class IVE_EXPORT Drawable : public ReadWrite, public osg::Drawable{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
