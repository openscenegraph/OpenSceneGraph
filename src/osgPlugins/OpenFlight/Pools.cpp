//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <assert.h>
#include "Pools.h"
#include "Document.h"

using namespace flt;


osg::Vec4 ColorPool::getColor(int indexIntensity) const
{
    if (_old) // version <= 13
    {
        // bit 0-6:  intensity
        // bit 7-11  color index
        // bit 12    fixed intensity bit
        bool fixedIntensity = (indexIntensity & 0x1000) ? true : false;
        unsigned int index = (fixedIntensity) ? (indexIntensity & 0x0fff)+(4096>>7) : indexIntensity >> 7;
        assert(index<size());
        osg::Vec4 col = at(index);
        if (!fixedIntensity)
        {
            float intensity = (float)(indexIntensity & 0x7f)/127.f;
            col[0] *= intensity;
            col[1] *= intensity;
            col[2] *= intensity;
        }
        return col;
    }
    else // version > 13
    {
        // bit 0-6:  intensity
        // bit 7-15  color index
        int index = indexIntensity >> 7;

        if (index<0 || index>=(int)size())
        {
            // color index not available.
            return osg::Vec4(1,1,1,1);
        }

        osg::Vec4 col = at(index);
        float intensity = (float)(indexIntensity & 0x7f)/127.f;
        col[0] *= intensity;
        col[1] *= intensity;
        col[2] *= intensity;
        return col;
    }
}


MaterialPool::MaterialPool()
{
    // Default material.
    // http://www.multigen-paradigm.com/ubb/Forum1/HTML/000228.html
    _defaultMaterial = new osg::Material;
    _defaultMaterial->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(1,1,1,1));
    _defaultMaterial->setDiffuse (osg::Material::FRONT_AND_BACK,osg::Vec4(1,1,1,1));
    _defaultMaterial->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(0,0,0,1));
    _defaultMaterial->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0,0,0,1));
    _defaultMaterial->setShininess(osg::Material::FRONT_AND_BACK,0);
}



osg::Material* MaterialPool::get(int index)
{
    iterator itr = find(index);
    if (itr != end())
        return (*itr).second.get();

    // Use default material if not found in material palette.
    return _defaultMaterial.get();
}


namespace {

    osg::Vec4 finalColor(const osg::Vec4& materialColor, const osg::Vec4& faceColor)
    {
        return osg::Vec4(
            materialColor.r() * faceColor.r(),
            materialColor.g() * faceColor.g(),
            materialColor.b() * faceColor.b(),
            materialColor.a() * faceColor.a());
    }

} // end namespace

osg::Material* MaterialPool::getOrCreateMaterial(int index, const osg::Vec4& faceColor)
{
    MaterialParameters materialParam(index,faceColor);

    // Look for final material.
    FinalMaterialMap::iterator itr = _finalMaterialMap.find(materialParam);
    if (itr != _finalMaterialMap.end())
    {
        // Final material found.
        return (*itr).second.get();
    }

    osg::Material* poolMaterial = get(index);

    // Create final material.
    osg::Material* material = dynamic_cast<osg::Material*>(poolMaterial->clone(osg::CopyOp::SHALLOW_COPY));
    osg::Vec4 materialPaletteAmbient = poolMaterial->getAmbient(osg::Material::FRONT);
    osg::Vec4 materialPaletteDiffuse = poolMaterial->getDiffuse(osg::Material::FRONT);

    material->setAmbient(osg::Material::FRONT_AND_BACK, finalColor(materialPaletteAmbient, faceColor));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, finalColor(materialPaletteDiffuse, faceColor));
    material->setAlpha(osg::Material::FRONT_AND_BACK, materialPaletteAmbient.a()*faceColor.a());

    // Set final material so it can be reused.
    _finalMaterialMap[materialParam] = material;

    return material;
}
