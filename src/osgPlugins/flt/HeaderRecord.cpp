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
    SHeader *pHeader = (SHeader*)getData();

    ENDIAN( pHeader->diFormatRevLev );
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
}


// virtual
void HeaderRecord::decode()
{
    SHeader    *pHeader = (SHeader*)getData();

}


// virtual
bool HeaderRecord::readLocalData(Input& fr)
{
    return PrimNodeRecord::readLocalData(fr);
}

