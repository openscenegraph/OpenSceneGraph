#ifndef IVE_VIEWPOINT
#define IVE_VIEWPOINT 1

#include <osgfIVE/ViewPoint>
#include <ive/ReadWrite.h>

namespace ive{
class IVE_EXPORT ViewPoint : public osgfIVE::ViewPoint, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
