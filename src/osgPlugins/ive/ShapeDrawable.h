#ifndef IVE_SHAPEDRAWABLE
#define IVE_SHAPEDRAWABLE 1

#include <osg/ShapeDrawable>
#include "ReadWrite.h"

namespace ive{
class ShapeDrawable : public osg::ShapeDrawable, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
