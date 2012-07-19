#ifndef IVE_CONVEXPLANARPOLYGON
#define IVE_CONVEXPLANARPOLYGON 1

#include <osg/ConvexPlanarPolygon>
#include "ReadWrite.h"

namespace ive{
class ConvexPlanarPolygon : public osg::ConvexPlanarPolygon, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);

        virtual ~ConvexPlanarPolygon() {}
};
}

#endif
