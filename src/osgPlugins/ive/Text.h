#ifndef IVE_TEXT
#define IVE_TEXT 1

#include <osgText/Text>
#include "ReadWrite.h"

namespace ive{
class Text : public osgText::Text, public ReadWrite {
public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
};
}

#endif
