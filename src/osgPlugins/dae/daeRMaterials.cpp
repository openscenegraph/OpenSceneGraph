/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */

#include "daeReader.h"
#include "ReaderWriterDAE.h"

#include <dae.h>
#include <dae/daeSIDResolver.h>
#include <dae/domAny.h>
#include <dom/domCOLLADA.h>
#include <dom/domProfile_COMMON.h>
#include <dom/domConstants.h>

#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/LightModel>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <sstream>

using namespace osgDAE;

#ifdef COLLADA_DOM_2_4_OR_LATER
#include <dom/domAny.h>
using namespace ColladaDOM141;
#endif

template <typename T>
void daeReader::getTransparencyCounts(daeDatabase* database, int& zero, int& one) const
{
    std::vector<T*> constantVec;
    database->typeLookup(constantVec);

    for (size_t i = 0; i < constantVec.size(); ++i)
    {
        if (const domCommon_transparent_type* pTransparent = constantVec[i]->getTransparent())
        {
            domFx_opaque_enum opaque = pTransparent->getOpaque();
            if (opaque == FX_OPAQUE_ENUM_RGB_ZERO)
            {
                ++one;
                continue;
            }
        }

        if (const domCommon_float_or_param_type* pTransparency = constantVec[i]->getTransparency())
        {
            float transparency;

            domFloat transparencyParam = 1.0;
            if (pTransparency->getFloat())
            {
                transparency = pTransparency->getFloat()->getValue();
            }
            else if (pTransparency->getParam() &&
                GetFloatParam(pTransparency->getParam()->getRef(), transparencyParam))
            {
                transparency = transparencyParam;
            }
            else
            {
                continue;
            }

            if (transparency < 0.01f)
            {
                ++zero;
            }
            else if (transparency > 0.99f)
            {
                ++one;
            }
        }

    }

}

bool daeReader::findInvertTransparency(daeDatabase* database) const
{
    int zero = 0, one = 0;
    getTransparencyCounts<domProfile_COMMON::domTechnique::domConstant>(database, zero, one);
    getTransparencyCounts<domProfile_COMMON::domTechnique::domLambert>(database, zero, one);
    getTransparencyCounts<domProfile_COMMON::domTechnique::domPhong>(database, zero, one);
    getTransparencyCounts<domProfile_COMMON::domTechnique::domBlinn>(database, zero, one);

    return zero > one;
}

// <bind_material>
// elements:
// 0..*    <param>
//        name
//        sid
//        semantic
//        type
// 1    <technique_common>
//        0..*    <instance_material>
//                symbol
//                target
//                sid
//                name
// 0..*    <technique>
//        profile
// 0..* <extra>
//        id
//        name
//        type
void daeReader::processBindMaterial( domBind_material *bm, domGeometry *geom, osg::Geode *geode, osg::Geode *cachedGeode )
{
    if (bm->getTechnique_common() == NULL )
    {
        OSG_WARN << "No COMMON technique for bind_material" << std::endl;
        return;
    }

    for (size_t i =0; i < geode->getNumDrawables(); i++)
    {
        osg::Drawable* drawable = geode->getDrawable(i);
        std::string materialName = drawable->getName();
        osg::Geometry *cachedGeometry = dynamic_cast<osg::Geometry*>(cachedGeode->getDrawable(i)->asGeometry());

        domInstance_material_Array &ima = bm->getTechnique_common()->getInstance_material_array();
        std::string symbol;
        bool found = false;
        for ( size_t j = 0; j < ima.getCount(); j++)
        {
            symbol = ima[j]->getSymbol();
            if (symbol.compare(materialName) == 0)
            {
                found = true;
                domMaterial *mat = daeSafeCast< domMaterial >(getElementFromURI( ima[j]->getTarget()));
                if (mat)
                {
                    // Check material cache if this material already exists
                    osg::StateSet* ss;
                    domMaterialStateSetMap::iterator iter = _materialMap.find( mat );
                    if (iter != _materialMap.end() )
                    {
                        // Reuse material
                        ss = iter->second.get();
                    }
                    else
                    {
                        // Create new material
                        ss = new osg::StateSet;
                        processMaterial(ss, mat);
                        _materialMap.insert(std::make_pair(mat, ss));
                    }
                    drawable->setStateSet(ss);
                    // Need to process bind_vertex_inputs here
                    // 1. Clear the texcoord arrays and associated texcoord vertex indices
                    // from the current (cloned) drawable.
                    osg::Geometry *clonedGeometry = drawable->asGeometry();
                    if (NULL == clonedGeometry)
                    {
                        OSG_WARN << "Failed to convert drawable to geometry object" << std::endl;
                        break;
                    }
                    clonedGeometry->getTexCoordArrayList().clear();

                    // 2. For each possible texture unit find the correct texcoord array and
                    // indices from the cached drawable and place in the cloned drawable
                    // in the correct texture unit slot
                    unsigned int textureUnit(0);
                    if (copyTextureCoordinateSet(ss, cachedGeometry, clonedGeometry, ima[j], AMBIENT_OCCLUSION_UNIT, textureUnit)) ++textureUnit;
                    if (copyTextureCoordinateSet(ss, cachedGeometry, clonedGeometry, ima[j], MAIN_TEXTURE_UNIT     , textureUnit)) ++textureUnit;
                    if (copyTextureCoordinateSet(ss, cachedGeometry, clonedGeometry, ima[j], TRANSPARENCY_MAP_UNIT , textureUnit)) ++textureUnit;
                }
                else
                {
                    OSG_WARN << "Failed to locate <material> with id " << ima[i]->getTarget().getURI() << std::endl;
                }

                break;
            }
        }
        if (!found)
        {
            OSG_WARN << "Failed to locate <instance_material> with symbol " << materialName << std::endl;
        }
    }
}

// <material>
// attributes:
// 0..1    id
// 0..1    name
// elements:
// 0..1 <asset>
// 1    <instance_effect>
// 0..* <extra>
void    daeReader::processMaterial(osg::StateSet *ss, domMaterial *mat )
{
    if (!mat)
    {
        return;
    }
    if (mat->getName()) {
        ss->setName(mat->getName());
    }
    _currentInstance_effect = mat->getInstance_effect();
    if (!_currentInstance_effect)
    {
        return;
    }
    domEffect *effect = daeSafeCast< domEffect >( getElementFromURI( _currentInstance_effect->getUrl() ) );
    if (effect)
    {
        processEffect(ss, effect);

        //TODO: process all of the setParams that could happen here in the material. ESP. the textures
    }
    else
    {
        OSG_WARN << "Failed to locate effect " << mat->getInstance_effect()->getUrl().getURI() << std::endl;
    }
}

// <effect>
// attributes:
// 1    id
// 0..1    name
// elements:
// 0..1 <asset>
// 0..* <annotate>
// 0..* <image>
// 0..* <newparam>
// 1..*    <fx_profile_abstract>
// 0..* <extra>
void daeReader::processEffect(osg::StateSet *ss, domEffect *effect )
{
    bool hasCOMMON = false;

    for ( size_t i = 0; i < effect->getFx_profile_abstract_array().getCount(); i++ )
    {
        domProfile_COMMON *pc = daeSafeCast< domProfile_COMMON >( effect->getFx_profile_abstract_array()[i] );
        if (pc != NULL )
        {
            if (hasCOMMON )
            {
                OSG_WARN << "Effect already has a profile_COMMON. Skipping this one" << std::endl;
                continue;
            }
            _currentEffect = effect;
            processProfileCOMMON(ss, pc);
            hasCOMMON = true;
            continue;
        }

        OSG_WARN << "unsupported effect profile " << effect->getFx_profile_abstract_array()[i]->getTypeName() << std::endl;
    }
}

// <profile_COMMON>
// elements:
// 0..* <image>, <newparam>
// 1    <technique>
//        attributes:
//        elements:
//        0..1    <asset>
//        0..*    <image>, <newparam>
//        1        <constant>, <lambert>, <phong>, <blinn>
//        0..*    <extra>
// 0..* <extra>
void daeReader::processProfileCOMMON(osg::StateSet *ss, domProfile_COMMON *pc )
{
    domProfile_COMMON::domTechnique *teq = pc->getTechnique();

    domProfile_COMMON::domTechnique::domConstant *c = teq->getConstant();
    domProfile_COMMON::domTechnique::domLambert *l = teq->getLambert();
    domProfile_COMMON::domTechnique::domPhong *p = teq->getPhong();
    domProfile_COMMON::domTechnique::domBlinn *b = teq->getBlinn();

    ss->setMode( GL_CULL_FACE, osg::StateAttribute::ON ); // Cull Back faces

    // See if there are any extra's that are supported by OpenSceneGraph
    const domExtra_Array& ExtraArray = pc->getExtra_array();
    size_t NumberOfExtras = ExtraArray.getCount();
    size_t CurrentExtra;
    for (CurrentExtra = 0; CurrentExtra < NumberOfExtras; CurrentExtra++)
    {
        const domTechnique_Array& TechniqueArray = ExtraArray[CurrentExtra]->getTechnique_array();
        size_t NumberOfTechniques = TechniqueArray.getCount();
        size_t CurrentTechnique;
        for (CurrentTechnique = 0; CurrentTechnique < NumberOfTechniques; CurrentTechnique++)
        {
            //  <technique profile="GOOGLEEARTH">
            //      <double_sided>0</double_sided>
            //  </technique>
            const domTechniqueRef& TechniqueRef = TechniqueArray[CurrentTechnique];
            if (TechniqueRef->getProfile() && strcmp(TechniqueRef->getProfile(), "GOOGLEEARTH") == 0)
            {
                const daeElementRefArray& ElementArray = TechniqueRef->getContents();
                size_t NumberOfElements = ElementArray.getCount();
                size_t CurrentElement;
                for (CurrentElement = 0; CurrentElement < NumberOfElements; CurrentElement++)
                {
                    domAny* pAny = (domAny*)ElementArray[CurrentElement].cast();
                    if (strcmp(pAny->getElementName(), "double_sided") == 0)
                    {
                        daeString Value = pAny->getValue();
                        if (Value && strcmp(Value, "1") == 0)
                            ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
                    }
                }
            }
        }
    }

    osg::ref_ptr< osg::Material > mat = new osg::Material();
    // <blinn>
    // elements:
    // 0..1 <emission>
    // 0..1 <ambient>
    // 0..1 <diffuse>
    // 0..1 <specular>
    // 0..1 <shininess>
    // 0..1 <reflective>
    // 0..1 <reflectivity>
    // 0..1 <transparent>
    // 0..1 <transparency>
    // 0..1 <index_of_refraction>
    if (b != NULL )
    {
        osg::Texture2D *EmissionStateAttribute = NULL;
        osg::Texture2D *AmbientStateAttribute = NULL;
        osg::Texture2D *DiffuseStateAttribute = NULL;
        processColorOrTextureType(ss, b->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &EmissionStateAttribute );
        if (NULL != EmissionStateAttribute)
            OSG_WARN << "Currently no support for <texture> in Emission channel " << std::endl;

        processColorOrTextureType(ss, b->getAmbient(), osg::Material::AMBIENT, mat.get(), NULL,  &AmbientStateAttribute );

        processColorOrTextureType(ss, b->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &DiffuseStateAttribute );
        if (DiffuseStateAttribute != NULL )
        {
            if (AmbientStateAttribute != NULL )
            {
                // Set the ambient and diffuse colour white so that the incoming fragment colour ends up as a
                // lit white colour. I modulate both textures onto this to approximate the lighting equation.
                // Using a zero diffuse and then an ADD of the diffuse texture seems overlit to me.
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                // Use the ambient texture map as an occlusion map.
                unsigned int textureUnit( _pluginOptions.usePredefinedTextureUnits ? AMBIENT_OCCLUSION_UNIT : 0);
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, AmbientStateAttribute );
                // Modulate in the diffuse texture
                textureUnit = _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 1;
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, DiffuseStateAttribute );
            }
            else
            {
                // Set the diffuse colour white so that the incoming fragment colour ends up as the global diffuse lighting colour
                // plus any constant ambient contribution after the lighting calculation. This means that I am modulating the the
                // ambient with the texture as well but I cannot see a way of avoiding that.
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                unsigned int textureUnit( _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0);
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, DiffuseStateAttribute );
            }
        }
        else
        {
            if (NULL != AmbientStateAttribute  )
                OSG_WARN << "Ambient occlusion map only supported when diffuse texture also specified" << std::endl;
        }

        if (processColorOrTextureType(ss, b->getSpecular(), osg::Material::SPECULAR, mat.get(), b->getShininess() ) && (NULL != DiffuseStateAttribute) )
        {
            // Diffuse texture will defeat specular highlighting
            // So postpone specular - Not sure if I should do this here
            // because it will override any global light model states
            osg::LightModel* lightmodel = new osg::LightModel;
            lightmodel->setColorControl(osg::LightModel::SEPARATE_SPECULAR_COLOR);
            ss->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);
        }

        processTransparencySettings(b->getTransparent(), b->getTransparency(), ss, mat.get(), _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0 );
    }
    // <phong>
    // elements:
    // 0..1 <emission>
    // 0..1 <ambient>
    // 0..1 <diffuse>
    // 0..1 <specular>
    // 0..1 <shininess>
    // 0..1 <reflective>
    // 0..1 <reflectivity>
    // 0..1 <transparent>
    // 0..1 <transparency>
    // 0..1 <index_of_refraction>
    else if (p != NULL )
    {
        osg::Texture2D *EmissionStateAttribute = NULL;
        osg::Texture2D *AmbientStateAttribute = NULL;
        osg::Texture2D *DiffuseStateAttribute = NULL;
        processColorOrTextureType(ss, p->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &EmissionStateAttribute );
        if (NULL != EmissionStateAttribute)
            OSG_WARN << "Currently no support for <texture> in Emission channel " << std::endl;

        processColorOrTextureType(ss, p->getAmbient(), osg::Material::AMBIENT, mat.get(), NULL,  &AmbientStateAttribute );

        processColorOrTextureType(ss, p->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &DiffuseStateAttribute );
        if (DiffuseStateAttribute != NULL )
        {
            if (AmbientStateAttribute != NULL )
            {
                // Set the ambient and diffuse colour white so that the incoming fragment colour ends up as a
                // lit white colour. I modulate both textures onto this to approximate the lighting equation.
                // Using a zero diffuse and then an ADD of the diffuse texture seems overlit to me.
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                // Use the ambient texture map as an occlusion map.
                unsigned int textureUnit( _pluginOptions.usePredefinedTextureUnits ? AMBIENT_OCCLUSION_UNIT : 0);
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, AmbientStateAttribute );
                // Modulate in the diffuse texture
                textureUnit = _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 1;
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, DiffuseStateAttribute );
            }
            else
            {
                // Set the diffuse colour white so that the incoming fragment colour ends up as the global diffuse lighting colour
                // plus any constant ambient contribution after the lighting calculation. This means that I am modulating the the
                // ambient with the texture as well but I cannot see a way of avoiding that.
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                unsigned int textureUnit( _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0);
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, DiffuseStateAttribute );
            }
        }
        else
        {
            if (NULL != AmbientStateAttribute  )
                OSG_WARN << "Ambient occlusion map only supported when diffuse texture also specified" << std::endl;
        }

        if (processColorOrTextureType(ss, p->getSpecular(), osg::Material::SPECULAR, mat.get(), p->getShininess() ) && (NULL != DiffuseStateAttribute) )
        {
            // Diffuse texture will defeat specular highlighting
            // So postpone specular - Not sure if I should do this here
            // because it will override any global light model states
            osg::LightModel* lightmodel = new osg::LightModel;
            lightmodel->setColorControl(osg::LightModel::SEPARATE_SPECULAR_COLOR);
            ss->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);
        }

        processTransparencySettings(p->getTransparent(), p->getTransparency(), ss, mat.get(), _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0 );
    }
    // <lambert>
    // elements:
    // 0..1 <emission>
    // 0..1 <ambient>
    // 0..1 <diffuse>
    // 0..1 <reflective>
    // 0..1 <reflectivity>
    // 0..1 <transparent>
    // 0..1 <transparency>
    // 0..1 <index_of_refraction>
    else if (l != NULL )
    {
        osg::Texture2D *EmissionStateAttribute = NULL;
        osg::Texture2D *AmbientStateAttribute = NULL;
        osg::Texture2D *DiffuseStateAttribute = NULL;
        processColorOrTextureType(ss, l->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &EmissionStateAttribute );
        if (NULL != EmissionStateAttribute)
            OSG_WARN << "Currently no support for <texture> in Emission channel " << std::endl;

        processColorOrTextureType(ss, l->getAmbient(), osg::Material::AMBIENT, mat.get(), NULL,  &AmbientStateAttribute);

        processColorOrTextureType(ss, l->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &DiffuseStateAttribute );
        if (DiffuseStateAttribute != NULL )
        {
            if (AmbientStateAttribute != NULL )
            {
                // Set the ambient and diffuse colour white so that the incoming fragment colour ends up as a
                // lit white colour. I modulate both textures onto this to approximate the lighting equation.
                // Using a zero diffuse and then an ADD of the diffuse texture seems overlit to me.
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                // Use the ambient texture map as an occlusion map.
                unsigned int textureUnit( _pluginOptions.usePredefinedTextureUnits ? AMBIENT_OCCLUSION_UNIT : 0);
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, AmbientStateAttribute );
                // Modulate in the diffuse texture
                textureUnit = _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 1;
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, DiffuseStateAttribute );
            }
            else
            {
                // Set the diffuse colour white so that the incoming fragment colour ends up as the global diffuse lighting colour
                // plus any constant ambient contribution after the lighting calculation. This means that I am modulating the the
                // ambient with the texture as well but I cannot see a way of avoiding that.
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                unsigned int textureUnit( _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0);
                ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( textureUnit, DiffuseStateAttribute );
            }
        }
        else
        {
            if (NULL != AmbientStateAttribute  )
                OSG_WARN << "Ambient occlusion map only supported when diffuse texture also specified" << std::endl;
        }

        processTransparencySettings(l->getTransparent(), l->getTransparency(), ss, mat.get(), _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0 );
    }
    // <constant>
    // elements:
    // 0..1 <emission>
    // 0..1 <reflective>
    // 0..1 <reflectivity>
    // 0..1 <transparent>
    // 0..1 <transparency>
    // 0..1 <index_of_refraction>
    else if (c != NULL )
    {
        osg::Texture2D *sa = NULL;
        processColorOrTextureType(ss, c->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &sa );
        if (sa != NULL )
        {
            unsigned int textureUnit( _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0);
            ss->setTextureMode( textureUnit, GL_TEXTURE_2D, GL_TRUE );
            ss->setTextureAttribute( textureUnit, new osg::TexEnv(osg::TexEnv::REPLACE) );
            ss->setTextureAttribute( textureUnit, sa );
        }

        // Use the emission colour as the main colour in transparency calculations
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, mat->getEmission(osg::Material::FRONT_AND_BACK));

        processTransparencySettings(c->getTransparent(), c->getTransparency(), ss, mat.get(), _pluginOptions.usePredefinedTextureUnits ? MAIN_TEXTURE_UNIT : 0 );

        // Kill the lighting
        mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.0, 0.0, 1.0));
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.0, 0.0, 1.0));
    }
    ss->setAttribute( mat.get() );
}

// colorOrTexture
// 1  of
//         <color>
//        <param>
//        attributes:
//        1        ref
//        <texture>
//        attributes:
//        1        texture
//        1        texcoord
//        0..*    extra
bool daeReader::processColorOrTextureType(const osg::StateSet* ss,
                                          domCommon_color_or_texture_type *cot,
                                          osg::Material::ColorMode channel,
                                          osg::Material *mat,
                                          domCommon_float_or_param_type *fop,
                                          osg::Texture2D **sa,
                                          bool blinn)
{
    if (cot == NULL )
    {
        return false;
    }
    bool retVal = false;

    std::string texCoordSet;

    //osg::StateAttribute *sa = NULL;
    //TODO: Make all channels process <param ref=""> type of value
    if (channel == osg::Material::EMISSION )
    {
        if (cot->getColor() != NULL )
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
            retVal = true;
        }
        else if (cot->getParam() != NULL)
        {
            domFloat4 f4;
            if (GetFloat4Param(cot->getParam()->getRef(), f4))
            {
                mat->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
                retVal = true;
            }
        }
        else if (cot->getTexture() != NULL)
        {
            if (sa != NULL)
            {
                *sa = processTexture( cot->getTexture(), ss, MAIN_TEXTURE_UNIT);
                retVal = true;
            }
            else
                OSG_WARN << "Currently no support for <texture> in Emission channel " << std::endl;
        }
        else
        {
            OSG_WARN << "Missing <color>, <param> or <texture> in Emission channel " << std::endl;
        }
    }
    else if (channel == osg::Material::AMBIENT )
    {
        if (cot->getColor() != NULL )
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
            retVal = true;
        }
        else if (cot->getParam() != NULL)
        {
            domFloat4 f4;
            if (cot->getParam()->getRef() != 0 && GetFloat4Param(cot->getParam()->getRef(), f4))
            {
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
                retVal = true;
            }
        }
        else if (cot->getTexture() != NULL)
        {
            if (sa != NULL)
                *sa = processTexture( cot->getTexture(), ss, AMBIENT_OCCLUSION_UNIT);
            else
            {
                OSG_WARN << "Currently no support for <texture> in Ambient channel " << std::endl;
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
            }
            retVal = true;
       }
        else
        {
            OSG_WARN << "Missing <color>, <param> or <texture> in Ambient channel " << std::endl;
        }
    }
    else if (channel == osg::Material::DIFFUSE )
    {
        if (cot->getColor() != NULL)
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
            retVal = true;
        }
        else if (cot->getTexture() != NULL)
        {
            if (sa != NULL)
                *sa = processTexture( cot->getTexture(), ss, MAIN_TEXTURE_UNIT);
            else
            {
                OSG_WARN << "Currently no support for <texture> in Diffuse channel " << std::endl;
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.8f, 0.8f, 0.8f, 1.0f ) );
            }
            domExtra *extra = cot->getTexture()->getExtra();
            if (extra != NULL && extra->getType() != NULL && strcmp( extra->getType(), "color" ) == 0 )
            {
                //the extra data for osg. Diffuse color can happen with a texture.
                for ( unsigned int i = 0; i < extra->getTechnique_array().getCount(); i++ )
                {
                    domTechnique *teq = extra->getTechnique_array()[i];
                    if (strcmp( teq->getProfile(), "SCEI" ) == 0 )
                    {
                        osg::Vec4 col;
                        domAny *dcol = (domAny*)(daeElement*)teq->getContents()[0];
                        std::istringstream diffuse_colour((const char *)dcol->getValue());
                        diffuse_colour >> col.r() >> col.g() >> col.b() >> col.a();
                        mat->setDiffuse( osg::Material::FRONT_AND_BACK, col );
                        break;
                    }
                }
            }
            retVal = true;
        }
        else if (cot->getParam() != NULL)
        {
            domFloat4 f4;
            if (GetFloat4Param(cot->getParam()->getRef(), f4))
            {
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
                retVal = true;
            }
        }
        else
        {
            OSG_WARN << "Missing <color>, <param> or <texture> in Diffuse channel " << std::endl;
        }
    }
    else if (channel == osg::Material::SPECULAR )
    {
        if (cot->getColor() != NULL )
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], (f4.getCount()==4)? f4[3] : 1.0 ) );
            retVal = true;
        }
        else if (cot->getParam() != NULL)
        {
            domFloat4 f4;
            if (GetFloat4Param(cot->getParam()->getRef(), f4))
            {
                mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
                retVal = true;
            }
        }
        else if (cot->getTexture() != NULL)
        {
            OSG_WARN << "Currently no support for <texture> in Specular channel " << std::endl;
        }
        else
        {
            OSG_WARN << "Missing <color>, <param> or <texture> in Specular channel " << std::endl;
        }

        if (fop != NULL && fop->getFloat() != NULL )
        {
            float shininess = fop->getFloat()->getValue();
            if (blinn)
            {
                // If the blinn mode is in the range [0,1] rescale it to [0,128]
                if (shininess < 1)
                    shininess *= 128.0f;
            }
            mat->setShininess( osg::Material::FRONT_AND_BACK, shininess );
        }
    }

    return retVal;
}

bool daeReader::GetFloat4Param(xsNCName Reference, domFloat4 &f4) const
{
    std::string MyReference = Reference;

    MyReference.insert(0, "./");
    daeSIDResolver Resolver(_currentEffect, MyReference.c_str());
    daeElement *el = Resolver.getElement();
    if (NULL == el)
            return false;

    if (NULL != _currentInstance_effect)
    {
        // look here first for setparams
        // I am sure there must be a better way of doing this
        // Maybe the Collada DAE guys can give us a parameter management mechanism !
        const domInstance_effect::domSetparam_Array& SetParamArray = _currentInstance_effect->getSetparam_array();
        size_t NumberOfSetParams = SetParamArray.getCount();
        for (size_t i = 0; i < NumberOfSetParams; i++)
        {
            // Just do a simple comaprison of the ref strings for the time being
            if (0 == strcmp(SetParamArray[i]->getRef(), Reference))
            {
                if (NULL != SetParamArray[i]->getFx_basic_type_common() && (NULL != SetParamArray[i]->getFx_basic_type_common()->getFloat4()))
                {
                    f4 = SetParamArray[i]->getFx_basic_type_common()->getFloat4()->getValue();
                    return true;
                }
            }
        }
    }

    domCommon_newparam_type *cnp = daeSafeCast< domCommon_newparam_type >( el );
    domFx_newparam_common *npc = daeSafeCast< domFx_newparam_common >( el );
    if ((cnp != NULL) && (NULL != cnp->getFloat4()))
    {
        f4 = cnp->getFloat4()->getValue();
        return true;
    }
    else if ((npc != NULL) && (NULL != npc->getFx_basic_type_common()) && (NULL != npc->getFx_basic_type_common()->getFloat4()))
    {
        f4 = npc->getFx_basic_type_common()->getFloat4()->getValue();
        return true;
    }
    else
        return false;
}

bool daeReader::GetFloatParam(xsNCName Reference, domFloat &f) const
{
    std::string MyReference = Reference;

    MyReference.insert(0, "./");
    daeSIDResolver Resolver(_currentEffect, MyReference.c_str());
    daeElement *el = Resolver.getElement();
    if (NULL == el)
        return false;

    if (NULL != _currentInstance_effect)
    {
        // look here first for setparams
        // I am sure there must be a better way of doing this
        // Maybe the Collada DAE guys can give us a parameter management mechanism !
        const domInstance_effect::domSetparam_Array& SetParamArray = _currentInstance_effect->getSetparam_array();
        size_t NumberOfSetParams = SetParamArray.getCount();
        for (size_t i = 0; i < NumberOfSetParams; i++)
        {
            // Just do a simple comaprison of the ref strings for the time being
            if (0 == strcmp(SetParamArray[i]->getRef(), Reference))
            {
                if (NULL != SetParamArray[i]->getFx_basic_type_common() && (NULL != SetParamArray[i]->getFx_basic_type_common()->getFloat()))
                {
                    f = SetParamArray[i]->getFx_basic_type_common()->getFloat()->getValue();
                    return true;
                }
            }
        }
    }

    domCommon_newparam_type *cnp = daeSafeCast< domCommon_newparam_type >( el );
    domFx_newparam_common *npc = daeSafeCast< domFx_newparam_common >( el );
    if ((cnp != NULL) && (NULL != cnp->getFloat()))
    {
        f = cnp->getFloat()->getValue();
        return true;
    }
    else if ((npc != NULL) && (NULL != npc->getFx_basic_type_common()) && (NULL != npc->getFx_basic_type_common()->getFloat()))
    {
        f = npc->getFx_basic_type_common()->getFloat()->getValue();
        return true;
    }
    else
        return false;
}

osg::Texture::WrapMode getWrapMode(domFx_sampler_wrap_common domWrap)
{
    switch (domWrap)
    {
    case FX_SAMPLER_WRAP_COMMON_WRAP:
        return osg::Texture::REPEAT;
    case FX_SAMPLER_WRAP_COMMON_MIRROR:
        return osg::Texture::MIRROR;
    case FX_SAMPLER_WRAP_COMMON_CLAMP:
        return osg::Texture::CLAMP_TO_EDGE;
    case FX_SAMPLER_WRAP_COMMON_NONE:
    case FX_SAMPLER_WRAP_COMMON_BORDER:
        return osg::Texture::CLAMP_TO_BORDER;
    default:
        OSG_WARN << "Unrecognised domFx_sampler_wrap_common." << std::endl;
    }
    return osg::Texture::CLAMP;
}

osg::Texture::FilterMode getFilterMode(domFx_sampler_filter_common domFilter, bool allowMipMap)
{
    switch (domFilter)
    {
    case FX_SAMPLER_FILTER_COMMON_NONE:
    case FX_SAMPLER_FILTER_COMMON_NEAREST:
        return osg::Texture::NEAREST;
    case FX_SAMPLER_FILTER_COMMON_LINEAR:
        return osg::Texture::LINEAR;
    case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_NEAREST:
        return allowMipMap ? osg::Texture::NEAREST_MIPMAP_NEAREST : osg::Texture::NEAREST;
    case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_NEAREST:
        return allowMipMap ? osg::Texture::LINEAR_MIPMAP_NEAREST : osg::Texture::LINEAR;
    case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_LINEAR:
        return allowMipMap ? osg::Texture::NEAREST_MIPMAP_LINEAR : osg::Texture::NEAREST;
    case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR:
        return allowMipMap ? osg::Texture::LINEAR_MIPMAP_LINEAR : osg::Texture::LINEAR;
    default:
        OSG_WARN << "Unrecognised domFx_sampler_filter_common." << std::endl;
    }
    return osg::Texture::LINEAR;
}

std::string daeReader::processImagePath(const domImage* pDomImage) const
{
    if (pDomImage == NULL)
    {
        OSG_WARN << "Could not locate image for texture" << std::endl;
        return std::string();
    }

    //Got a sampler and a surface and an imaged. Time to create the texture stuff for osg
    if (pDomImage->getInit_from())
    {
        pDomImage->getInit_from()->getValue().validate();
        if (strcmp(pDomImage->getInit_from()->getValue().getProtocol(), "file") == 0)
        {
            std::string path = pDomImage->getInit_from()->getValue().pathDir() +
                pDomImage->getInit_from()->getValue().pathFile();
            path = ReaderWriterDAE::ConvertColladaCompatibleURIToFilePath(path);
            if (path.empty())
            {
                OSG_WARN << "Unable to get path from URI." << std::endl;
                return std::string();
            }
#ifdef WIN32
            // If the path has a drive specifier or a UNC name then strip the leading /
            if (path.size() > 2 && (path[2] == ':' || (path[1] == '/' && path[2] == '/')))
                return path.substr(1, std::string::npos);
#endif
            return path;
        }
        else
        {
            OSG_WARN << "Only images with a \"file\" scheme URI are supported in this version." << std::endl;
        }
    }
    else
    {
        OSG_WARN << "Embedded image data is not supported in this version." << std::endl;
    }
    return std::string();
}

float luminance(const osg::Vec4& color)
{
    return
        color.r() * 0.212671f +
        color.g() * 0.715160f +
        color.b() * 0.072169f;
}

osg::Image* daeReader::processImageTransparency(const osg::Image* srcImg, domFx_opaque_enum opaque, float transparency) const
{
    int s = srcImg->s();
    int t = srcImg->t();
    unsigned char* pixels = new unsigned char [s * t];

    if (opaque == FX_OPAQUE_ENUM_RGB_ZERO)
    {
        for (int i = 0; i < t; ++i)
        {
            for (int j = 0; j < s; ++j)
            {
                osg::Vec4 color(srcImg->getColor(j, i));

                pixels[i * s + j] = static_cast<unsigned char>(
                    (1.0f - luminance(color) * transparency) * 255.0f);
            }
        }
    }
    else
    {
        bool texHasAlpha = false;
        switch (srcImg->getPixelFormat())
        {
        case GL_ALPHA:
        case GL_LUMINANCE_ALPHA:
        case GL_RGBA:
        case GL_BGRA:
            texHasAlpha = true;
        }

        if (texHasAlpha)
        {
            for (int i = 0; i < t; ++i)
            {
                for (int j = 0; j < s; ++j)
                {
                    osg::Vec4 color(srcImg->getColor(j, i));

                    pixels[i * s + j] = static_cast<unsigned char>(
                        color.a() * transparency * 255.0f);
                }
            }
        }
        else
        {
            for (int i = 0; i < t; ++i)
            {
                for (int j = 0; j < s; ++j)
                {
                    osg::Vec4 color(srcImg->getColor(j, i));

                    pixels[i * s + j] = static_cast<unsigned char>(
                        luminance(color) * transparency * 255.0f);
                }
            }
        }
    }

    osg::Image* transparentImage = new osg::Image;
    transparentImage->setWriteHint(osg::Image::STORE_INLINE);
    transparentImage->setImage(s, t, 1, GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, pixels, osg::Image::USE_NEW_DELETE);

    return transparentImage;
}

osg::Texture2D* daeReader::processTexture(
    domCommon_color_or_texture_type_complexType::domTexture *tex,
    const osg::StateSet* ss, TextureUnitUsage tuu,
    domFx_opaque_enum opaque, float transparency)
{
    TextureParameters parameters;
    parameters.transparent = tuu == TRANSPARENCY_MAP_UNIT;
    parameters.opaque = opaque;
    parameters.transparency = transparency;

    //find the newparam for the sampler based on the texture attribute
    domFx_sampler2D_common *sampler = NULL;
    domFx_surface_common *surface = NULL;
    domImage *dImg = NULL;

    std::string target = std::string("./") + std::string(tex->getTexture());
    OSG_INFO<<"processTexture("<<target<<")"<<std::endl;

    daeSIDResolver res1( _currentEffect, target.c_str() );
    daeElement *el = res1.getElement();

    if (el == NULL )
    {
        OSG_WARN << "Could not locate newparam for texture sampler2D \"" << tex->getTexture() <<
            "\". Checking if data does incorrect linking straight to the image" << std::endl;
        _dae->getDatabase()->getElement( (daeElement**)&dImg, 0, tex->getTexture(), "image" );
        if (dImg != NULL )
        {
            OSG_WARN << "Direct image link found. Data is incorrect but will continue to load texture" << std::endl;
        }
    }
    else
    {
        domCommon_newparam_type *cnp = daeSafeCast< domCommon_newparam_type >( el );
        domFx_newparam_common *npc = daeSafeCast< domFx_newparam_common >( el );

        if (cnp != NULL )
        {
            sampler = cnp->getSampler2D();
        }
        else if (npc != NULL )
        {
            sampler = npc->getFx_basic_type_common()->getSampler2D();
        }

        if (sampler == NULL )
        {
            OSG_WARN << "Wrong newparam type. Expected sampler2D" << std::endl;
            return NULL;
        }

        if (sampler->getSource() == NULL || sampler->getSource()->getValue() == NULL)
        {
            OSG_WARN << "Could not locate source for sampler2D" << std::endl;
            return NULL;
        }

        //find the newparam for the surface based on the sampler2D->source value
        target = std::string("./") + std::string( sampler->getSource()->getValue() );
        daeSIDResolver res2( _currentEffect, target.c_str() );
        el = res2.getElement();
        if (el == NULL )
        {
            OSG_WARN << "Could not locate newparam for source " << sampler->getSource()->getValue() << std::endl;
            return NULL;
        }
        cnp = daeSafeCast< domCommon_newparam_type >( el );
        npc = daeSafeCast< domFx_newparam_common >( el );

        if (cnp != NULL )
        {
            surface = cnp->getSurface();
        }
        else if (npc != NULL )
        {
            surface = npc->getFx_basic_type_common()->getSurface();
        }

        if (surface == NULL )
        {
            OSG_WARN << "Wrong newparam type. Expected surface" << std::endl;
            return NULL;
        }

        //look for the domImage based on the surface initialization stuff
        daeIDRef &ref = surface->getFx_surface_init_common()->getInit_from_array()[0]->getValue();
        dImg = daeSafeCast< domImage >( getElementFromIDRef( ref ) );
    }

    parameters.filename = processImagePath(dImg);
    if (parameters.filename.empty())
    {
        return NULL;
    }

    //set texture parameters
    if (sampler)
    {
        if (sampler->getWrap_s())
        {
            parameters.wrap_s = getWrapMode(sampler->getWrap_s()->getValue());
        }
        if (sampler->getWrap_t())
        {
            parameters.wrap_t = getWrapMode(sampler->getWrap_s()->getValue());
        }

        if (sampler->getMinfilter() && sampler->getMinfilter()->getValue() != FX_SAMPLER_FILTER_COMMON_NONE)
        {
            parameters.filter_min = getFilterMode(sampler->getMinfilter()->getValue(), true);
        }
        if (sampler->getMagfilter() && sampler->getMagfilter()->getValue() != FX_SAMPLER_FILTER_COMMON_NONE)
        {
            parameters.filter_mag = getFilterMode(sampler->getMagfilter()->getValue(), false);
        }

        if (sampler->getBorder_color() != NULL )
        {
            const domFloat4& col = sampler->getBorder_color()->getValue();
            parameters.border.set(col[0], col[1], col[2], col[3]);
        }
    }

    osg::Texture2D* t2D = NULL;
    TextureParametersMap::const_iterator mapIt = _textureParamMap.find(parameters);
    if (mapIt != _textureParamMap.end())
    {
        t2D = mapIt->second.get();
    }
    else
    {
        osg::ref_ptr<osg::Image> img = osgDB::readRefImageFile(parameters.filename);

        if (!img.valid())
        {
            _textureParamMap[parameters] = NULL;
            return NULL;
        }

        OSG_INFO<<"  processTexture(..) - readImage("<<parameters.filename<<")"<<std::endl;

        if (tuu == TRANSPARENCY_MAP_UNIT)
        {
            img = processImageTransparency(img.get(), opaque, transparency);
        }

        t2D = new osg::Texture2D(img.get());

        t2D->setWrap( osg::Texture::WRAP_S, parameters.wrap_s);
        t2D->setWrap( osg::Texture::WRAP_T, parameters.wrap_t);
        t2D->setFilter( osg::Texture::MIN_FILTER, parameters.filter_min);
        t2D->setFilter( osg::Texture::MAG_FILTER, parameters.filter_mag);
        t2D->setBorderColor(parameters.border);

        _textureParamMap[parameters] = t2D;
    }

    _texCoordSetMap[TextureToCoordSetMap::key_type(ss, tuu)] = tex->getTexcoord();

    return t2D;
}


/*
Collada 1.4.1 Specification (2nd Edition) Patch Release Notes: Revision C Release notes

In <blinn>, <constant>, <lambert>, and <phong>, the child element <transparent> now has an
optional opaque attribute whose valid values are:
 A_ONE (the default): Takes the transparency information from the colors alpha channel, where the value 1.0 is opaque.
 RGB_ZERO: Takes the transparency information from the colors red, green, and blue channels, where the value 0.0 is opaque,
 with each channel modulated independently.

 In the Specification, this is described in the FX Reference chapter in the
common_color_or_texture_type entry, along with a description of how transparency works in the
Getting Started with COLLADA FX chapter in the Determining Transparency section.


Collada Digital Asset Schema Release 1.5.0 Release Notes

The <transparent> elements opaque attribute now allows, in addition to A_ONE and RGB_ZERO, the following values:
 A_ZERO: Takes the transparency information from the colors alpha channel, where the value 0.0 is opaque.
 RGB_ONE: Takes the transparency information from the colors red, green, and blue channels, where the value 1.0
 is opaque, with each channel modulated independently.

When we update to a version of the dom using that schema we will need to modify the code below
*/

void daeReader::processTransparencySettings( domCommon_transparent_type *ctt,
                                            domCommon_float_or_param_type *pTransparency,
                                            osg::StateSet *ss,
                                            osg::Material *material,
                                            unsigned int diffuseTextureUnit  )
{
    if (ss == NULL)
        return;

    if (NULL == ctt && NULL == pTransparency)
        return;

    float transparency = 1.0f;
    if (pTransparency)
    {
        if (pTransparency->getFloat())
        {
            transparency = pTransparency->getFloat()->getValue();
        }
        else if (pTransparency->getParam())
        {
            domFloat transparencyParam;
            if (GetFloatParam(pTransparency->getParam()->getRef(), transparencyParam))
            {
                transparency = transparencyParam;
            }
        }

        if (_invertTransparency)
        {
            transparency = 1.0f - transparency;
        }
    }

    osg::Texture2D* pTransparentTexture = NULL;
    osg::Vec4 transparentColor(transparency, transparency, transparency, transparency);

    // Fix up defaults according to "Determining Transparency" chapter of 1.4.1 spec
    domFx_opaque_enum opaque = FX_OPAQUE_ENUM_A_ONE;
    if (ctt)
    {
        opaque = ctt->getOpaque();
        if (ctt->getColor())
        {
            const domFx_color_common& domColorValue = ctt->getColor()->getValue();
            transparentColor.set(
                domColorValue.get(0),
                domColorValue.get(1),
                domColorValue.get(2),
                domColorValue.get(3));

            if (opaque == FX_OPAQUE_ENUM_RGB_ZERO)
            {
                transparentColor.set(
                    1.0f - transparentColor.r() * transparency,
                    1.0f - transparentColor.g() * transparency,
                    1.0f - transparentColor.b() * transparency,
                    1.0f - luminance(transparentColor) * transparency);
            }
            else
            {
                float a = transparentColor.a() * transparency;
                transparentColor.set(a, a, a, a);
            }
        }
        else if (ctt->getTexture())
        {
            pTransparentTexture = processTexture(ctt->getTexture(), ss, TRANSPARENCY_MAP_UNIT, opaque, transparency);
        }
    }

    if (pTransparentTexture)
    {
        ss->setTextureAttributeAndModes(TRANSPARENCY_MAP_UNIT, pTransparentTexture);
        ss->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }
    else
    {
        bool strictTransparency = _pluginOptions.strictTransparency;
        if (!strictTransparency)
        {
            const osg::Texture* pMainTexture = dynamic_cast<osg::Texture*>(
                ss->getTextureAttribute(diffuseTextureUnit, osg::StateAttribute::TEXTURE));
            bool haveTranslucentTexture = pMainTexture &&
                pMainTexture->getImage(0) && pMainTexture->getImage(0)->isImageTranslucent();
            strictTransparency = !haveTranslucentTexture;
        }

        if (strictTransparency)
        {
            if (transparentColor == osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f))
            {
                return;
            }

            ss->setAttributeAndModes(new osg::BlendColor(transparentColor));
            ss->setAttributeAndModes(new osg::BlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR));
        }
        else
        {
            ss->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        }
    }

    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

bool daeReader::copyTextureCoordinateSet(const osg::StateSet* ss, const osg::Geometry* cachedGeometry, osg::Geometry* clonedGeometry, const domInstance_material* im, TextureUnitUsage tuu, unsigned int textureUnit)
{
    unsigned int localTextureUnit( _pluginOptions.usePredefinedTextureUnits ? tuu : textureUnit);
    if (!ss->getTextureAttribute(localTextureUnit, osg::StateAttribute::TEXTURE))
        return false;

    const std::string& texCoordSetName = _texCoordSetMap
        [TextureToCoordSetMap::key_type(ss, tuu)];
    if (texCoordSetName.empty()) return false;

    const domInstance_material::domBind_vertex_input_Array &bvia = im->getBind_vertex_input_array();
    size_t k;
    for (k = 0; k < bvia.getCount(); k++)
    {
        if (!strcmp(bvia[k]->getSemantic(), texCoordSetName.c_str()) && !strcmp(bvia[k]->getInput_semantic(), COMMON_PROFILE_INPUT_TEXCOORD))
        {
            unsigned int set = bvia[k]->getInput_set();
            if (set < cachedGeometry->getNumTexCoordArrays())
            {
                clonedGeometry->setTexCoordArray(localTextureUnit, const_cast<osg::Array*>(cachedGeometry->getTexCoordArray(set)));
            }
            else
            {
                OSG_WARN << "Texture coordinate set " << set << " not found." << std::endl;
            }
            break;
        }
    }
    if (k == bvia.getCount())
    {
        OSG_WARN << "Failed to find matching <bind_vertex_input> for " << texCoordSetName << std::endl;

        //bind_vertex_input failed, we try bind
        const domInstance_material::domBind_Array &ba = im->getBind_array();
        for (k = 0; k < ba.getCount(); k++)
        {
            if (!strcmp(ba[k]->getSemantic(), texCoordSetName.c_str()) )
            {
                IdToCoordIndexMap::const_iterator it = _texCoordIdMap.find(ba[k]->getTarget());
                if (it!=_texCoordIdMap.end()&& (cachedGeometry->getNumTexCoordArrays()>it->second))
                {
                  clonedGeometry->setTexCoordArray(localTextureUnit, const_cast<osg::Array*>(cachedGeometry->getTexCoordArray(it->second)));
                }
                else
                {
                    OSG_WARN << "Texture coordinate set " << ba[k]->getTarget() << " not found." << std::endl;
                }
                break;
            }
        }
        if (k == ba.getCount())
        {
            if (cachedGeometry->getNumTexCoordArrays())
            {
                clonedGeometry->setTexCoordArray(localTextureUnit, const_cast<osg::Array*>(cachedGeometry->getTexCoordArray(0)));
            }
        }
    }
    return true;
}
