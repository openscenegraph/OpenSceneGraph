// MaterialPaletteRecord.cpp

#include "flt.h"
#include "Registry.h"
#include "MaterialPaletteRecord.h"

using namespace flt;

////////////////////////////////////////////////////////////////////
//
//                       MaterialPaletteRecord
//
////////////////////////////////////////////////////////////////////

RegisterRecordProxy<MaterialPaletteRecord> g_MaterialPaletteProxy;

MaterialPaletteRecord::MaterialPaletteRecord()
{
}


// virtual
MaterialPaletteRecord::~MaterialPaletteRecord()
{
}


// virtual
void MaterialPaletteRecord::endian()
{
    SMaterial *pSMaterial = (SMaterial*)getData();

    ENDIAN( pSMaterial->diIndex );
    ENDIAN( pSMaterial->diFlags );
    pSMaterial->Ambient.endian();
    pSMaterial->Diffuse.endian();
    pSMaterial->Specular.endian();
    pSMaterial->Emissive.endian();
    ENDIAN( pSMaterial->sfShininess );
    ENDIAN( pSMaterial->sfAlpha );
}
