// HeaderRecord.cpp

#include "osg/Group"

#include "flt.h"
#include "Registry.h"
#include "HeaderRecord.h"
#include "Input.h"

using namespace flt;

/*
From MultiGen-Paradigm User Forum:
http://www.multigen-paradigm.com/ubb/Forum8/HTML/000035.html

Q:
Hi,
I am using the MultiGen API to read .flt files. Today I received a
.flt file with Packed color for polygons. Going through the MGAPI
documentation I didn't find anything about Packed colors in polygons
except fltPolyFlagRgbMode, which is set to "1" in that file.
Could you please answer the following questions:
1) Name of the parameter used for Packed 
primary color in the fltPolygon structure;
2) The function to read the values.
Regards, Zoia. 


A:
The short answer is: 
There is no fltPolygon attribute that defines the packed color.
You need to use mgGetPolyColorRGB and mgGetPolyAltColorRGB to 
get the primary and alternate rgb values for a polygon.

The long answer, including a bit of explanation is:

OpenFlight databases can define color values in RGB space (packed
color as you called it) or color index space. The fltHdrRgbMode 
field of the fltHeader record specifies which color space is used
by the database.

If the database is in color index mode, you can get the RGB 
colors applied to a polygon by first getting the fltPolyPrimeColor
and fltPolyPrimeIntensity attributes of the polygon and then 
converting those values to RGB by calling mgRGB2Index. 

Note: The attributes fltPolyAltColor and fltPolyAltIntensity can
be used to get the alternate color index and intensity. 

If the database is in RGB mode, you can get the RGB colors applied
to a polygon by calling the function mgGetPolyColorRGB. But if you
take a closer look at this function you will discover that it 
returns the RGB colors for a polygon regardless of whether the 
database is in color index or RGB mode.
If the database is in color index mode, mgGetPolyColorRGB acquires
the color index applied to the polygon and looks up the 
corresponding RGB values for you. 

So, if you are only after the RGB color values applied to a 
polygon, the function mgGetPolyColorRGB is always your best bet. 
Note: mgGetPolyAltColorRGB is the equivalent function to use to
get the alternate RGB color values applied to a polygon.

Hope this helps.
*/



////////////////////////////////////////////////////////////////////
//
//                          HeaderRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<HeaderRecord> g_HeaderProxy;

HeaderRecord::HeaderRecord()
{
}


// virtual
HeaderRecord::~HeaderRecord()
{
}


void HeaderRecord::endian()
{
    // OpenFlight spec dictates values that arepacked and not do not necessarily
    //   adhere to alignment rules. Copy values out of the OpenFlight packed header
    //   and into the SHeader struct and let the compiler worry about alignment
    //   issues within the struct.

    SHeader *pHeader = (SHeader*) malloc( sizeof(SHeader) );
    char* src = (char*)getData();

    // Numeric constant data sizes taken from OpenFlight spec
    memcpy( &(pHeader->RecHeader), src, 4 ); src += 4;
    memcpy( &(pHeader->szIdent), src, 8 ); src += 8;

    // Be sure to swap revision level here, since we reference it to determine
    //   which other fields to memcpy.
    memcpy( &(pHeader->diFormatRevLev), src, 4 ); src += 4;
    ENDIAN( pHeader->diFormatRevLev );

    memcpy( &(pHeader->diDatabaseRevLev), src, 4 ); src += 4;
    memcpy( &(pHeader->szDaTimLastRev), src, 32 ); src += 32;
    memcpy( &(pHeader->iNextGroup), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextLOD), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextObject), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextPolygon), src, 2 ); src += 2;
    memcpy( &(pHeader->iMultDivUnit), src, 2 ); src += 2;
    memcpy( &(pHeader->swVertexCoordUnit), src, 1 ); src += 1;
    memcpy( &(pHeader->swTexWhite), src, 1 ); src += 1;
    memcpy( &(pHeader->dwFlags), src, 4 ); src += 4;
    src += 4*6; // Reserved
    memcpy( &(pHeader->diProjection), src, 4 ); src += 4;
    src += 4*7; // Reserved
    memcpy( &(pHeader->iNextDegOfFreedom), src, 2 ); src += 2;
    memcpy( &(pHeader->iVertexStorage), src, 2 ); src += 2;
    memcpy( &(pHeader->diDatabaseSource), src, 4 ); src += 4;
    memcpy( &(pHeader->dfSWDatabaseCoordX), src, 8 ); src += 8;
    memcpy( &(pHeader->dfSWDatabaseCoordY), src, 8 ); src += 8;
    memcpy( &(pHeader->dfDatabaseOffsetX), src, 8 ); src += 8;
    memcpy( &(pHeader->dfDatabaseOffsetY), src, 8 ); src += 8;
    memcpy( &(pHeader->iNextSound), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextPath), src, 2 ); src += 2;
    src += 4*2; // Reserved
    memcpy( &(pHeader->iNextClippingRegion), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextText), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextBSP), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextSwitch), src, 2 ); src += 2;
    src += 4; // reserved
    memcpy( &(pHeader->SWCorner), src, 8*2 ); src += 8*2;
    memcpy( &(pHeader->NECorner), src, 8*2 ); src += 8*2;
    memcpy( &(pHeader->Origin), src, 8*2 ); src += 8*2;
    memcpy( &(pHeader->dfLambertUpperLat), src, 8 ); src += 8;
    memcpy( &(pHeader->dfLambertLowerLat), src, 8 ); src += 8;
    memcpy( &(pHeader->iNextLightSource), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextLightPoint), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextRoad), src, 2 ); src += 2;
    memcpy( &(pHeader->iNextCat), src, 2 ); src += 2;
    src += 2*4; // Reserved;
    memcpy( &(pHeader->diEllipsoid), src, 4 ); src += 4;

    if ( pHeader->diFormatRevLev >= 1570 )
    {
        memcpy( &(pHeader->iNextAdaptiveNodeID), src, 2 ); src += 2;
        memcpy( &(pHeader->iNextCurveNodeID), src, 2 ); src += 2;

        if ( pHeader->diFormatRevLev >= 1580 )
        {
            memcpy( &(pHeader->iUTMZone), src, 2 ); src += 2;
            src += 6; // Reserved
        }
        else
            // Must be v15.7
            src += 2; // Reserved

        memcpy( &(pHeader->dfDatabaseDeltaZ), src, 8 ); src += 8;
        memcpy( &(pHeader->dfRadius), src, 8 ); src += 8;
        memcpy( &(pHeader->iNextMeshNodeID), src, 2 ); src += 2;

        if ( pHeader->diFormatRevLev >= 1580 )
        {
            memcpy( &(pHeader->iNextLightPointSysID), src, 2 ); src += 2;
            src += 4; // Reserved
            memcpy( &(pHeader->dfEarthMajorAxis), src, 8 ); src += 8;
            memcpy( &(pHeader->dfEarthMinorAxis), src, 8 ); src += 8;
        }
        else
            // Must be v15.7
            src += 2; // Reserved
    }

    // Now that we've copied the data into SHeader, we're done with the original packed
    //   data as read out of the OpenFlight file. Free it, and replace it with the
    //   SHeader struct, so that subsequent typecasts of _pData work as expected.
    free( _pData );
    _pData = (SRecHeader*)pHeader;


    // Proceed with byteswapping
    ENDIAN( pHeader->diDatabaseRevLev );
    ENDIAN( pHeader->iNextGroup );
    ENDIAN( pHeader->iNextLOD );
    ENDIAN( pHeader->iNextObject );
    ENDIAN( pHeader->iNextPolygon );
    ENDIAN( pHeader->iMultDivUnit );
    ENDIAN( pHeader->dwFlags );
    ENDIAN( pHeader->diProjection );
    ENDIAN( pHeader->iNextDegOfFreedom );
    ENDIAN( pHeader->iVertexStorage );
    ENDIAN( pHeader->diDatabaseSource );
    ENDIAN( pHeader->dfSWDatabaseCoordX );
    ENDIAN( pHeader->dfSWDatabaseCoordY );
    ENDIAN( pHeader->dfDatabaseOffsetX );
    ENDIAN( pHeader->dfDatabaseOffsetY );
    ENDIAN( pHeader->iNextSound );
    ENDIAN( pHeader->iNextPath );
    ENDIAN( pHeader->iNextClippingRegion );
    ENDIAN( pHeader->iNextText );
    ENDIAN( pHeader->iNextBSP );
    ENDIAN( pHeader->iNextSwitch );
    pHeader->SWCorner.endian();
    pHeader->NECorner.endian();
    pHeader->Origin.endian();
    ENDIAN( pHeader->dfLambertUpperLat );
    ENDIAN( pHeader->dfLambertLowerLat );
    ENDIAN( pHeader->iNextLightSource );

    // New with 15.7
    // Note: Don't use "getFltFile()->getFlightVersion()", it's not set yet.
    if ( pHeader->diFormatRevLev >= 1570 )
    {
        ENDIAN( pHeader->iNextAdaptiveNodeID );
        ENDIAN( pHeader->iNextCurveNodeID );
        ENDIAN( pHeader->dfDatabaseDeltaZ );
        ENDIAN( pHeader->dfRadius );
        ENDIAN( pHeader->iNextMeshNodeID );

        if ( pHeader->diFormatRevLev >= 1580 )
        {
            ENDIAN( pHeader->iUTMZone );
            ENDIAN( pHeader->iNextLightPointSysID );
            ENDIAN( pHeader->dfEarthMajorAxis );
            ENDIAN( pHeader->dfEarthMinorAxis );
        }
    }
}


// virtual
void HeaderRecord::decode()
{
    // nothing done here, so commenting out, RO, March 2002.
    // SHeader    *pHeader = (SHeader*)getData();
}


// virtual
bool HeaderRecord::readLocalData(Input& fr)
{
    return PrimNodeRecord::readLocalData(fr);
}

