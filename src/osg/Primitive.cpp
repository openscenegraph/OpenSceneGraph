#include <osg/Primitive>

using namespace osg;

static char* s_PrimitiveNames[] =
{
    "Primitive",                // 0
    "DrawArrays",                // 1
    "UByteDrawElements",        // 2
    "UShortDrawElements",       // 3
    "UIntDrawElements"          // 4
};

const char* Primitive::className() const
{
    if (_primitiveType>=PrimitivePrimitiveType && _primitiveType<=UIntDrawElementsPrimitiveType)
        return s_PrimitiveNames[_primitiveType];
    else
        return "UnkownAttributeArray";
}

