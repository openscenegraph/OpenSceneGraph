#ifndef IVE_TEXTURECUBEMAP
#define IVE_TEXTURECUBEMAP 1

#include <osg/TextureCubeMap>
#include "ReadWrite.h"

namespace ive
{

class TextureCubeMap : public osg::TextureCubeMap, public ReadWrite
{
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
