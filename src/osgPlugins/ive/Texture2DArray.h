#ifndef IVE_TEXTURE2DARRAY
#define IVE_TEXTURE2DARRAY 1

#include <osg/Texture2DArray>
#include "ReadWrite.h"

namespace ive
{

class Texture2DArray : public osg::Texture2DArray, public ReadWrite
{
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
