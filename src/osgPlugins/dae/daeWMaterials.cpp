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

#include "daeWriter.h"
#include "ReaderWriterDAE.h"

#include <dae/domAny.h>
#include <dom/domCOLLADA.h>
#include <dom/domConstants.h>
#include <dom/domProfile_COMMON.h>

#include <sstream>
#include <osgDB/ConvertUTF>
#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>

//#include <dom/domLibrary_effects.h>
//#include <dom/domLibrary_materials.h>

#ifdef WIN32
#include "windows.h"
#endif

using namespace osgDAE;


void daeWriter::processMaterial( osg::StateSet *ss, domBind_material *pDomBindMaterial, const std::string &geoName )
{
    osg::ref_ptr<osg::StateSet> ssClean = CleanStateSet(ss); // Need to hold a ref to this or the materialMap.find() will delete it
    domBind_material::domTechnique_common *tc = daeSafeCast< domBind_material::domTechnique_common >( pDomBindMaterial->add( COLLADA_ELEMENT_TECHNIQUE_COMMON ) );
    domInstance_material *pDomInstanceMaterial = daeSafeCast< domInstance_material >( tc->add( COLLADA_ELEMENT_INSTANCE_MATERIAL ) );
    const std::string symbol( _pluginOptions.namesUseCodepage ? osgDB::convertStringFromCurrentCodePageToUTF8(geoName + "_material") : (geoName + "_material") );
    pDomInstanceMaterial->setSymbol( symbol.c_str() );

    // See if material already exists in cache
    MaterialMap::iterator iter = materialMap.find( ssClean );
    if ( iter != materialMap.end() )
    {
        std::string url = "#" + std::string( iter->second->getId() );
        pDomInstanceMaterial->setTarget( url.c_str() );
        return;
    }

    if ( lib_mats == NULL )
    {
        lib_mats = daeSafeCast< domLibrary_materials >( dom->add( COLLADA_ELEMENT_LIBRARY_MATERIALS ) );
    }

    domMaterial *mat = daeSafeCast< domMaterial >( lib_mats->add( COLLADA_ELEMENT_MATERIAL ) );
    std::string name( ssClean->getName() );
    if ( name.empty() )
    {
        name = "material";
    }
    name = uniquify( name );

    mat->setId( name.c_str() );

    std::string url = "#" + name;
    pDomInstanceMaterial->setTarget( url.c_str() );

    domInstance_effect *ie = daeSafeCast<domInstance_effect>( mat->add( COLLADA_ELEMENT_INSTANCE_EFFECT ) );

    if ( lib_effects == NULL )
    {
        lib_effects = daeSafeCast< domLibrary_effects >( dom->add( COLLADA_ELEMENT_LIBRARY_EFFECTS ) );
    }
    domEffect *effect = daeSafeCast< domEffect >( lib_effects->add( COLLADA_ELEMENT_EFFECT ) );
    std::string efName = name + "_effect";

    effect->setId( efName.c_str() );

    url = "#" + efName;
    ie->setUrl( url.c_str() );

    domProfile_COMMON *pc = daeSafeCast< domProfile_COMMON >( effect->add( COLLADA_ELEMENT_PROFILE_COMMON ) );
    domProfile_COMMON::domTechnique *pc_teq = daeSafeCast< domProfile_COMMON::domTechnique >( pc->add( COLLADA_ELEMENT_TECHNIQUE ) );
    pc_teq->setSid( "t0" );
    domProfile_COMMON::domTechnique::domPhong *phong = daeSafeCast< domProfile_COMMON::domTechnique::domPhong >( pc_teq->add( COLLADA_ELEMENT_PHONG ) );

    osg::Texture *tex = static_cast<osg::Texture*>(ssClean->getTextureAttribute( 0, osg::StateAttribute::TEXTURE ));
    if ( ssClean->getTextureAttribute( 1, osg::StateAttribute::TEXTURE ) != NULL )
    {
        tex = static_cast<osg::Texture*>(ssClean->getTextureAttribute( 1, osg::StateAttribute::TEXTURE ));
    }
    if ( tex != NULL && tex->getImage( 0 ) != NULL )
    {
        //TODO: Export all of the texture Attributes like wrap mode and all that jazz
        domImage *img = daeSafeCast< domImage >( pc->add( COLLADA_ELEMENT_IMAGE ) );
        std::string iName = efName + "-image";
        img->setId( iName.c_str() );

        osg::Image *osgimg = tex->getImage( 0 );

        domImage::domInit_from *imgif = daeSafeCast< domImage::domInit_from >( img->add( COLLADA_ELEMENT_INIT_FROM ) );

        std::string fileURI;
        if (_pluginOptions.linkOrignialTextures)
        {
            // We link to orignial images (not the ones in memory).
            fileURI = osgDB::findDataFile(osgimg->getFileName());
            if (fileURI=="" && _pluginOptions.forceTexture)
            {
                fileURI = osgDB::getRealPath(osgimg->getFileName());
            }
        }
        else
        {
            // We do not link to original images but to the ones in memory. Then must ensure to write the images.
            _externalWriter.write(*osgimg, _options, NULL, &fileURI);
        }

        fileURI = ReaderWriterDAE::ConvertFilePathToColladaCompatibleURI(fileURI);

        daeURI dd(*dae, fileURI);
        imgif->setValue( dd );
        // The document URI should contain the canonical path it was created with
        if (_pluginOptions.linkOrignialTextures) imgif->getValue().makeRelativeTo(doc->getDocumentURI());

        if (!_pluginOptions.earthTex)
        {
            domCommon_newparam_type *np = daeSafeCast< domCommon_newparam_type >( pc->add(COLLADA_ELEMENT_NEWPARAM) );
            std::string surfName = efName + "-surface";
            np->setSid( surfName.c_str() );
            domFx_surface_common *surface = daeSafeCast< domFx_surface_common >( np->add(COLLADA_ELEMENT_SURFACE) );
            domFx_surface_init_from_common *sif = daeSafeCast< domFx_surface_init_from_common >( surface->add(COLLADA_ELEMENT_INIT_FROM) );
            sif->setValue( iName.c_str() );
            surface->setType( FX_SURFACE_TYPE_ENUM_2D );

            np = daeSafeCast< domCommon_newparam_type >( pc->add( COLLADA_ELEMENT_NEWPARAM ) );
            std::string sampName = efName + "-sampler";
            np->setSid( sampName.c_str() );
            domFx_sampler2D_common *sampler = daeSafeCast< domFx_sampler2D_common >( np->add( COLLADA_ELEMENT_SAMPLER2D ) );
            domFx_sampler2D_common_complexType::domSource *source = daeSafeCast< domFx_sampler2D_common_complexType::domSource >( sampler->add( COLLADA_ELEMENT_SOURCE ) );
            source->setValue( surfName.c_str() );

            //set sampler state
            domFx_sampler2D_common_complexType::domWrap_s *wrap_s = daeSafeCast< domFx_sampler2D_common_complexType::domWrap_s >( sampler->add( COLLADA_ELEMENT_WRAP_S ) );
            osg::Texture::WrapMode wrap = tex->getWrap( osg::Texture::WRAP_S );
            switch( wrap )
            {
            case osg::Texture::CLAMP:
            case osg::Texture::CLAMP_TO_EDGE:
                wrap_s->setValue( FX_SAMPLER_WRAP_COMMON_CLAMP );
                break;
            case osg::Texture::CLAMP_TO_BORDER:
                wrap_s->setValue( FX_SAMPLER_WRAP_COMMON_BORDER );
                break;
            case osg::Texture::REPEAT:
                wrap_s->setValue( FX_SAMPLER_WRAP_COMMON_WRAP );
                break;
            case osg::Texture::MIRROR:
                wrap_s->setValue( FX_SAMPLER_WRAP_COMMON_MIRROR );
                break;
            default:
                wrap_s->setValue( FX_SAMPLER_WRAP_COMMON_NONE );
                break;
            }

            domFx_sampler2D_common_complexType::domWrap_t *wrap_t = daeSafeCast< domFx_sampler2D_common_complexType::domWrap_t >( sampler->add( COLLADA_ELEMENT_WRAP_T ) );
            wrap = tex->getWrap( osg::Texture::WRAP_T );
            switch( wrap )
            {
            case osg::Texture::CLAMP:
            case osg::Texture::CLAMP_TO_EDGE:
                wrap_t->setValue( FX_SAMPLER_WRAP_COMMON_CLAMP );
                break;
            case osg::Texture::CLAMP_TO_BORDER:
                wrap_t->setValue( FX_SAMPLER_WRAP_COMMON_BORDER );
                break;
            case osg::Texture::REPEAT:
                wrap_t->setValue( FX_SAMPLER_WRAP_COMMON_WRAP );
                break;
            case osg::Texture::MIRROR:
                wrap_t->setValue( FX_SAMPLER_WRAP_COMMON_MIRROR );
                break;
            default:
                wrap_t->setValue( FX_SAMPLER_WRAP_COMMON_NONE );
                break;
            }

            const osg::Vec4 &bcol = tex->getBorderColor();
            domFx_sampler2D_common_complexType::domBorder_color *dbcol = daeSafeCast< domFx_sampler2D_common_complexType::domBorder_color >( sampler->add( COLLADA_ELEMENT_BORDER_COLOR ) );
            dbcol->getValue().append( bcol.r() );
            dbcol->getValue().append( bcol.g() );
            dbcol->getValue().append( bcol.b() );
            dbcol->getValue().append( bcol.a() );

            domFx_sampler2D_common_complexType::domMinfilter *minfilter = daeSafeCast< domFx_sampler2D_common_complexType::domMinfilter >( sampler->add( COLLADA_ELEMENT_MINFILTER ) );
            osg::Texture::FilterMode mode = tex->getFilter( osg::Texture::MIN_FILTER );
            switch( mode )
            {
            case osg::Texture::LINEAR:
                minfilter->setValue( FX_SAMPLER_FILTER_COMMON_LINEAR );
                break;
            case osg::Texture::LINEAR_MIPMAP_LINEAR:
                minfilter->setValue( FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR );
                break;
            case osg::Texture::LINEAR_MIPMAP_NEAREST:
                minfilter->setValue( FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_NEAREST );
                break;
            case osg::Texture::NEAREST:
                minfilter->setValue( FX_SAMPLER_FILTER_COMMON_NEAREST );
                break;
            case osg::Texture::NEAREST_MIPMAP_LINEAR:
                minfilter->setValue( FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_LINEAR );
                break;
            case osg::Texture::NEAREST_MIPMAP_NEAREST:
                minfilter->setValue( FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_NEAREST );
                break;
            }

            domFx_sampler2D_common_complexType::domMagfilter *magfilter = daeSafeCast< domFx_sampler2D_common_complexType::domMagfilter >( sampler->add( COLLADA_ELEMENT_MAGFILTER ) );
            mode = tex->getFilter( osg::Texture::MAG_FILTER );
            switch( mode )
            {
            case osg::Texture::LINEAR:
                magfilter->setValue( FX_SAMPLER_FILTER_COMMON_LINEAR );
                break;
            case osg::Texture::LINEAR_MIPMAP_LINEAR:
                magfilter->setValue( FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_LINEAR );
                break;
            case osg::Texture::LINEAR_MIPMAP_NEAREST:
                magfilter->setValue( FX_SAMPLER_FILTER_COMMON_LINEAR_MIPMAP_NEAREST );
                break;
            case osg::Texture::NEAREST:
                magfilter->setValue( FX_SAMPLER_FILTER_COMMON_NEAREST );
                break;
            case osg::Texture::NEAREST_MIPMAP_LINEAR:
                magfilter->setValue( FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_LINEAR );
                break;
            case osg::Texture::NEAREST_MIPMAP_NEAREST:
                magfilter->setValue( FX_SAMPLER_FILTER_COMMON_NEAREST_MIPMAP_NEAREST );
                break;
            }


            domCommon_color_or_texture_type *cot = daeSafeCast< domCommon_color_or_texture_type >( phong->add( COLLADA_ELEMENT_DIFFUSE ) );
            domCommon_color_or_texture_type_complexType::domTexture *dtex = daeSafeCast< domCommon_color_or_texture_type_complexType::domTexture >( cot->add( COLLADA_ELEMENT_TEXTURE ) );
            dtex->setTexture( sampName.c_str() );
            dtex->setTexcoord( "texcoord0" );
        }
        else
        {
            // Earth tex
            domCommon_color_or_texture_type *cot = daeSafeCast< domCommon_color_or_texture_type >( phong->add( COLLADA_ELEMENT_DIFFUSE ) );
            domCommon_color_or_texture_type_complexType::domTexture *dtex = daeSafeCast< domCommon_color_or_texture_type_complexType::domTexture >( cot->add( COLLADA_ELEMENT_TEXTURE ) );
            dtex->setTexture( iName.c_str() );
            dtex->setTexcoord( "texcoord0" );
        }

        domInstance_material::domBind_vertex_input *bvi = daeSafeCast< domInstance_material::domBind_vertex_input >( pDomInstanceMaterial->add( COLLADA_ELEMENT_BIND_VERTEX_INPUT ) );
        bvi->setSemantic( "texcoord0" );
        bvi->setInput_semantic( COMMON_PROFILE_INPUT_TEXCOORD );
        bvi->setInput_set( 0 );
    }
    osg::Material *osgmat = static_cast<osg::Material*>(ssClean->getAttribute( osg::StateAttribute::MATERIAL ));
    if ( osgmat != NULL )
    {
        const osg::Vec4 &eCol = osgmat->getEmissionFrontAndBack()?osgmat->getEmission( osg::Material::FRONT_AND_BACK ):osgmat->getEmission( osg::Material::FRONT );
        const osg::Vec4 &aCol = osgmat->getAmbientFrontAndBack()?osgmat->getAmbient( osg::Material::FRONT_AND_BACK ):osgmat->getAmbient( osg::Material::FRONT );
        const osg::Vec4 &dCol = osgmat->getDiffuseFrontAndBack()?osgmat->getDiffuse( osg::Material::FRONT_AND_BACK ):osgmat->getDiffuse( osg::Material::FRONT );
        const osg::Vec4 &sCol = osgmat->getSpecularFrontAndBack()?osgmat->getSpecular( osg::Material::FRONT_AND_BACK ):osgmat->getSpecular( osg::Material::FRONT );
        float shininess = osgmat->getShininessFrontAndBack()?osgmat->getShininess( osg::Material::FRONT_AND_BACK ):osgmat->getShininess( osg::Material::FRONT );

        domCommon_color_or_texture_type *cot = daeSafeCast< domCommon_color_or_texture_type >( phong->add( COLLADA_ELEMENT_EMISSION ) );
        domCommon_color_or_texture_type_complexType::domColor *col = daeSafeCast< domCommon_color_or_texture_type_complexType::domColor >( cot->add( COLLADA_ELEMENT_COLOR ) );
        col->getValue().append( eCol.r() );
        col->getValue().append( eCol.g() );
        col->getValue().append( eCol.b() );
        col->getValue().append( eCol.a() );

        cot = daeSafeCast< domCommon_color_or_texture_type >( phong->add( COLLADA_ELEMENT_AMBIENT ) );
        col = daeSafeCast< domCommon_color_or_texture_type_complexType::domColor >( cot->add( COLLADA_ELEMENT_COLOR ) );
        col->getValue().append( aCol.r() );
        col->getValue().append( aCol.g() );
        col->getValue().append( aCol.b() );
        col->getValue().append( aCol.a() );


        //### check if we really have a texture
        if ( phong->getDiffuse() == NULL )
        {
            cot = daeSafeCast< domCommon_color_or_texture_type >( phong->add( COLLADA_ELEMENT_DIFFUSE ) );
            col = daeSafeCast< domCommon_color_or_texture_type_complexType::domColor >( cot->add( COLLADA_ELEMENT_COLOR ) );
            col->getValue().append( dCol.r() );
            col->getValue().append( dCol.g() );
            col->getValue().append( dCol.b() );
            col->getValue().append( dCol.a() );
        }
        else
        {
            cot = phong->getDiffuse();

            if (_pluginOptions.writeExtras)
            {
                // Adds the following to a texture element

                //<extra type="color">
                //    <technique profile="SCEI">
                //        <color>1.0 1.0 1.0 1.0</color>
                //    </technique>
                //</extra>

                domCommon_color_or_texture_type_complexType::domTexture *dtex = cot->getTexture();
                domExtra *extra = daeSafeCast< domExtra >( dtex->add( COLLADA_ELEMENT_EXTRA ) );
                extra->setType( "color" );
                domTechnique *teq = daeSafeCast< domTechnique >( extra->add( COLLADA_ELEMENT_TECHNIQUE ) );
                teq->setProfile( "SCEI" );
                domAny *any = (domAny*)(daeElement*)teq->add( COLLADA_ELEMENT_COLOR );

                std::ostringstream colVal;
                colVal << dCol.r() << " " << dCol.g() << " " << dCol.b() << " " << dCol.a();
                any->setValue( colVal.str().c_str() );
            }
        }

        cot = daeSafeCast< domCommon_color_or_texture_type >( phong->add( COLLADA_ELEMENT_SPECULAR ) );
        col = daeSafeCast< domCommon_color_or_texture_type_complexType::domColor >( cot->add( COLLADA_ELEMENT_COLOR ) );
        col->getValue().append( sCol.r() );
        col->getValue().append( sCol.g() );
        col->getValue().append( sCol.b() );
        col->getValue().append( sCol.a() );

        domCommon_float_or_param_type *fop = daeSafeCast< domCommon_float_or_param_type >( phong->add( COLLADA_ELEMENT_SHININESS ) );
        domCommon_float_or_param_type_complexType::domFloat *f = daeSafeCast< domCommon_float_or_param_type_complexType::domFloat >( fop->add(COLLADA_ELEMENT_FLOAT) );
        f->setValue( shininess );
    }

    // Do common transparency stuff here
    // if (osg::StateSet::TRANSPARENT_BIN == ssClean->getRenderingHint()) cannot do this at the moment because stateSet::merge does does not handle the hints
    if (osg::StateSet::TRANSPARENT_BIN == m_CurrentRenderingHint)
    {
        osg::BlendFunc *pBlendFunc = static_cast< osg::BlendFunc * >( ssClean->getAttribute( osg::StateAttribute::BLENDFUNC ) );
        osg::BlendColor *pBlendColor = static_cast< osg::BlendColor * >( ssClean->getAttribute( osg::StateAttribute::BLENDCOLOR ) );
        if (pBlendFunc != NULL)
        {
            if (pBlendColor != NULL)
            {
                if ((GL_CONSTANT_ALPHA == pBlendFunc->getSource()) && (GL_ONE_MINUS_CONSTANT_ALPHA == pBlendFunc->getDestination()))
                {
                    // A_ONE opaque mode
                    domCommon_transparent_type *pTransparent = daeSafeCast<domCommon_transparent_type>(phong->add(COLLADA_ELEMENT_TRANSPARENT));
                    pTransparent->setOpaque(FX_OPAQUE_ENUM_A_ONE);
                    domCommon_color_or_texture_type_complexType::domColor *pColor = daeSafeCast<domCommon_color_or_texture_type_complexType::domColor>(pTransparent->add(COLLADA_ELEMENT_COLOR));
                    domCommon_float_or_param_type *pFop = daeSafeCast<domCommon_float_or_param_type>(phong->add(COLLADA_ELEMENT_TRANSPARENCY));
                    domCommon_float_or_param_type_complexType::domFloat *pTransparency = daeSafeCast<domCommon_float_or_param_type_complexType::domFloat>(pFop->add(COLLADA_ELEMENT_FLOAT));
                    if (_pluginOptions.googleMode)
                    {
                        pColor->getValue().append(1.0);
                        pColor->getValue().append(1.0);
                        pColor->getValue().append(1.0);
                        pColor->getValue().append(1.0);
                        pTransparency->setValue(1.0 - pBlendColor->getConstantColor().a());
                    }
                    else
                    {
                        pColor->getValue().append(pBlendColor->getConstantColor().r());
                        pColor->getValue().append(pBlendColor->getConstantColor().g());
                        pColor->getValue().append(pBlendColor->getConstantColor().b());
                        pColor->getValue().append(pBlendColor->getConstantColor().a());
                        pTransparency->setValue(1.0);
                    }
                }
                else if ((GL_ONE_MINUS_CONSTANT_COLOR == pBlendFunc->getSource()) && (GL_CONSTANT_COLOR == pBlendFunc->getDestination()))
                {
                    // RGB_ZERO opaque mode
                    domCommon_transparent_type *pTransparent = daeSafeCast<domCommon_transparent_type>(phong->add(COLLADA_ELEMENT_TRANSPARENT));
                    pTransparent->setOpaque(FX_OPAQUE_ENUM_RGB_ZERO);
                    domCommon_color_or_texture_type_complexType::domColor *pColor = daeSafeCast<domCommon_color_or_texture_type_complexType::domColor>(pTransparent->add(COLLADA_ELEMENT_COLOR));
                    pColor->getValue().append(pBlendColor->getConstantColor().r());
                    pColor->getValue().append(pBlendColor->getConstantColor().g());
                    pColor->getValue().append(pBlendColor->getConstantColor().b());
                    pColor->getValue().append(pBlendColor->getConstantColor().a());
                    domCommon_float_or_param_type *pFop = daeSafeCast<domCommon_float_or_param_type>(phong->add(COLLADA_ELEMENT_TRANSPARENCY));
                    domCommon_float_or_param_type_complexType::domFloat *pTransparency = daeSafeCast<domCommon_float_or_param_type_complexType::domFloat>(pFop->add(COLLADA_ELEMENT_FLOAT));
                    pTransparency->setValue(1.0);
                }
                else
                    OSG_WARN << "Unsupported BlendFunction parameters in transparency processing." << std::endl;
            }
            else if (tex != NULL && tex->getImage( 0 ) != NULL)
            {
                domCommon_transparent_type *ctt = daeSafeCast< domCommon_transparent_type >( phong->add(COLLADA_ELEMENT_TRANSPARENT) );
                ctt->setOpaque( FX_OPAQUE_ENUM_A_ONE );
                domCommon_color_or_texture_type_complexType::domTexture * dtex = daeSafeCast< domCommon_color_or_texture_type_complexType::domTexture >( ctt->add(COLLADA_ELEMENT_TEXTURE) );

                if (!_pluginOptions.earthTex)
                {
                    std::string sampName = efName + "-sampler";
                    dtex->setTexture( sampName.c_str() );
                }
                else
                {
                    std::string iName = efName + "-image";
                    dtex->setTexture( iName.c_str() );
                }
                dtex->setTexcoord( "texcoord0" );
            }
            else
            {
                OSG_WARN << "Transparency processing - No texture or BlendColor." << std::endl;
            }
        }
        else
        {
            OSG_WARN << "Transparency processing - BlendFunction not found." << std::endl;
        }
    }

    if (_pluginOptions.writeExtras)
    {
        // Adds the following to a Profile_COMMON element

        //<extra>
        //    <technique profile="GOOGLEEARTH">
        //        <double_sided>0</double_sided>
        //    </technique>
        //</extra>

        // Process GOOGLE one sided stuff here
        if (osg::StateAttribute::INHERIT != ssClean->getMode(GL_CULL_FACE))
        {
            domExtra *pExtra = daeSafeCast<domExtra>(pc->add(COLLADA_ELEMENT_EXTRA));
            domTechnique *pTechnique = daeSafeCast<domTechnique>(pExtra->add( COLLADA_ELEMENT_TECHNIQUE ) );
            pTechnique->setProfile("GOOGLEEARTH");
            domAny *pAny = (domAny*)(daeElement*)pTechnique->add("double_sided");
            if (GL_FALSE == ssClean->getMode(GL_CULL_FACE))
                pAny->setValue("1");
            else
                pAny->setValue("0");
        }
    }

    materialMap.insert( std::make_pair( ssClean, mat ) );
}

osg::StateSet* daeWriter::CleanStateSet(osg::StateSet* pStateSet) const
{
    // TODO - clean out all the the attributes and modes not used to define materials
    osg::StateSet* pCleanedStateSet = new osg::StateSet;
    pCleanedStateSet->setTextureAttributeList(pStateSet->getTextureAttributeList());
    if (NULL != pStateSet->getAttribute(osg::StateAttribute::BLENDFUNC))
        pCleanedStateSet->setAttribute(pStateSet->getAttribute(osg::StateAttribute::BLENDFUNC));
    if (NULL != pStateSet->getAttribute(osg::StateAttribute::BLENDCOLOR))
        pCleanedStateSet->setAttribute(pStateSet->getAttribute(osg::StateAttribute::BLENDCOLOR));
    if (NULL != pStateSet->getAttribute(osg::StateAttribute::MATERIAL))
        pCleanedStateSet->setAttribute(pStateSet->getAttribute(osg::StateAttribute::MATERIAL));
    // pCleanedStateSet->setRenderingHint(pStateSet->getRenderingHint()); does not work at the moment due to stateSet::merge
    if (osg::StateAttribute::ON != pStateSet->getMode(GL_CULL_FACE))
        pCleanedStateSet->setMode(GL_CULL_FACE, pStateSet->getMode(GL_CULL_FACE));
    return pCleanedStateSet;
}


