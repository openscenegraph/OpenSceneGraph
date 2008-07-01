#ifndef IVE_TEXT3D
#define IVE_TEXT3D 1

#include <osgText/Text3D>
#include "ReadWrite.h"

namespace ive{
class Text3D : public osgText::Text3D, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
