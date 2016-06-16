#ifndef IVE_OCCLUSIONQUERYNODE
#define IVE_OCCLUSIONQUERYNODE 1

#include <osg/OcclusionQueryNode>

#include "ReadWrite.h"

namespace ive{
class OcclusionQueryNode : public osg::OcclusionQueryNode {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
