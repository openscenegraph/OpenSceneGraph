// FltRecords.h


#ifndef __FLT_RECORDS_H
#define __FLT_RECORDS_H

#include "flt.h"

namespace flt {

///////////////////////////////////////////////////////////////




typedef struct     MorphingVertexListTag
{
    SRecHeader    RecHeader;
    int32    diAOffset;    // Byte offset to the actual vertex record in the vertex table.
    int32    diMOffset;    // Byte offset to the morph  vertex record in the vertex table.
} SMorphingVertexList;    // see OF doc




typedef struct ReplicateTag
{
    SRecHeader    RecHeader;
    int16    iNumber;    // Number of replications
    int16    iSpare;        // Spare for fullword alignment
} SReplicate;

/*
typedef struct ReferenceTag    // OBSOLETE
{
    SRecHeader    RecHeader;
    int16    iSpare;        // Spare
    int16    iNumber;    // Instance definition number
} SReference;


typedef struct DefinitionTag    // OBSOLETE
{
    SRecHeader    RecHeader;
    int16    iSpare;        // Spare
    int16    iNumber;    // Instance definition number
} SDefinition;
*/




/*
typedef struct ColorTableTag
{
    SRecHeader        RecHeader;
    char            szReserved[128];// Reserved
    color32        Colors[1024];    // Array of brightest RGB of color 0 - 1024
} SColorTable;

// this record is sometimes immediately followed by a: int32 numberOfColorNames
// and then the specified number of ColorNames as described in the structure below
// to check if such a list exist: compare RecHeaderBuff.wLength with sizeof(SColorTable)

typedef struct ColorNameListTag
{
    uint16    wEntryLength;
    int16    iReserved_1;
    int16    iEntryIndex;
    int16    iReserved_2;
    char    *szName;        // calc length of string from wEntryLength
} SColorNameList;
*/

/*
typedef struct ComponentTag
{
    float32    sfRed;        // red component of material
    float32    sfGreen;    // green component of material
    float32    sfBlue;        // blue component of material
} SComponent;
*/









}; // end namespace flt

#endif // __FLT_RECORDS_H

