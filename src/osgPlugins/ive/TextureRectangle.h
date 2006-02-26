#ifndef IVE_TEXTURERECTANGLE
#define IVE_TEXTURERECTANGLE 1

#include <osg/TextureRectangle>
#include "ReadWrite.h"

namespace ive{
class TextureRectangle : public osg::TextureRectangle, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
