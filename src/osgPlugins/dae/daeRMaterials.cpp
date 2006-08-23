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
#include <osgDB/Registry>
#include <osgDB/ReadFile>

using namespace osgdae;

void daeReader::processBindMaterial( domBind_material *bm, osg::Node *geo )
{
    if ( bm->getTechnique_common() == NULL )
    {
        osg::notify( osg::WARN ) << "No COMMON technique for bind_material" << std::endl;
        return;
    }
    osg::Group *group = geo->asGroup();
    if ( group == NULL )
    {
        //this shouldn't happen unless something is terribly wrong
        return;
    }
    domInstance_material_Array &ima = bm->getTechnique_common()->getInstance_material_array();
    size_t count = ima.getCount();
    for ( size_t i = 0; i < count; i++ )
    {
        std::string symbol = ima[i]->getSymbol();
        domMaterial *mat = daeSafeCast< domMaterial >( getElementFromURI( ima[i]->getTarget() ) );
        if ( mat == NULL ) 
        {
            osg::notify( osg::WARN ) << "Failed to locate material " << ima[i]->getTarget().getURI() << std::endl;
            continue;
        }
        osg::StateSet *ss;
        //check material cache if this material already exists
        std::map< domMaterial*, osg::StateSet*>::iterator iter = materialMap.find( mat );
        if ( iter != materialMap.end() )
        {
            ss = iter->second;
        }
        else
        {
            ss = processMaterial( mat );
            materialMap.insert( std::make_pair( mat, ss ) );
        }
        if ( ss == NULL )
        {
            continue;
        }
        //TODO: process all of the <bind>s and <bind_vertex_input>s that are here in the instance_material.

        for ( unsigned int x = 0; x < group->getNumChildren(); x++ )
        {
            //I named the geode with the material symbol so I can do this check for binding
            if ( group->getChild( x )->getName() == symbol )
            {
                /*if ( group->getChild( x )->getStateSet() != NULL )
                {
                    //already have a stateSet this means I am an instance so clone me.
                    group->replaceChild( group->getChild( x ), (osg::Node*)group->getChild( x )->cloneType() );
                }*/
                group->getChild( x )->setStateSet( ss );
            }
        }
    }
}

osg::StateSet *daeReader::processMaterial( domMaterial *mat )
{
    domEffect *effect = daeSafeCast< domEffect >( getElementFromURI( mat->getInstance_effect()->getUrl() ) );
    if ( effect == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to locate effect " << mat->getInstance_effect()->getUrl().getURI() << std::endl;
        return NULL;
    }
    osg::StateSet *ss = processEffect( effect );
    
    //TODO: process all of the setParams that could happen here in the material. ESP. the textures

    return ss;
}

osg::StateSet *daeReader::processEffect( domEffect *effect )
{
    bool hasCOMMON = false;
    osg::StateSet *ss = NULL;

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
            ss = processProfileCOMMON( pc );
            hasCOMMON = true;
            continue;
        }

        osg::notify( osg::WARN ) << "unsupported effect profile " << effect->getFx_profile_abstract_array()[i]->getTypeName() << std::endl;
    }

    return ss;
}

osg::StateSet *daeReader::processProfileCOMMON( domProfile_COMMON *pc )
{
    osg::StateSet *ss = new osg::StateSet();
    
    domProfile_COMMON::domTechnique *teq = pc->getTechnique();

    domProfile_COMMON::domTechnique::domConstant *c = teq->getConstant();
    domProfile_COMMON::domTechnique::domLambert *l = teq->getLambert();
    domProfile_COMMON::domTechnique::domPhong *p = teq->getPhong();
    domProfile_COMMON::domTechnique::domBlinn *b = teq->getBlinn();

    ss->setMode( GL_CULL_FACE, GL_TRUE );
    //ss->setMode( GL_LIGHTING, GL_FALSE );

    osg::ref_ptr< osg::Material > mat = new osg::Material();
    bool insertMat = false;
    if ( b != NULL )
    {
        bool tmp;
        tmp = processColorOrTextureType( b->getEmission(), osg::Material::EMISSION, mat.get() );
        insertMat = insertMat || tmp;
        
        tmp = processColorOrTextureType( b->getAmbient(), osg::Material::AMBIENT, mat.get() );
        insertMat = insertMat || tmp;
        
        osg::StateAttribute *sa = NULL;
        tmp = processColorOrTextureType( b->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &sa );
        insertMat = insertMat || tmp;
        if ( sa != NULL ) 
        {
            ss->setTextureMode( 0, GL_TEXTURE_2D, GL_TRUE );
            ss->setTextureAttribute( 0, sa );
        }

        tmp = processColorOrTextureType( b->getSpecular(), osg::Material::SPECULAR, mat.get(), b->getShininess() );
        insertMat = insertMat || tmp;

        osg::StateAttribute *sa2 = NULL;
        sa2 = processTransparentType( b->getTransparent(), ss );
        if ( sa2 != NULL )
        {
            if ( sa == NULL )
            {
                ss->setTextureMode( 0, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( 0, sa2 );
            }
            else
            {
                osg::notify( osg::WARN ) << "Already have a texture in the diffuse channel" << std::endl;
            }
        }

    }
    else if ( p != NULL )
    {
        bool tmp;
        tmp = processColorOrTextureType( p->getEmission(), osg::Material::EMISSION, mat.get() );
        insertMat = insertMat || tmp;
        
        tmp = processColorOrTextureType( p->getAmbient(), osg::Material::AMBIENT, mat.get() );
        insertMat = insertMat || tmp;
        
        osg::StateAttribute *sa = NULL;
        tmp = processColorOrTextureType( p->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &sa );
        insertMat = insertMat || tmp;
        if ( sa != NULL ) 
        {
            ss->setTextureMode( 0, GL_TEXTURE_2D, GL_TRUE );
            ss->setTextureAttribute( 0, sa );
        }
        
        tmp = processColorOrTextureType( p->getSpecular(), osg::Material::SPECULAR, mat.get(), p->getShininess() );
        insertMat = insertMat || tmp;

        osg::StateAttribute *sa2 = NULL;
        sa2 = processTransparentType( p->getTransparent(), ss );
        if ( sa2 != NULL )
        {
            if ( sa == NULL )
            {
                ss->setTextureMode( 0, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( 0, sa2 );
            }
            else
            {
                osg::notify( osg::WARN ) << "Already have a texture in the diffuse channel" << std::endl;
            }
        }

    }
    else if ( l != NULL )
    {
        bool tmp;
        tmp = processColorOrTextureType( l->getEmission(), osg::Material::EMISSION, mat.get() );
        insertMat = insertMat || tmp;
        
        tmp = processColorOrTextureType( l->getAmbient(), osg::Material::AMBIENT, mat.get() );
        insertMat = insertMat || tmp;
        
        osg::StateAttribute *sa = NULL;
        tmp = processColorOrTextureType( l->getDiffuse(), osg::Material::DIFFUSE, mat.get(), NULL, &sa );
        insertMat = insertMat || tmp;
        if ( sa != NULL ) 
        {
            ss->setTextureMode( 0, GL_TEXTURE_2D, GL_TRUE );
            ss->setTextureAttribute( 0, sa );
        }

        osg::StateAttribute *sa2 = NULL;
        sa2 = processTransparentType( l->getTransparent(), ss );
        if ( sa2 != NULL )
        {
            if ( sa == NULL )
            {
                ss->setTextureMode( 0, GL_TEXTURE_2D, GL_TRUE );
                ss->setTextureAttribute( 0, sa2 );
            }
            else
            {
                osg::notify( osg::WARN ) << "Already have a texture in the diffuse channel" << std::endl;
            }
        }
        
    }
    else if ( c != NULL )
    {
        insertMat = processColorOrTextureType( c->getEmission(), osg::Material::EMISSION, mat.get() );

        osg::StateAttribute *sa2 = NULL;
        sa2 = processTransparentType( c->getTransparent(), ss );
        if ( sa2 != NULL )
        {
            ss->setTextureMode( 0, GL_TEXTURE_2D, GL_TRUE );
            ss->setTextureAttribute( 0, sa2 );
        }
    }
    if ( insertMat )
    {
        ss->setAttribute( mat.get() );
    }

    return ss;
}

bool daeReader::processColorOrTextureType( domCommon_color_or_texture_type *cot, 
osg::Material::ColorMode channel,
osg::Material *mat,
domCommon_float_or_param_type *fop,
osg::StateAttribute **sa )
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
        else
        {
            osg::notify( osg::WARN ) << "Currently no support for <param> or <texture> in Emission channel " << std::endl;
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
        else
        {
            osg::notify( osg::WARN ) << "Currently no support for <param> or <texture> in Ambient channel " << std::endl;
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
        else if ( cot->getTexture() != NULL && sa != NULL )
        {
            *sa = processTexture( cot->getTexture() );
            domExtra *extra = cot->getTexture()->getExtra();
            if ( extra != NULL && extra->getType() != NULL && 
                 strcmp( extra->getType(), "color" ) == 0 )
            {
                //the extra data for osg. Diffuse color can happen with a texture.
                for ( unsigned int i = 0; i < extra->getTechnique_array().getCount(); i++ )
                {
                    domTechnique *teq = extra->getTechnique_array()[i];
                    if ( strcmp( teq->getProfile(), "SCEI" ) == 0 )
                    {
                        osg::Vec4 col;
                        domAny *dcol = (domAny*)(daeElement*)teq->getContents()[0];
                        char *val = (char *)dcol->getValue();
                        int cnt = 0;
                        while ( cnt < 4 && strlen( val ) != 0 )
                        {
                            char *space = strchr( val, ' ' );
                            if ( space != NULL )
                            {
                                *space = 0;
                            }
                            int tmp = atoi( val );
                            col[cnt] = tmp;
                            cnt++;
                            val = space + 1;                            
                        }
                        mat->setDiffuse( osg::Material::FRONT_AND_BACK, col );
                        retVal = true;
                        break;
                    }
                }
            }
        }
        else
        {
            osg::notify( osg::WARN ) << "Currently no support for <param> in Diffuse channel " << std::endl;
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
        else
        {
            osg::notify( osg::WARN ) << "Currently no support for <param> or <texture> in Specular channel " << std::endl;
        }
        if ( fop != NULL && fop->getFloat() != NULL )
        {
            mat->setShininess( osg::Material::FRONT_AND_BACK, fop->getFloat()->getValue() );
            retVal = true;
        }
    }

    return retVal;
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
    dImg->getInit_from()->getValue().validate();

    std::string filename = (dImg->getInit_from()->getValue().getURI()+7);
    osg::Image *img = osgDB::readImageFile( filename.c_str() );

    osg::notify(osg::INFO)<<"  processTexture(..) - readImage("<<filename<<")"<<std::endl;

    osg::Texture2D *t2D = new osg::Texture2D( img );
    //set texture parameters
    if ( sampler != NULL )
    {
        if ( sampler->getWrap_s() != NULL )
        {
            osg::Texture::WrapMode wrap;
            switch( sampler->getWrap_s()->getValue() )
            {
            case FX_SAMPLER_WRAP_COMMON_NONE:
            case FX_SAMPLER_WRAP_COMMON_WRAP:
                wrap = osg::Texture::REPEAT;
                break;
            case FX_SAMPLER_WRAP_COMMON_MIRROR:
                wrap = osg::Texture::MIRROR;
                break;
            case FX_SAMPLER_WRAP_COMMON_CLAMP:
                wrap = osg::Texture::CLAMP;
                break;
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
            case FX_SAMPLER_WRAP_COMMON_NONE:
            case FX_SAMPLER_WRAP_COMMON_WRAP:
                wrap = osg::Texture::REPEAT;
                break;
            case FX_SAMPLER_WRAP_COMMON_MIRROR:
                wrap = osg::Texture::MIRROR;
                break;
            case FX_SAMPLER_WRAP_COMMON_CLAMP:
                wrap = osg::Texture::CLAMP;
                break;
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

    return t2D;
}

osg::StateAttribute *daeReader::processTransparentType( domCommon_transparent_type *ctt, osg::StateSet *ss )
{
    if ( ctt == NULL )
    {
        return false;
    }
//  bool retVal = false;
    osg::StateAttribute *sa = NULL;
    if ( ctt->getColor() != NULL )
    {
        domFloat4 &f4 = ctt->getColor()->getValue();
        //##compliant with OSG 1.0 API
        osg::BlendColor *bc = new osg::BlendColor();
        bc->setConstantColor(osg::Vec4( f4[0], f4[1], f4[2], f4[3] ));
        ss->setAttribute( bc );
        ss->setMode( GL_BLEND, GL_TRUE );
        ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        ss->setRenderBinDetails( 10, "DepthSortedBin" );
    }
    else if ( ctt->getTexture() != NULL )
    {
        sa = processTexture( ctt->getTexture() );
        osg::BlendFunc *bf = new osg::BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        ss->setAttribute( bf );
        ss->setMode( GL_BLEND, GL_TRUE );
        ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        ss->setRenderBinDetails( 10, "DepthSortedBin" );
    }
    return sa;
}
