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

#include <dae.h>
#include <dae/daeSIDResolver.h>
#include <dae/domAny.h>
#include <dom/domCOLLADA.h>
#include <dom/domProfile_COMMON.h>

#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/LightModel>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <sstream>

using namespace osgdae;


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
        osg::notify( osg::WARN ) << "No COMMON technique for bind_material" << std::endl;
        return;
    }

    for (size_t i =0; i < geode->getNumDrawables(); i++)
    {
        osg::Drawable* drawable = geode->getDrawable(i);
        std::string materialName = drawable->getName();
        ReaderGeometry *cachedGeometry = dynamic_cast<ReaderGeometry*>(cachedGeode->getDrawable(i)->asGeometry());
        
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
                    domMaterialStateSetMap::iterator iter = materialMap.find( mat );
                    if ( iter != materialMap.end() )
                    {
                        // Reuse material
                        ss = iter->second.get();
                    }
                    else
                    {
                        // Create new material
                        ss = new osg::StateSet;
                        processMaterial(ss, mat);
                        materialMap.insert(std::make_pair(mat, ss));
                    }
                    drawable->setStateSet(ss);
                    // Need to process bind_vertex_inputs here
                    // This all feels like a horrible kludge to me
                    // I wish somebody with a better knowledge of Collada and OSG than me would have a go at it!
                    // 1. Clear the texcoord arrays and associated texcoord vertex indices
                    // from the current (cloned) drawable.
                    osg::Geometry *clonedGeometry = drawable->asGeometry();
                    if (NULL == clonedGeometry)
                    {
                        osg::notify( osg::WARN ) << "Failed to convert drawable to geometry object" << std::endl;
                        break;
                    }
                    clonedGeometry->getTexCoordArrayList().clear();
                    // 2. For each possible texture unit find the correct texcoord array and 
                    // indices from the original (uncloned) drawable and place in the cloned drawable
                    // in the correct texture unit slot
                    std::string TransparencyMapTexcoordName;
                    osg::Texture2D *Texture;
                    if (NULL != (Texture = dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(AMBIENT_OCCLUSION_UNIT, osg::StateAttribute::TEXTURE))))
                    {
                        std::string AmbientOcclusionTexcoordName = Texture->getName();
                        if (!AmbientOcclusionTexcoordName.empty())
                        {
                            domInstance_material::domBind_vertex_input_Array &bvia = ima[j]->getBind_vertex_input_array();
                            size_t k;
                            for ( k = 0; k < bvia.getCount(); k++)
                            {
                                if (!strcmp(bvia[k]->getSemantic(), AmbientOcclusionTexcoordName.c_str()) && !strcmp(bvia[k]->getInput_semantic(), "TEXCOORD"))
                                {
                                    // OK - found the effect name, now see if I can find a matching set in the cachedGeometry
                                    if (NULL != cachedGeometry)
                                    {
                                        std::map<int, int>::iterator iTr;
                                        if (cachedGeometry->_TexcoordSetMap.end() != (iTr = cachedGeometry->_TexcoordSetMap.find(bvia[k]->getInput_set())))
                                        {
                                            // Copy the texture cordinates and indices (if any) into the cloned geometry
                                            clonedGeometry->setTexCoordData(AMBIENT_OCCLUSION_UNIT, cachedGeometry->getTexCoordData(iTr->second));
                                        }
                                    }
                                    break;
                                }
                            }
                            if (k == bvia.getCount())
                                osg::notify( osg::WARN ) << "Failed to find matching <bind_vertex_input> for " << AmbientOcclusionTexcoordName << std::endl;
                        }
                    }
                    if (NULL != (Texture = dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(MAIN_TEXTURE_UNIT, osg::StateAttribute::TEXTURE))))
                    {
                        std::string MainTextureTexcoordName = Texture->getName();
                        if (!MainTextureTexcoordName.empty())
                        {
                            domInstance_material::domBind_vertex_input_Array &bvia = ima[j]->getBind_vertex_input_array();
                            size_t k;
                            for ( k = 0; k < bvia.getCount(); k++)
                            {
                                if (!strcmp(bvia[k]->getSemantic(), MainTextureTexcoordName.c_str()) && !strcmp(bvia[k]->getInput_semantic(), "TEXCOORD"))
                                {
                                    // OK - found the effect name, now see if I can find a matching set in the cachedGeometry
                                    if (NULL != cachedGeometry)
                                    {
                                        std::map<int, int>::iterator iTr;
                                        if (cachedGeometry->_TexcoordSetMap.end() != (iTr = cachedGeometry->_TexcoordSetMap.find(bvia[k]->getInput_set())))
                                        {
                                            // Copy the texture cordinates and indices (if any) into the cloned geometry
                                            clonedGeometry->setTexCoordData(MAIN_TEXTURE_UNIT, cachedGeometry->getTexCoordData(iTr->second));
                                        }
                                    }
                                    break;
                                }
                            }
                            if (k == bvia.getCount())
                            {
                                osg::notify( osg::WARN ) << "Failed to find matching <bind_vertex_input> for " << MainTextureTexcoordName << std::endl;
                                // This may be a departure from the spec. For the time being I am only going to do this
                                // for the MAIN_TEXTURE_UNIT.
                                // Not found so just use the first TEXCOORD we have if any.
                                if (cachedGeometry->_TexcoordSetMap.size() > 0)
                                    clonedGeometry->setTexCoordData(MAIN_TEXTURE_UNIT, cachedGeometry->getTexCoordData(cachedGeometry->_TexcoordSetMap.begin()->second));
                            }
                        }
                    }
                    if (NULL != (Texture = dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(TRANSPARENCY_MAP_UNIT, osg::StateAttribute::TEXTURE))))
                    {
                        std::string TransparencyMapTexcoordName = Texture->getName();
                        if (!TransparencyMapTexcoordName.empty())
                        {
                            domInstance_material::domBind_vertex_input_Array &bvia = ima[j]->getBind_vertex_input_array();
                            size_t k;
                            for ( k = 0; k < bvia.getCount(); k++)
                            {
                                if (!strcmp(bvia[k]->getSemantic(), TransparencyMapTexcoordName.c_str()) && !strcmp(bvia[k]->getInput_semantic(), "TEXCOORD"))
                                {
                                    // OK - found the effect name, now see if I can find a matching set in the cachedGeometry
                                    if (NULL != cachedGeometry)
                                    {
                                        std::map<int, int>::iterator iTr;
                                        if (cachedGeometry->_TexcoordSetMap.end() != (iTr = cachedGeometry->_TexcoordSetMap.find(bvia[k]->getInput_set())))
                                        {
                                            // Copy the texture cordinates and indices (if any) into the cloned geometry
                                            clonedGeometry->setTexCoordData(TRANSPARENCY_MAP_UNIT, cachedGeometry->getTexCoordData(iTr->second));
                                        }
                                    }
                                    break;
                                }
                            }
                            if (k == bvia.getCount())
                                osg::notify( osg::WARN ) << "Failed to find matching <bind_vertex_input> for " << TransparencyMapTexcoordName << std::endl;
                        }
                    }
                }
                else
                {
                    osg::notify( osg::WARN ) << "Failed to locate <material> with id " << ima[i]->getTarget().getURI() << std::endl;
                }

                break;
            }
        }
        if (!found)
        {
            osg::notify( osg::WARN ) << "Failed to locate <instance_material> with symbol " << materialName << std::endl;
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
    currentInstance_effect = mat->getInstance_effect();
    domEffect *effect = daeSafeCast< domEffect >( getElementFromURI( currentInstance_effect->getUrl() ) );
    if (effect)
    {
        processEffect(ss, effect);
    
        //TODO: process all of the setParams that could happen here in the material. ESP. the textures
    }
    else
    {
        osg::notify( osg::WARN ) << "Failed to locate effect " << mat->getInstance_effect()->getUrl().getURI() << std::endl;
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
        if ( pc != NULL )
        {
            if ( hasCOMMON )
            {
                osg::notify( osg::WARN ) << "Effect already has a profile_COMMON. Skipping this one" << std::endl;
                continue;
            }
            currentEffect = effect;
            processProfileCOMMON(ss, pc);
            hasCOMMON = true;
            continue;
        }

        osg::notify( osg::WARN ) << "unsupported effect profile " << effect->getFx_profile_abstract_array()[i]->getTypeName() << std::endl;
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
            if (strcmp(TechniqueArray[CurrentTechnique]->getProfile(), "GOOGLEEARTH") == 0)
            {
                const daeElementRefArray& ElementArray = TechniqueArray[CurrentTechnique]->getContents();
                size_t NumberOfElements = ElementArray.getCount();
                size_t CurrentElement;
                for (CurrentElement = 0; CurrentElement < NumberOfElements; CurrentElement++)
                {
                    domAny* pAny = (domAny*)ElementArray[CurrentElement].cast();
                    if (strcmp(pAny->getElementName(), "double_sided") == 0)
                    {
                        daeString Value = pAny->getValue();
                        if (strcmp(Value, "1") == 0)
                            ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
                    }
                }
            }
        }
    }

    xsNCName DiffuseTextureName = NULL;
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
    if ( b != NULL )
    {
        osg::StateAttribute *EmissionStateAttribute = NULL;
        osg::StateAttribute *AmbientStateAttribute = NULL;
        osg::StateAttribute *DiffuseStateAttribute = NULL;
        processColorOrTextureType( b->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &EmissionStateAttribute );
        if (NULL != EmissionStateAttribute)
            osg::notify( osg::WARN ) << "Currently no support for <texture> in Emission channel " << std::endl;

        processColorOrTextureType( b->getAmbient(), osg::Material::AMBIENT, mat.get(), NULL,  &AmbientStateAttribute );
        
        processColorOrTextureType( b->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &DiffuseStateAttribute );
        if ( DiffuseStateAttribute != NULL ) 
        {
            if ( AmbientStateAttribute != NULL ) 
            {
                // Set the ambient and diffuse colour white so that the incoming fragment colour ends up as a
                // lit white colour. I modulate both textures onto this to approximate the lighting equation.
                // Using a zero diffuse and then an ADD of the diffuse texture seems overlit to me.
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                // Use the ambient texture map as an occlusion map.
                ss->setTextureMode( AMBIENT_OCCLUSION_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(AMBIENT_OCCLUSION_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( AMBIENT_OCCLUSION_UNIT, AmbientStateAttribute );
                // Modulate in the diffuse texture
                ss->setTextureMode( MAIN_TEXTURE_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(MAIN_TEXTURE_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( MAIN_TEXTURE_UNIT, DiffuseStateAttribute );
            }
            else
            {
                // Set the diffuse colour white so that the incoming fragment colour ends up as the global diffuse lighting colour
                // plus any constant ambient contribution after the lighting calculation. This means that I am modulating the the
                // ambient with the texture as well but I cannot see a way of avoiding that.
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                ss->setTextureMode( MAIN_TEXTURE_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(MAIN_TEXTURE_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( MAIN_TEXTURE_UNIT, DiffuseStateAttribute );
            }

            // Save the texture name for later
            DiffuseTextureName = b->getDiffuse()->getTexture()->getTexture();
        }
        else
        {
            if ( NULL != AmbientStateAttribute  ) 
                osg::notify( osg::WARN ) << "Ambient occlusion map only supported when diffuse texture also specified" << std::endl;
        }

        if(processColorOrTextureType( b->getSpecular(), osg::Material::SPECULAR, mat.get(), b->getShininess() ) && (NULL != DiffuseStateAttribute) )
        {
            // Diffuse texture will defeat specular highlighting
            // So postpone specular - Not sure if I should do this here
            // beacuse it will override any global light model states
            osg::LightModel* lightmodel = new osg::LightModel;
            lightmodel->setColorControl(osg::LightModel::SEPARATE_SPECULAR_COLOR);
            ss->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);
        }

        processTransparencySettings(b->getTransparent(), b->getTransparency(), ss, mat.get(), DiffuseTextureName );
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
    else if ( p != NULL )
    {
        osg::StateAttribute *EmissionStateAttribute = NULL;
        osg::StateAttribute *AmbientStateAttribute = NULL;
        osg::StateAttribute *DiffuseStateAttribute = NULL;
        processColorOrTextureType( p->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &EmissionStateAttribute );
        if (NULL != EmissionStateAttribute)
            osg::notify( osg::WARN ) << "Currently no support for <texture> in Emission channel " << std::endl;
        
        processColorOrTextureType( p->getAmbient(), osg::Material::AMBIENT, mat.get(), NULL,  &AmbientStateAttribute );
        
        processColorOrTextureType( p->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &DiffuseStateAttribute );
        if ( DiffuseStateAttribute != NULL ) 
        {
            if ( AmbientStateAttribute != NULL ) 
            {
                // Set the ambient and diffuse colour white so that the incoming fragment colour ends up as a
                // lit white colour. I modulate both textures onto this to approximate the lighting equation.
                // Using a zero diffuse and then an ADD of the diffuse texture seems overlit to me.
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                // Use the ambient texture map as an occlusion map.
                ss->setTextureMode( AMBIENT_OCCLUSION_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(AMBIENT_OCCLUSION_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( AMBIENT_OCCLUSION_UNIT, AmbientStateAttribute );
                // Modulate in the diffuse texture
                ss->setTextureMode( MAIN_TEXTURE_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(MAIN_TEXTURE_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( MAIN_TEXTURE_UNIT, DiffuseStateAttribute );
            }
            else
            {
                // Set the diffuse colour white so that the incoming fragment colour ends up as the global diffuse lighting colour
                // plus any constant ambient contribution after the lighting calculation. This means that I am modulating the the
                // ambient with the texture as well but I cannot see a way of avoiding that.
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                ss->setTextureMode( MAIN_TEXTURE_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(MAIN_TEXTURE_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( MAIN_TEXTURE_UNIT, DiffuseStateAttribute );
            }

            // Save the texture name for later
            DiffuseTextureName = p->getDiffuse()->getTexture()->getTexture();
        }
        else
        {
            if ( NULL != AmbientStateAttribute  ) 
                osg::notify( osg::WARN ) << "Ambient occlusion map only supported when diffuse texture also specified" << std::endl;
        }
        
        if(processColorOrTextureType( p->getSpecular(), osg::Material::SPECULAR, mat.get(), p->getShininess() ) && (NULL != DiffuseStateAttribute) )
        {
            // Diffuse texture will defeat specular highlighting
            // So postpone specular - Not sure if I should do this here
            // beacuse it will override any global light model states
            osg::LightModel* lightmodel = new osg::LightModel;
            lightmodel->setColorControl(osg::LightModel::SEPARATE_SPECULAR_COLOR);
            ss->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);
        }

        processTransparencySettings(p->getTransparent(), p->getTransparency(), ss, mat.get(), DiffuseTextureName );
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
    else if ( l != NULL )
    {
        osg::StateAttribute *EmissionStateAttribute = NULL;
        osg::StateAttribute *AmbientStateAttribute = NULL;
        osg::StateAttribute *DiffuseStateAttribute = NULL;
        processColorOrTextureType( l->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &EmissionStateAttribute );
        if (NULL != EmissionStateAttribute)
            osg::notify( osg::WARN ) << "Currently no support for <texture> in Emission channel " << std::endl;
        
        processColorOrTextureType( l->getAmbient(), osg::Material::AMBIENT, mat.get(), NULL,  &AmbientStateAttribute);

        processColorOrTextureType( l->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &DiffuseStateAttribute );
        if ( DiffuseStateAttribute != NULL ) 
        {
            if ( AmbientStateAttribute != NULL ) 
            {
                // Set the ambient and diffuse colour white so that the incoming fragment colour ends up as a
                // lit white colour. I modulate both textures onto this to approximate the lighting equation.
                // Using a zero diffuse and then an ADD of the diffuse texture seems overlit to me.
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                // Use the ambient texture map as an occlusion map.
                ss->setTextureMode( AMBIENT_OCCLUSION_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(AMBIENT_OCCLUSION_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( AMBIENT_OCCLUSION_UNIT, AmbientStateAttribute );
                // Modulate in the diffuse texture
                ss->setTextureMode( MAIN_TEXTURE_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(MAIN_TEXTURE_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( MAIN_TEXTURE_UNIT, DiffuseStateAttribute );
            }
            else
            {
                // Set the diffuse colour white so that the incoming fragment colour ends up as the global diffuse lighting colour
                // plus any constant ambient contribution after the lighting calculation. This means that I am modulating the the
                // ambient with the texture as well but I cannot see a way of avoiding that.
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                ss->setTextureMode( MAIN_TEXTURE_UNIT, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute(MAIN_TEXTURE_UNIT, new osg::TexEnv(osg::TexEnv::MODULATE) );
                ss->setTextureAttribute( MAIN_TEXTURE_UNIT, DiffuseStateAttribute );
            }

            // Save the texture name for later
            DiffuseTextureName = l->getDiffuse()->getTexture()->getTexture();
        }
        else
        {
            if ( NULL != AmbientStateAttribute  ) 
                osg::notify( osg::WARN ) << "Ambient occlusion map only supported when diffuse texture also specified" << std::endl;
        }

        processTransparencySettings(l->getTransparent(), l->getTransparency(), ss, mat.get(), DiffuseTextureName );
    }
    // <constant>
    // elements:
    // 0..1 <emission>
    // 0..1 <reflective>
    // 0..1 <reflectivity>
    // 0..1 <transparent>
    // 0..1 <transparency>
    // 0..1 <index_of_refraction>
    else if ( c != NULL )
    {
        osg::StateAttribute *sa = NULL;
        processColorOrTextureType( c->getEmission(), osg::Material::EMISSION, mat.get(), NULL, &sa );
        if ( sa != NULL ) 
        {
            ss->setTextureMode( MAIN_TEXTURE_UNIT, GL_TEXTURE_2D, GL_TRUE );
            ss->setTextureAttribute(MAIN_TEXTURE_UNIT, new osg::TexEnv(osg::TexEnv::REPLACE) );
            ss->setTextureAttribute( MAIN_TEXTURE_UNIT, sa );
        }

        // Use the emission colour as the main colour in transparency calculations
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, mat->getEmission(osg::Material::FRONT_AND_BACK));

        processTransparencySettings(c->getTransparent(), c->getTransparency(), ss, mat.get(), NULL );

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
bool daeReader::processColorOrTextureType(    domCommon_color_or_texture_type *cot, 
                                            osg::Material::ColorMode channel,
                                            osg::Material *mat,
                                            domCommon_float_or_param_type *fop,
                                            osg::StateAttribute **sa,
                                            bool blinn)
{
    if ( cot == NULL )
    {
        return false;
    }
    bool retVal = false;

    //osg::StateAttribute *sa = NULL;
    //TODO: Make all channels process <param ref=""> type of value
    if ( channel == osg::Material::EMISSION )
    {
        if ( cot->getColor() != NULL )
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
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
                *sa = processTexture( cot->getTexture() );
                retVal = true;
            }
            else
                osg::notify( osg::WARN ) << "Currently no support for <texture> in Emission channel " << std::endl;
        }       
        else
        {
            osg::notify( osg::WARN ) << "Missing <color>, <param> or <texture> in Emission channel " << std::endl;
        }
    }
    else if ( channel == osg::Material::AMBIENT )
    {
        if ( cot->getColor() != NULL )
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
            retVal = true;
        }
        else if (cot->getParam() != NULL)
        {
            domFloat4 f4;
            if (GetFloat4Param(cot->getParam()->getRef(), f4))
            {
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
                retVal = true;
            }
        }
        else if (cot->getTexture() != NULL)
        {
            if (sa != NULL)
                *sa = processTexture( cot->getTexture() );
            else
            {
                osg::notify( osg::WARN ) << "Currently no support for <texture> in Ambient channel " << std::endl;
                mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
            }
            retVal = true;
       }
        else
        {
            osg::notify( osg::WARN ) << "Missing <color>, <param> or <texture> in Ambient channel " << std::endl;
        }
    }
    else if ( channel == osg::Material::DIFFUSE )
    {
        if ( cot->getColor() != NULL )
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
            retVal = true;
        }
        else if ( cot->getTexture() != NULL)
        {
            if (sa != NULL)
                *sa = processTexture( cot->getTexture() );
            else
            {
                osg::notify( osg::WARN ) << "Currently no support for <texture> in Diffuse channel " << std::endl;
                mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.8f, 0.8f, 0.8f, 1.0f ) );
            }
            domExtra *extra = cot->getTexture()->getExtra();
            if ( extra != NULL && extra->getType() != NULL && strcmp( extra->getType(), "color" ) == 0 )
            {
                //the extra data for osg. Diffuse color can happen with a texture.
                for ( unsigned int i = 0; i < extra->getTechnique_array().getCount(); i++ )
                {
                    domTechnique *teq = extra->getTechnique_array()[i];
                    if ( strcmp( teq->getProfile(), "SCEI" ) == 0 )
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
            osg::notify( osg::WARN ) << "Missing <color>, <param> or <texture> in Diffuse channel " << std::endl;
        }
    }
    else if ( channel == osg::Material::SPECULAR )
    {
        if ( cot->getColor() != NULL )
        {
            domFloat4 &f4 = cot->getColor()->getValue();
            mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( f4[0], f4[1], f4[2], f4[3] ) );
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
            osg::notify( osg::WARN ) << "Currently no support for <texture> in Specular channel " << std::endl;
        }
        else
        {
            osg::notify( osg::WARN ) << "Missing <color>, <param> or <texture> in Specular channel " << std::endl;
        }

        if ( fop != NULL && fop->getFloat() != NULL )
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

bool daeReader::GetFloat4Param(xsNCName Reference, domFloat4 &f4)
{
    std::string MyReference = Reference;

    MyReference.insert(0, "./");
    daeSIDResolver Resolver(currentEffect, MyReference.c_str());
    daeElement *el = Resolver.getElement();
    if (NULL == el)
            return false;

    if (NULL != currentInstance_effect)
    {
        // look here first for setparams
        // I am sure there must be a better way of doing this
        // Maybe the Collada DAE guys can give us a parameter management mechanism !
        const domInstance_effect::domSetparam_Array& SetParamArray = currentInstance_effect->getSetparam_array();
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

bool daeReader::GetFloatParam(xsNCName Reference, domFloat &f)
{
    std::string MyReference = Reference;

    MyReference.insert(0, "./");
    daeSIDResolver Resolver(currentEffect, MyReference.c_str());
    daeElement *el = Resolver.getElement();
    if (NULL == el)
        return false;

    if (NULL != currentInstance_effect)
    {
        // look here first for setparams
        // I am sure there must be a better way of doing this
        // Maybe the Collada DAE guys can give us a parameter management mechanism !
        const domInstance_effect::domSetparam_Array& SetParamArray = currentInstance_effect->getSetparam_array();
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

osg::StateAttribute *daeReader::processTexture( domCommon_color_or_texture_type_complexType::domTexture *tex )
{
    //find the newparam for the sampler based on the texture attribute
    domFx_sampler2D_common *sampler = NULL;
    domFx_surface_common *surface = NULL;
    domImage *dImg = NULL;

    std::string target = std::string("./") + std::string(tex->getTexture());
    osg::notify(osg::NOTICE)<<"processTexture("<<target<<")"<<std::endl;
    
    daeSIDResolver res1( currentEffect, target.c_str() );
    daeElement *el = res1.getElement();

    if ( el == NULL )
    {
        osg::notify( osg::WARN ) << "Could not locate newparam for texture sampler2D" << tex->getTexture() << std::endl;
        osg::notify( osg::WARN ) << "Checking if data does incorrect linking straight to the image" << std::endl;
        dae->getDatabase()->getElement( (daeElement**)&dImg, 0, tex->getTexture(), "image" );
        if ( dImg != NULL )
        {
            osg::notify( osg::WARN ) << "Direct image link found. Data is incorrect but will continue to load texture" << std::endl;
        }
    }
    else
    {
        domCommon_newparam_type *cnp = daeSafeCast< domCommon_newparam_type >( el ); 
        domFx_newparam_common *npc = daeSafeCast< domFx_newparam_common >( el );

        if ( cnp != NULL )
        {
            sampler = cnp->getSampler2D();
        }
        else if ( npc != NULL )
        {
            sampler = npc->getFx_basic_type_common()->getSampler2D();
        }

        if ( sampler == NULL )
        {
            osg::notify( osg::WARN ) << "Wrong newparam type. Expected sampler2D" << std::endl;
            return NULL;
        }

        //find the newparam for the surface based on the sampler2D->source value
        target = std::string("./") + std::string( sampler->getSource()->getValue() );
        daeSIDResolver res2( currentEffect, target.c_str() );
        el = res2.getElement();
        if ( el == NULL )
        {
            osg::notify( osg::WARN ) << "Could not locate newparam for source " << sampler->getSource()->getValue() << std::endl;
            return NULL;
        }
        cnp = daeSafeCast< domCommon_newparam_type >( el ); 
        npc = daeSafeCast< domFx_newparam_common >( el );

        if ( cnp != NULL )
        {
            surface = cnp->getSurface();
        }
        else if ( npc != NULL )
        {
            surface = npc->getFx_basic_type_common()->getSurface();
        }

        if ( surface == NULL )
        {
            osg::notify( osg::WARN ) << "Wrong newparam type. Expected surface" << std::endl;
            return NULL;
        }

        //look for the domImage based on the surface initialization stuff
        daeIDRef &ref = surface->getFx_surface_init_common()->getInit_from_array()[0]->getValue();
        dImg = daeSafeCast< domImage >( getElementFromIDRef( ref ) );
    }
    if ( dImg == NULL )
    {
        osg::notify( osg::WARN ) << "Could not locate image for texture" << std::endl;
        return NULL;
    }
    //Got a sampler and a surface and an imaged. Time to create the texture stuff for osg
    osg::ref_ptr<osg::Image> img = NULL;
    if ( dImg->getInit_from() != NULL )
    {
        // daeURI uri = dImg->getInit_from()->getValue();
        dImg->getInit_from()->getValue().validate();
        if ( std::string( dImg->getInit_from()->getValue().getProtocol() ) == std::string( "file" ) )
        {
            //unsigned int bufSize = 1; //for the null char
            //bufSize += dImg->getInit_from()->getValue().pathDir().size();
            //bufSize += dImg->getInit_from()->getValue().pathFile().size();
           std::string path =  dImg->getInit_from()->getValue().pathDir()+
                                  dImg->getInit_from()->getValue().pathFile();
              // remove space encodings
              //
              path = cdom::uriToNativePath(path);
           if(path.empty())
           {
              osg::notify( osg::WARN ) << "Unable to get path from URI." << std::endl;
              return NULL;
           }
#ifdef WIN32
            // If the path has a drive specifier or a UNC name then strip the leading /
            const char* filename =path.c_str();
            if ((path[2] == ':') || ((path[1] == '/') && (path[2] == '/')))
               ++filename;// = path+1;
//            else
//                filename = path;
#else
            const char* filename = path.c_str();
#endif
            img = osgDB::readRefImageFile( filename );

            osg::notify(osg::INFO)<<"  processTexture(..) - readImage("<<filename<<")"<<std::endl;
            
            //Moved this below the osg::notify - Parag, 24/7/2007
            //delete [] path;

            
        }
        else
        {
            osg::notify( osg::WARN ) << "Only images with a \"file\" scheme URI are supported in this version." << std::endl;
            return NULL;
        }
    }
    else
    {
        osg::notify( osg::WARN ) << "Embedded image data is not supported in this version." << std::endl;
            return NULL;
    }

    osg::Texture2D *t2D = new osg::Texture2D( img.get() );
    //set texture parameters
    if ( sampler != NULL )
    {
        if ( sampler->getWrap_s() != NULL )
        {
            osg::Texture::WrapMode wrap;
            switch( sampler->getWrap_s()->getValue() )
            {
            case FX_SAMPLER_WRAP_COMMON_WRAP:
                wrap = osg::Texture::REPEAT;
                break;
            case FX_SAMPLER_WRAP_COMMON_MIRROR:
                wrap = osg::Texture::MIRROR;
                break;
            case FX_SAMPLER_WRAP_COMMON_CLAMP:
                wrap = osg::Texture::CLAMP_TO_EDGE;
                break;
            case FX_SAMPLER_WRAP_COMMON_NONE:
            case FX_SAMPLER_WRAP_COMMON_BORDER:
                wrap = osg::Texture::CLAMP_TO_BORDER;
                break;
            default:
                wrap = osg::Texture::CLAMP;
                break;
            }
            t2D->setWrap( osg::Texture::WRAP_S, wrap );
        }
        else
        {
            t2D->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
        }
        if ( sampler->getWrap_t() != NULL )
        {
            osg::Texture::WrapMode wrap;
            switch( sampler->getWrap_t()->getValue() )
            {
            case FX_SAMPLER_WRAP_COMMON_WRAP:
                wrap = osg::Texture::REPEAT;
                break;
            case FX_SAMPLER_WRAP_COMMON_MIRROR:
                wrap = osg::Texture::MIRROR;
                break;
            case FX_SAMPLER_WRAP_COMMON_CLAMP:
                wrap = osg::Texture::CLAMP_TO_EDGE;
                break;
            case FX_SAMPLER_WRAP_COMMON_NONE:
            case FX_SAMPLER_WRAP_COMMON_BORDER:
                wrap = osg::Texture::CLAMP_TO_BORDER;
                break;
            default:
                wrap = osg::Texture::CLAMP;
                break;
            }
            t2D->setWrap( osg::Texture::WRAP_T, wrap );
        }
        else
        {
            t2D->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
        }
        if ( sampler->getMinfilter() != NULL )
        {
            osg::Texture::FilterMode mode;
            switch( sampler->getMinfilter()->getValue() )
            {
            case FX_SAMPLER_FILTER_COMMON_NEAREST:
                mode = osg::Texture::NEAREST;
                break;
            case FX_SAMPLER_FILTER_COMMON_LINEAR:
                mode = osg::Texture::LINEAR;
                break;
            case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_NEAREST:
                mode = osg::Texture::NEAREST_MIPMAP_NEAREST;
                break;
            case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_NEAREST:
                mode = osg::Texture::LINEAR_MIPMAP_NEAREST;
                break;
            case FX_SAMPLER_FILTER_COMMON_NONE:
            case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_LINEAR:
                mode = osg::Texture::NEAREST_MIPMAP_LINEAR;
                break;
            case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR:
                mode = osg::Texture::LINEAR_MIPMAP_LINEAR;
                break;
            default:
                mode = osg::Texture::LINEAR;
                break;
            }
            t2D->setFilter( osg::Texture::MIN_FILTER, mode );
        }
        else
        {
            t2D->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR );
        }
        if ( sampler->getMagfilter() != NULL )
        {
            osg::Texture::FilterMode mode;
            switch( sampler->getMagfilter()->getValue() )
            {
            case FX_SAMPLER_FILTER_COMMON_NEAREST:
                mode = osg::Texture::NEAREST;
                break;
            case FX_SAMPLER_FILTER_COMMON_NONE:
            case FX_SAMPLER_FILTER_COMMON_LINEAR:
                mode = osg::Texture::LINEAR;
                break;
            case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_NEAREST:
                mode = osg::Texture::NEAREST_MIPMAP_NEAREST;
                break;
            case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_NEAREST:
                mode = osg::Texture::LINEAR_MIPMAP_NEAREST;
                break;
            case FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_LINEAR:
                mode = osg::Texture::NEAREST_MIPMAP_LINEAR;
                break;
            case FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR:
                mode = osg::Texture::LINEAR_MIPMAP_LINEAR;
                break;
            default:
                mode = osg::Texture::LINEAR;
                break;
            }
            t2D->setFilter( osg::Texture::MAG_FILTER, mode );
        }
        else
        {
            t2D->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
        }
        if ( sampler->getBorder_color() != NULL )
        {
            const domFloat4 &col = sampler->getBorder_color()->getValue();
            t2D->setBorderColor( osg::Vec4( col[0], col[1], col[2], col[3] ) );
        }
    }
    else 
    {
        t2D->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
        t2D->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
        t2D->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR );
        t2D->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    }

    // Store the texcoord name in the texture object
    t2D->setName(tex->getTexcoord());
    return t2D;
}


/*
Collada 1.4.1 Specification (2nd Edition) Patch Release Notes: Revision C Release notes

In <blinn>, <constant>, <lambert>, and <phong>, the child element <transparent> now has an
optional opaque attribute whose valid values are:
• A_ONE (the default): Takes the transparency information from the color’s alpha channel, where the value 1.0 is opaque.
• RGB_ZERO: Takes the transparency information from the color’s red, green, and blue channels, where the value 0.0 is opaque,
with each channel modulated independently.
In the Specification, this is described in the “FX Reference” chapter in the
common_color_or_texture_type entry, along with a description of how transparency works in the
“Getting Started with COLLADA FX” chapter in the “Determining Transparency” section.


Collada Digital Asset Schema Release 1.5.0 Release Notes

The <transparent> element’s opaque attribute now allows, in addition to A_ONE and RGB_ZERO, the following values:
• A_ZERO: Takes the transparency information from the color’s alpha channel, where the value 0.0 is opaque.
• RGB_ONE: Takes the transparency information from the color’s red, green, and blue channels, where the value 1.0
* is opaque, with each channel modulated independently.
* When we update to a version of the dom using that schema we will need to modify the code below 
*/

void daeReader::processTransparencySettings( domCommon_transparent_type *ctt,
                                            domCommon_float_or_param_type *pTransparency,
                                            osg::StateSet *ss,
                                            osg::Material *material,
                                            xsNCName diffuseTextureName  )
{
    if (NULL == ctt && NULL == pTransparency)
        return;

    if (ctt && ctt->getTexture() != NULL)
    {
        if (!diffuseTextureName || (strcmp( ctt->getTexture()->getTexture(), diffuseTextureName)))
        {
            osg::notify( osg::WARN ) << "Currently no support for different textures in diffuse and transparent channels." << std::endl;
            return;
        }
    }
    
    // Fix up defaults according to 1.4.1 release notes
    domFloat4 f4;
    domFx_opaque_enum Opaque = FX_OPAQUE_ENUM_A_ONE;
    if (NULL == ctt)
    {
        f4.append(0.0f);
        f4.append(0.0f);
        f4.append(0.0f);
        f4.append(1.0f);
    }
    else
    {
        Opaque = ctt->getOpaque();
        if (NULL != ctt->getColor())
        {
            f4 = ctt->getColor()->getValue();
        }
        else if ((NULL == ctt->getParam()) || !GetFloat4Param(ctt->getParam()->getRef(), f4))
        {
            f4.append(0.0f);
            f4.append(0.0f);
            f4.append(0.0f);
            f4.append(1.0f);
        }
    }

    domFloat Transparency = 1.0f;
    if (NULL != pTransparency)
    {
        if (NULL != pTransparency->getFloat())
        {
            Transparency = pTransparency->getFloat()->getValue();
            if (m_AuthoringTool == GOOGLE_SKETCHUP) // Google back to front support
                Transparency = 1.0f - Transparency;
        }
        else if (NULL != pTransparency->getParam())
        {
            if (GetFloatParam(pTransparency->getParam()->getRef(), Transparency))
            {
                if (m_AuthoringTool == GOOGLE_SKETCHUP) // Google back to front support
                    Transparency = 1.0f - Transparency;
            }
            else
                Transparency = 1.0f;
        }
    }

    if (NULL != ctt || NULL != pTransparency)
    {
        int SourceBlendFactor;
        int DestBlendFactor;
        bool SwitchOnTheBlender = false;
        if (m_StrictTransparency)
        {
            // Process transparent and transparency settings accroding to a strict interpretation of the spec
            // See https://collada.org/public_forum/viewtopic.php?f=12&t=1210
            SwitchOnTheBlender = true;
            switch(Opaque)
            {
    /*
                case FX_OPAQUE_ENUM_RGB_ONE:
                    if (ctt->getTexture() != NULL)
                    {
                        SourceBlendFactor = GL_SRC_COLOR;
                        DestBlendFactor = GL_ONE_MINUS_SRC_COLOR;
                    }
                    else
                    {
                        SourceBlendFactor = GL_CONSTANT_COLOR;
                        DestBlendFactor = GL_ONE_MINUS_CONSTANT_COLOR;
                    }
                    break;
                case FX_OPAQUE_ALPHA_ZERO:
                    if (ctt->getTexture() != NULL)
                    {
                        SourceBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
                        DestBlendFactor = GL_SRC_ALPHA;
                    }
                    else
                    {
                        SourceBlendFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
                        DestBlendFactor = GL_CONSTANT_ALPHA;
                    }
                    break;
    */
                case FX_OPAQUE_ENUM_RGB_ZERO:
                    if (ctt->getTexture() != NULL)
                    {
                        SourceBlendFactor = GL_ONE_MINUS_SRC_COLOR;
                        DestBlendFactor = GL_SRC_COLOR;
                    }
                    else
                    {
                        SourceBlendFactor = GL_ONE_MINUS_CONSTANT_COLOR;
                        DestBlendFactor = GL_CONSTANT_COLOR;
                    }
                    break;
                default:
                    if (ctt->getTexture() != NULL)
                    {
                        SourceBlendFactor = GL_SRC_ALPHA;
                        DestBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
                    }
                    else
                    {
                        SourceBlendFactor = GL_CONSTANT_ALPHA;
                        DestBlendFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
                    }
                    break;
            }
        }
        else
        {
            // Jump through various hoops to accomodate the multiplicity of different ways
            // that various people have interpreted the specification
            // I assume that the presence of either a <tansparent> or a <transparency> element
            // means that the user may want some kind of alpha blending
            bool HaveDiffuseTexture = false;
            bool HaveTranslucentDiffuseTexture = false;
            // Unfortunately isImageTransparent only works for  A_ONE_OPAQUE
            if ((NULL != ss) &&
                    (HaveDiffuseTexture = (GL_TRUE == ss->getTextureMode(MAIN_TEXTURE_UNIT, GL_TEXTURE_2D))) &&
                    (NULL != dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(MAIN_TEXTURE_UNIT, osg::StateAttribute::TEXTURE))) &&
                    (NULL != dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(MAIN_TEXTURE_UNIT, osg::StateAttribute::TEXTURE))->getImage()) &&
                    (dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(MAIN_TEXTURE_UNIT, osg::StateAttribute::TEXTURE))->getImage()->isImageTranslucent()))
                HaveTranslucentDiffuseTexture = true;
            osg::Vec4 Diffuse;
            if (material)
                Diffuse = material->getDiffuse(osg::Material::FRONT_AND_BACK);

            // Determine whether or not to switch on the blender and which blending factors to use.
            // I switch the blender on if the supplied (or default) <transparent> and <transparency> elements work out as non opaque,
            // or if they work out opaque and I have a translucent texture in the MAIN_TEXTURE_UNIT or a non opaque value in the diffuse colour
            switch(Opaque)
            {
    /*
                case FX_OPAQUE_ENUM_RGB_ONE:
                    if ((Transparency  * f4[0] > 0.99f) &&
                        (Transparency  * f4[1] > 0.99f) &&
                        (Transparency  * f4[2] > 0.99f))
                    {
                        SourceBlendFactor = GL_SRC_COLOR;
                        DestBlendFactor = GL_ONE_MINUS_SRC_COLOR;
                        // It would be nice to check for a translucent texture here as well
                        if (!HaveDiffuseTexture && (Diffuse.r() < 0.99f) && (Diffuse.g() < 0.99f) && (Diffuse.b() < 0.99f))
                            SwitchOnTheBlender = true;
                    }
                    else
                    {
                        SourceBlendFactor = GL_CONSTANT_COLOR;
                        DestBlendFactor = GL_ONE_MINUS_CONSTANT_COLOR;
                        SwitchOnTheBlender = true;
                    }
                    break;
                case FX_OPAQUE_ALPHA_ZERO:
                    if (Transparency  * f4[3] < 0.01f)
                    {
                        SourceBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
                        DestBlendFactor = GL_SRC_ALPHA;
                        // It would be nice to check for a translucent texture here as well
                        if (Diffuse.a() > 0.01f)
                            SwitchOnTheBlender = true;
                    }
                    else
                    {
                        SourceBlendFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
                        DestBlendFactor = GL_CONSTANT_ALPHA;
                        SwitchOnTheBlender = true;
                    }
                    break;
    */
                case FX_OPAQUE_ENUM_RGB_ZERO:
                    if ((Transparency  * f4[0] < 0.01f) &&
                        (Transparency  * f4[1] < 0.01f) &&
                        (Transparency  * f4[2] < 0.01f))
                    {
                        SourceBlendFactor = GL_ONE_MINUS_SRC_COLOR;
                        DestBlendFactor = GL_SRC_COLOR;
                        // It would be nice to check for a translucent texture here as well
    //                    if (!HaveDiffuseTexture && (Diffuse.r() > 0.01f) && (Diffuse.g() > 0.01f) && (Diffuse.b() > 0.01f))
    //                        SwitchOnTheBlender = true;
                    }
                    else
                    {
                        SourceBlendFactor = GL_ONE_MINUS_CONSTANT_COLOR;
                        DestBlendFactor = GL_CONSTANT_COLOR;
                        SwitchOnTheBlender = true;
                    }
                    break;
                default:
                    if (Transparency  * f4[3] > 0.99f)
                    {
                        SourceBlendFactor = GL_SRC_ALPHA;
                        DestBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
                        if (HaveTranslucentDiffuseTexture || (Diffuse.a() < 0.99f))
                            SwitchOnTheBlender = true;
                    }
                    else
                    {
                        SourceBlendFactor = GL_CONSTANT_ALPHA;
                        DestBlendFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
                        SwitchOnTheBlender = true;
                    }
                    break;
            }
        }
        if (SwitchOnTheBlender)
        {
            if ((SourceBlendFactor == GL_CONSTANT_COLOR) ||
                (SourceBlendFactor == GL_ONE_MINUS_CONSTANT_COLOR) ||
                (SourceBlendFactor == GL_CONSTANT_ALPHA) ||
                (SourceBlendFactor == GL_ONE_MINUS_CONSTANT_ALPHA))
            {
                osg::BlendColor *bc = new osg::BlendColor();
                bc->setConstantColor(osg::Vec4( f4[0] * Transparency, f4[1] * Transparency, f4[2] * Transparency, f4[3] * Transparency ));
                ss->setAttribute( bc );
            }
            osg::BlendFunc *bf = new osg::BlendFunc(SourceBlendFactor, DestBlendFactor);
            ss->setAttribute( bf );
            ss->setMode( GL_BLEND, osg::StateAttribute::ON );

            ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        }
    }
}
