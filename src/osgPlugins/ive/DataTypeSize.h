#ifndef IVE_DATATYPESIZE
#define IVE_DATATYPESIZE 1


#define BOOLSIZE    1
#define CHARSIZE    1
#define SHORTSIZE    2
#define INTSIZE        4
#define FLOATSIZE    4
#define LONGSIZE    4
#define DOUBLESIZE    8
#define INT64SIZE    8
//Don't know where else to put this
namespace ive{

enum IncludeImageMode
{
    IMAGE_REFERENCE_FILE=0,
    IMAGE_INCLUDE_DATA,
    IMAGE_INCLUDE_FILE,
    IMAGE_COMPRESS_DATA
};


}

#endif
