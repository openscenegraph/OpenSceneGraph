// OldMaterialPaletteRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "OldMaterialPaletteRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       OldMaterialPaletteRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<OldMaterialPaletteRecord> g_OldMaterialPaletteProxy;

OldMaterialPaletteRecord::OldMaterialPaletteRecord()
{
}


// virtual
OldMaterialPaletteRecord::~OldMaterialPaletteRecord()
{
}


// virtual
void OldMaterialPaletteRecord::endian()
{
    SOldMaterial *pSMaterial = (SOldMaterial*)getData();

    for (int i=0; i < 64; i++)
    {
        pSMaterial->mat[i].Ambient.endian();
        pSMaterial->mat[i].Diffuse.endian();
        pSMaterial->mat[i].Specular.endian();
        pSMaterial->mat[i].Emissive.endian();
        ENDIAN( pSMaterial->mat[i].sfShininess );
        ENDIAN( pSMaterial->mat[i].sfAlpha );
        ENDIAN( pSMaterial->mat[i].diFlags );
    }
}
