#ifndef IVE_PAGEDLOD
#define IVE_PAGEDLOD 1

#include <osg/PagedLOD>
#include "ReadWrite.h"

namespace ive{
class PagedLOD : public osg::PagedLOD, public ReadWrite {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
