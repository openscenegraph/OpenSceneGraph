#include <osg/Array>

using namespace osg;

static char* s_ArrayNames[] =
{
    "AttributeArray", // 0
    "ByteArray",     // 1
    "ShortArray",    // 2
    "IntArray",      // 3

    "UByteArray",    // 4
    "UShortArray",   // 5
    "UIntArray",     // 6
    "UByte4Array",   // 7

    "FloatArray",    // 8
    "Vec2Array",     // 9
    "Vec3Array",     // 10
    "Vec4Array",      // 11
};

const char* AttributeArray::className() const
{
    if (_arrayType>=AttributeArrayType && _arrayType<=Vec4ArrayType)
        return s_ArrayNames[_arrayType];
    else
        return "UnkownAttributeArray";
}

