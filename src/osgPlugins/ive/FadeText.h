#ifndef IVE_FADETEXT
#define IVE_FADETEXT 1

#include <osgText/FadeText>
#include "ReadWrite.h"

namespace ive{
class FadeText : public osgText::FadeText, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
