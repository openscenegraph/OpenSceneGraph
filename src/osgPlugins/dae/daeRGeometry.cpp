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
#include "domSourceReader.h"
#include <dae.h>
#include <dom/domCOLLADA.h>
#include <dom/domInstance_geometry.h>
#include <dom/domInstance_controller.h>
#include <dom/domController.h>
#include <osg/StateSet>

#include <osg/Geometry>

using namespace osgdae;

osg::Geode* daeReader::processInstanceGeometry( domInstance_geometry *ig )
{
    daeElement *el = getElementFromURI( ig->getUrl() );
    domGeometry *geom = daeSafeCast< domGeometry >( el );
    if ( geom == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to locate geometry " << ig->getUrl().getURI() << std::endl;
        return NULL;
    }
    
    // Check cache if geometry already exists
    osg::Geode *geode;

    domGeometryGeodeMap::iterator iter = geometryMap.find( geom );
    if ( iter != geometryMap.end() )
    {
        osg::Geode* cachedGeode = iter->second;

        // Create a copy of the cached Geode with a copy of the drawables,
        // because the may be using a different material.
        // TODO Cloning is not necessary if the material layouts are exactly the same.
        // To check this we need to compare the material bindings used by the cached Geode
        // and this new instance_geometry material bindings.
        geode = static_cast<osg::Geode*>(cachedGeode->clone(osg::CopyOp::DEEP_COPY_DRAWABLES));
    }
    else
    {
        geode = processGeometry( geom );
        geometryMap.insert( std::make_pair( geom, geode ) );
    }

    if ( geode == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to load geometry " << ig->getUrl().getURI() << std::endl;
        return NULL;
    }

    // process material bindings
    if ( ig->getBind_material() != NULL )
    {
        processBindMaterial( ig->getBind_material(), geom, geode );
    }

    return geode;
}

// <controller>
// attributes:
// id, name
// elements:
// 0..1 <asset>
// 1    <skin>, <morph>
// 0..* <extra>
osg::Geode* daeReader::processInstanceController( domInstance_controller *ictrl )
{
    //TODO: support skinning
    daeElement *el = getElementFromURI( ictrl->getUrl());
    domController *ctrl = daeSafeCast< domController >( el );
    if ( ctrl == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to locate conroller " << ictrl->getUrl().getURI() << std::endl;
        return NULL;
    }

    osg::notify( osg::WARN ) << "Processing <controller>. There is not skinning support but will display the base mesh." << std::endl;

    el = NULL;
    //## non init
    daeURI *src=NULL;
    if ( ctrl->getSkin() != NULL )
    {
        src = &ctrl->getSkin()->getSource();
        el = getElementFromURI( ctrl->getSkin()->getSource() );
    }
    else if ( ctrl->getMorph() != NULL )
    {
        src = &ctrl->getSkin()->getSource();
        el = getElementFromURI( ctrl->getMorph()->getSource() );
    }
    
    //non init case
    if ( !src )
    {
        osg::notify( osg::WARN ) << "Failed to locate geometry : URI is NULL" << std::endl;
        return NULL;
    }
        
    domGeometry *geom = daeSafeCast< domGeometry >( el );
    if ( geom == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to locate geometry " << src->getURI() << std::endl;
        return NULL;
    }

    // Check cache if geometry already exists
    osg::Geode *geode;

    domGeometryGeodeMap::iterator iter = geometryMap.find( geom );
    if ( iter != geometryMap.end() )
    {
        osg::Geode* cachedGeode = iter->second;

        // Create a copy of the cached Geode with a copy of the drawables,
        // because the may be using a different material.
        // TODO Cloning is not necessary if the material layouts are exactly the same.
        // To check this we need to compare the material bindings used by the cached Geode
        // and this new instance_geometry material bindings.
        geode = static_cast<osg::Geode*>(cachedGeode->clone(osg::CopyOp::DEEP_COPY_DRAWABLES));
    }
    else
    {
        geode = processGeometry( geom );
        geometryMap.insert( std::make_pair( geom, geode ) );
    }

    if ( geode == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to load geometry " << src->getURI() << std::endl;
        return NULL;
    }
    //process material bindings
    if ( ictrl->getBind_material() != NULL )
    {
        processBindMaterial( ictrl->getBind_material(), geom, geode );
    }

    return geode;
}

// <geometry>
// attributes:
// id, name
// elements:
// 0..1 <asset>
// 1    <convex_mesh>, <mesh>, <spline>
// 0..* <extra>
osg::Geode *daeReader::processGeometry( domGeometry *geo )
{
    domMesh *mesh = geo->getMesh();
    if ( mesh == NULL )
    {
        osg::notify( osg::WARN ) << "Unsupported Geometry type loading " << geo->getId() << std::endl;
        return NULL;
    }

    osg::Geode* geode = new osg::Geode;
    if (geo->getId() != NULL )
    {
        geode->setName( geo->getId() );
    }

    // <mesh>
    // elements:
    // 1..* <source>
    // 1    <vertices>
    // 0..*    <lines>, <linestrips>, <polygons>, <polylist>, <triangles>, <trifans>, <tristrips>
    // 0..* <extra>
    size_t count = mesh->getContents().getCount();
    
    // 1..* <source>
    SourceMap sources;
    domSource_Array sourceArray = mesh->getSource_array();
    for ( size_t i = 0; i < sourceArray.getCount(); i++)
    {
        sources.insert( std::make_pair((daeElement*)sourceArray[i], domSourceReader( sourceArray[i] ) ) ); 
    }

    // 0..*    <lines>
    domLines_Array linesArray = mesh->getLines_array();
    for ( size_t i = 0; i < linesArray.getCount(); i++)
    {
        processSinglePPrimitive<domLines>(geode, linesArray[i], sources, GL_LINES );
    }

    // 0..*    <linestrips>
    domLinestrips_Array linestripsArray = mesh->getLinestrips_array();
    for ( size_t i = 0; i < linestripsArray.getCount(); i++)
    {
        processMultiPPrimitive<domLinestrips>(geode, linestripsArray[i], sources, GL_LINE_STRIP );
    }

    // 0..* <polygons>
    domPolygons_Array polygonsArray = mesh->getPolygons_array();
    for ( size_t i = 0; i < polygonsArray.getCount(); i++)
    {
        processMultiPPrimitive<domPolygons>(geode, polygonsArray[i], sources, GL_POLYGON );
    }

    // 0..* <polylist>
    domPolylist_Array polylistArray = mesh->getPolylist_array();
    for ( size_t i = 0; i < polylistArray.getCount(); i++)
    {
        processPolylist(geode, polylistArray[i], sources );
    }

    // 0..* <triangles>
    domTriangles_Array trianglesArray = mesh->getTriangles_array();
    for ( size_t i = 0; i < trianglesArray.getCount(); i++)
    {
        processSinglePPrimitive<domTriangles>(geode, trianglesArray[i], sources, GL_TRIANGLES );
    }

    // 0..* <trifans>
    domTrifans_Array trifansArray = mesh->getTrifans_array();
    for ( size_t i = 0; i < trifansArray.getCount(); i++)
    {
        processMultiPPrimitive<domTrifans>(geode, trifansArray[i], sources, GL_TRIANGLE_FAN );
    }

    // 0..* <tristrips>
    domTristrips_Array tristripsArray = mesh->getTristrips_array();
    for ( size_t i = 0; i < tristripsArray.getCount(); i++)
    {
        processMultiPPrimitive<domTristrips>(geode, tristripsArray[i], sources, GL_TRIANGLE_STRIP );
    }

    return geode;
}


template< typename T >
void daeReader::processSinglePPrimitive(osg::Geode* geode, T *group, SourceMap &sources, GLenum mode )
{
    osg::Geometry *geometry = new osg::Geometry();
    geometry->setName(group->getMaterial());

    IndexMap index_map;
    resolveArrays( group->getInput_array(), geometry, sources, index_map );

    osg::DrawArrayLengths* dal = new osg::DrawArrayLengths( mode );
    processP( group->getP(), geometry, index_map, dal/*mode*/ );
    geometry->addPrimitiveSet( dal );
    
    geode->addDrawable( geometry );
}

template< typename T >
void daeReader::processMultiPPrimitive(osg::Geode* geode, T *group, SourceMap &sources, GLenum mode )
{
    osg::Geometry *geometry = new osg::Geometry();
    geometry->setName(group->getMaterial());

    IndexMap index_map;
    resolveArrays( group->getInput_array(), geometry, sources, index_map );

    osg::DrawArrayLengths* dal = new osg::DrawArrayLengths( mode );

    for ( size_t i = 0; i < group->getP_array().getCount(); i++ )
    {
        processP( group->getP_array()[i], geometry, index_map, dal/*mode*/ );
    }
    geometry->addPrimitiveSet( dal );

    geode->addDrawable( geometry );
}

void daeReader::processPolylist(osg::Geode* geode, domPolylist *group, SourceMap &sources )
{
    osg::Geometry *geometry = new osg::Geometry();
    geometry->setName(group->getMaterial());

    IndexMap index_map;
    resolveArrays( group->getInput_array(), geometry, sources, index_map );

    osg::DrawArrayLengths* dal = new osg::DrawArrayLengths( GL_POLYGON );
    
    //domPRef p = (domP*)(daeElement*)domP::_Meta->create(); //I don't condone creating elements like this but I don't care
    domPRef p = (domP*)domP::registerElement(*dae)->create().cast();
    //if it created properly because I never want it as part of the document. Its just a temporary
    //element to trick the importer into loading polylists easier.
    unsigned int maxOffset = 0;
    for ( unsigned int i = 0; i < group->getInput_array().getCount(); i++ )
    {
        if ( group->getInput_array()[i]->getOffset() > maxOffset )
        {
            maxOffset = group->getInput_array()[i]->getOffset();
        }
    }
    maxOffset++;
    unsigned int pOffset = 0;
    for ( unsigned int i = 0; i < group->getCount(); i++ )
    {
        p->getValue().clear();
        for ( unsigned int x = 0; x < group->getVcount()->getValue()[i]; x++ )
        {
            for ( unsigned int y = 0; y < maxOffset; y++ )
            {
                p->getValue().append( group->getP()->getValue()[ pOffset + x*maxOffset + y ] );
            }
        }
        pOffset += group->getVcount()->getValue()[i] * maxOffset;
        processP( p, geometry, index_map, dal/*mode*/ );
    }
    
    geometry->addPrimitiveSet( dal );
    
    geode->addDrawable( geometry );
}

void daeReader::processP( domP *p, osg::Geometry *&/*geom*/, IndexMap &index_map, osg::DrawArrayLengths* dal /*GLenum mode*/ )
{
    int idxcount = index_map.size();
    int count = p->getValue().getCount();
    count = (count/idxcount)*idxcount;
    dal->push_back(count/idxcount);

    int j = 0;
    while ( j < count ) {
        for ( IndexMap::iterator k = index_map.begin(); k != index_map.end(); k++,j++ ) {
            int tmp = p->getValue()[j];
            k->second->push_back(tmp);
        }
    }
}

void daeReader::resolveArrays( domInputLocalOffset_Array &inputs, osg::Geometry *&geom, 
                                    SourceMap &sources, IndexMap &index_map )
{
    domVertices* vertices = NULL;
    daeElement* position_source = NULL;
    daeElement* color_source = NULL;
    daeElement* normal_source = NULL;
    daeElement* texcoord_source = NULL;

    int offset;
    int set;
    daeElement *tmp_el;
    domInputLocalOffset *tmp_input;

    if ( findInputSourceBySemantic( inputs, "VERTEX", tmp_el, &tmp_input ) ) 
    {
        vertices = daeSafeCast< domVertices >( tmp_el );
        if ( vertices == NULL ) 
        {
            osg::notify( osg::WARN )<<"Could not get vertices"<<std::endl;
            return;
        }
        offset = tmp_input->getOffset();
        set = tmp_input->getSet(); //if it wasn't actually set it will initialize to 0 which is 
        //the value we would want anyways.

        domInputLocal *tmp;
        findInputSourceBySemantic( vertices->getInput_array(), "POSITION", position_source, &tmp );
        findInputSourceBySemantic( vertices->getInput_array(), "COLOR", color_source, &tmp );
        findInputSourceBySemantic( vertices->getInput_array(), "NORMAL", normal_source, &tmp );
        findInputSourceBySemantic( vertices->getInput_array(), "TEXCOORD", texcoord_source, &tmp );
        
        if ( index_map[offset] == NULL ) 
        {
            index_map[offset] = new osg::IntArray();
        }
        geom->setVertexIndices( index_map[offset] );
        if ( position_source != NULL )
        {
            geom->setVertexArray( sources[position_source].getVec3Array() );
        }
        if ( color_source != NULL ) 
        {
            geom->setColorArray( sources[color_source].getVec4Array() );
            geom->setColorIndices( index_map[offset] );
            geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
        }
        if ( normal_source != NULL )
        {
            geom->setNormalArray( sources[normal_source].getVec3Array() );
            geom->setNormalIndices( index_map[offset] );
            geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
        }
        if ( texcoord_source != NULL )
        {
            domSourceReader *sc = &sources[texcoord_source];
            switch( sc->getArrayType() ) {
                case domSourceReader::Vec2:
                    geom->setTexCoordArray( set, sc->getVec2Array() );
                    break;
                case domSourceReader::Vec3:
                    geom->setTexCoordArray( set, sc->getVec3Array() );
                    break;
                default:
                    osg::notify( osg::WARN )<<"Unsupported array type: "<< sc->getArrayType() <<std::endl;
                    break;
            }
            geom->setTexCoordIndices( set, index_map[offset] );
        }
    }
    else 
    {
        osg::notify( osg::WARN )<<"Vertex data not found"<<std::endl;
        return;
    }

    if ( findInputSourceBySemantic( inputs, "COLOR", color_source, &tmp_input )) 
    {
        offset = tmp_input->getOffset();
        if ( index_map[offset] == NULL ) {
            index_map[offset] = new osg::IntArray();
        }
        geom->setColorIndices( index_map[offset] );
    }

    if ( color_source != NULL ) 
    {
        geom->setColorArray( sources[color_source].getVec4Array() );
        geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
    }

    if ( findInputSourceBySemantic( inputs, "NORMAL", normal_source, &tmp_input ) ) 
    {
        offset = tmp_input->getOffset();
        if ( index_map[offset] == NULL ) 
        {
            index_map[offset] = new osg::IntArray();
        }
        geom->setNormalIndices( index_map[offset] );
    }

    if ( normal_source ) 
    {
        geom->setNormalArray( sources[normal_source].getVec3Array() );
        geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
    }

    int unit = 0;
    while ( findInputSourceBySemantic( inputs, "TEXCOORD", texcoord_source, &tmp_input, unit ) ) 
    {
        offset = tmp_input->getOffset();
        set = tmp_input->getSet();

        if ( index_map[offset] == NULL ) 
        {
            index_map[offset] = new osg::IntArray();
        }
        //this should really be set. Then when you bind_vertex_input you can adjust accordingly for the
        //effect which currently only puts textures in texunit 0
        geom->setTexCoordIndices( unit/*set*/, index_map[offset] );

        if ( texcoord_source != NULL )
        {
            domSourceReader &sc = sources[texcoord_source];
            switch( sc.getArrayType() ) {
                case domSourceReader::Vec2:
                    geom->setTexCoordArray( unit/*set*/, sc.getVec2Array() );
                    break;
                case domSourceReader::Vec3:
                    geom->setTexCoordArray( unit/*set*/, sc.getVec3Array() );
                    break;
                default:
                    osg::notify( osg::WARN )<<"Unsupported array type: "<< sc.getArrayType() <<std::endl;
                    break;
            }
        }
        unit++;
    }
}
