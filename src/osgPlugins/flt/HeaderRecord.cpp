// HeaderRecord.cpp

#include "osg/Group"

#include "flt.h"
#include "Registry.h"
#include "HeaderRecord.h"
#include "Input.h"

using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                          HeaderRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<HeaderRecord> g_HeaderProxy;


HeaderRecord::HeaderRecord()
{
//  _pNode = NULL;
}


// virtual
HeaderRecord::~HeaderRecord()
{
}


void HeaderRecord::endian()
{
	SHeader	*pHeader = (SHeader*)getData();

//  VALID_RECORD(SHeader, pRecHdr)
	ENDIAN( pHeader->diFormatRevLev );
	ENDIAN( pHeader->diDatabaseRevLev );
	ENDIAN( pHeader->iNextGroup );
	ENDIAN( pHeader->iNextLOD );
	ENDIAN( pHeader->iNextObject );
	ENDIAN( pHeader->iNextPolygon );
	ENDIAN( pHeader->iMultDivUnit );
	ENDIAN( pHeader->diFlags );
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
//	SHeader	*pHeader = (SHeader*)getData();

    // Add node to scene graph
//  _pNode = new osg::Node;
//  _pNode->setName(pHeader->szIdent);
}


// virtual
bool HeaderRecord::readLocalData(Input& fr)
{
    return PrimNodeRecord::readLocalData(fr);
}


// virtual
int HeaderRecord::decodeAncillary(int op)
{
/*
    switch (op)
    {
    case COLOUR_TABLE_OP:
        G_TRACE0( "\tCOLOUR_TABLE_R\n" );
        break;

    case MATERIAL_PALETTE_OP:
        {
            fltMaterialPalette	rec( _pFltFile );
            rec.readRecordBody();
            rec.decodeRecord();
        }
        break;

    case LIGHT_SOURCE_PALETTE_R:
        G_TRACE0( "\tLIGHT_SOURCE_PALETTE_R\n" );
        break;

    case VERTEX_PALETTE_R:
        {
            fltVertexPalette rec( _pFltFile );
            rec.readRecordBody();
            rec.decodeRecord();
        }
        break;

	case VERTEX_COORD_R:
        {
            fltVertex rec( _pFltFile );
            rec.readRecordBody();
            rec.decodeRecord();
        }
        break;

    case VERTEX_NORMAL_COORD_R:
        {
			fltNormalVertex rec( _pFltFile );
            rec.readRecordBody();
            rec.decodeRecord();
        }
        break;

	case VERTEX_NORMAL_UV_COORD_R:
        {
			fltNormalTextureVertex  rec( _pFltFile );
            rec.readRecordBody();
            rec.decodeRecord();
        }
        break;

	case VERTEX_UV_COORD_R:
        {
			fltTextureVertex rec( _pFltFile );
            rec.readRecordBody();
            rec.decodeRecord();
        }
        break;

    default:
        return FALSE;
    } // end-switch
*/
    return true;
}


// virtual
int HeaderRecord::decodeLevel( int op )
{
/*
    switch (op)
    {
    case GROUP_OP:
        {
            fltGroup rec( _pFltFile, (csGroup*)_pContainer );
            rec.readRecordBody();
            rec.decodeRecord();
        }
        break;
    default:
        return FALSE;
    }
*/
    return true;
}

/*
void HeaderRecord::write()
{
	SHeader	*pHeader = (SHeader*)getData();

    G_TRACE0("Header\n");
    G_TRACE1("\tFormatRevisionLevel   %ld\n", pHeader->diFormatRevLev);
    G_TRACE1("\tDatabaseRevisionLevel %ld\n", pHeader->diDatabaseRevLev);
    G_TRACE1("\tDateAndTimeOfLastRev. %s\n",  pHeader->szDaTimLastRev);
    G_TRACE1("\tProjection            %ld\n", pHeader->diProjection);
    G_TRACE1("\tDatabase Source       %ld\n", pHeader->diDatabaseSource);
    G_TRACE1("\tSWCorner,Lat          %lf\n", pHeader->SWCorner.dfLat);
    G_TRACE1("\tSWCorner,Lon          %lf\n", pHeader->SWCorner.dfLon);
    G_TRACE1("\tNECorner,Lat          %lf\n", pHeader->NECorner.dfLat);
    G_TRACE1("\tNECorner,Lon          %lf\n", pHeader->NECorner.dfLon);
    G_TRACE1("\tOrigin,Lat            %lf\n", pHeader->Origin.dfLat);
    G_TRACE1("\tOrigin,Lon            %lf\n", pHeader->Origin.dfLon);
}
*/

