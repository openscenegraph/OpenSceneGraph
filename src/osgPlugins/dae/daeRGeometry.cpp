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

#include <osg/Geometry>

using namespace osgdae;

osg::Node* daeReader::processInstance_geometry( domInstance_geometry *ig )
{
    //TODO: cache geometries so they don't get processed mulitple times.
    //TODO: after cached need to check geometries and materials. both have to be the same for it
    //      to be the same instance.

    daeElement *el = getElementFromURI( ig->getUrl() );
    domGeometry *geom = daeSafeCast< domGeometry >( el );
    if ( geom == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to locate geometry " << ig->getUrl().getURI() << std::endl;
        return NULL;
    }
    //check cache if geometry already exists
    osg::Node *geo;

    std::map< domGeometry*, osg::Node* >::iterator iter = geometryMap.find( geom );
    if ( iter != geometryMap.end() )
    {
        geo = iter->second;
    }
    else
    {
        geo = processGeometry( geom );
        geometryMap.insert( std::make_pair( geom, geo ) );
    }
    if ( geo == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to load geometry " << ig->getUrl().getURI() << std::endl;
        return NULL;
    }
    //process material bindings
    if ( ig->getBind_material() != NULL )
    {
        processBindMaterial( ig->getBind_material(), geo );
    }

    return geo;
}

osg::Node* daeReader::processInstance_controller( domInstance_controller *ictrl )
{
    //TODO: cache geometries so they don't get processed mulitple times.
    //TODO: after cached need to check geometries and materials. both have to be the same for it
    //      to be the same instance.
    //TODO: support skinning

    osg::notify( osg::WARN ) << "Processing <instance_controller>. There is not skinning support but will display the base mesh." << std::endl;
    daeElement *el = getElementFromURI( ictrl->getUrl() );
    domController *ctrl = daeSafeCast< domController >( el );
    if ( ctrl == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to locate controller " << ictrl->getUrl().getURI() << std::endl;
        return NULL;
    }
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
    osg::Node *geo;

    std::map< domGeometry*, osg::Node* >::iterator iter = geometryMap.find( geom );
    if ( iter != geometryMap.end() )
    {
        geo = iter->second;
    }
    else
    {
        geo = processGeometry( geom );
        geometryMap.insert( std::make_pair( geom, geo ) );
    }
    if ( geo == NULL )
    {
        osg::notify( osg::WARN ) << "Failed to load geometry " << src->getURI() << std::endl;
        return NULL;
    }
    //process material bindings
    if ( ictrl->getBind_material() != NULL )
    {
        processBindMaterial( ictrl->getBind_material(), geo );
    }

    return geo;
}

osg::Node *daeReader::processGeometry( domGeometry *geo )
{
    domMesh *mesh = geo->getMesh();
    if ( mesh == NULL )
    {
        osg::notify( osg::WARN ) << "Unsupported Geometry type loading " << geo->getId() << std::endl;
        return NULL;
    }
    osg::Node *node = new osg::Group();

    if ( geo->getId() != NULL )
    {
        node->setName( geo->getId() );
    }

    SourceMap sources;
    
    size_t count = mesh->getContents().getCount();
    for ( size_t i = 0; i < count; i++ )
    {
        if ( daeSafeCast< domVertices >( mesh->getContents()[i] ) != NULL ) continue;

        domSource *s = daeSafeCast< domSource >( mesh->getContents()[i] );
        if ( s != NULL )
        {
            sources.insert( std::make_pair( (daeElement*)mesh->getContents()[i], 
                domSourceReader( s ) ) ); 
            continue;
        }

        osg::Node *n = NULL;

        domTriangles *t = daeSafeCast< domTriangles >( mesh->getContents()[i] );
        if ( t != NULL )
        {
            n = processSinglePPrimitive( t, sources, GL_TRIANGLES );
            if ( n != NULL )
            {
                node->asGroup()->addChild( n );
            }
            continue;
        }

        domTristrips *ts = daeSafeCast< domTristrips >( mesh->getContents()[i] );
        if ( ts != NULL )
        {
            n = processMultiPPrimitive( ts, sources, GL_TRIANGLE_STRIP );
            if ( n != NULL )
            {
                node->asGroup()->addChild( n );
            }
            continue;
        }

        domTrifans *tf = daeSafeCast< domTrifans >( mesh->getContents()[i] );
        if ( tf != NULL )
        {
            n = processMultiPPrimitive( tf, sources, GL_TRIANGLE_FAN );
            if ( n != NULL )
            {
                node->asGroup()->addChild( n );
            }
            continue;
        }

        domLines *l = daeSafeCast< domLines >( mesh->getContents()[i] );
        if ( l != NULL )
        {
            n = processSinglePPrimitive( l, sources, GL_LINES );
            if ( n != NULL )
            {
                node->asGroup()->addChild( n );
            }
            continue;
        }

        domLinestrips *ls = daeSafeCast< domLinestrips >( mesh->getContents()[i] );
        if ( ls != NULL )
        {
            n = processMultiPPrimitive( ls, sources, GL_LINE_STRIP );
            if ( n != NULL )
            {
                node->asGroup()->addChild( n );
            }
            continue;
        }

        domPolygons *p = daeSafeCast< domPolygons >( mesh->getContents()[i] );
        if ( p != NULL )
        {
            n = processMultiPPrimitive( p, sources, GL_POLYGON );
            if ( n != NULL )
            {
                node->asGroup()->addChild( n );
            }
            continue;
        }

        domPolylist *pl = daeSafeCast< domPolylist >( mesh->getContents()[i] );
        if ( pl != NULL )
        {
            n = processPolylist( pl, sources );
            if ( n != NULL )
            {
                node->asGroup()->addChild( n );
            }
            continue;
        }

        osg::notify( osg::WARN ) << "Unsupported primitive type " << mesh->getContents()[i]->getTypeName() << " in geometry " << geo->getId() << std::endl;
    }

    return node;
}

template< typename T >
osg::Node* daeReader::processSinglePPrimitive( T *group, SourceMap &sources, GLenum mode )
{
    osg::Geode* geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();

    //Setting the name of the geode to the material symbol for easy binding later
    if ( group->getMaterial() != NULL )
    {
        geode->setName( group->getMaterial() );
    }

    IndexMap index_map;
    resolveArrays( group->getInput_array(), geometry, sources, index_map );

    osg::DrawArrayLengths* dal = new osg::DrawArrayLengths( mode );
    processP( group->getP(), geometry, index_map, dal/*mode*/ );
    geometry->addPrimitiveSet( dal );
    
    geode->addDrawable( geometry );
    return geode;
}

template< typename T >
    osg::Node* daeReader::processMultiPPrimitive( T *group, SourceMap &sources, GLenum mode )
{
    osg::Geode* geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();

    //Setting the name of the geode to the material symbol for easy binding later
    if ( group->getMaterial() != NULL )
    {
        geode->setName( group->getMaterial() );
    }

    IndexMap index_map;
    resolveArrays( group->getInput_array(), geometry, sources, index_map );

    osg::DrawArrayLengths* dal = new osg::DrawArrayLengths( mode );

    for ( size_t i = 0; i < group->getP_array().getCount(); i++ )
    {
        processP( group->getP_array()[i], geometry, index_map, dal/*mode*/ );
    }
    geometry->addPrimitiveSet( dal );

    geode->addDrawable( geometry );
    return geode;
}

osg::Node* daeReader::processPolylist( domPolylist *group, SourceMap &sources )
{
    osg::Geode* geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();

    //Setting the name of the geode to the material symbol for easy binding later
    if ( group->getMaterial() != NULL )
    {
        geode->setName( group->getMaterial() );
    }

    IndexMap index_map;
    resolveArrays( group->getInput_array(), geometry, sources, index_map );

    osg::DrawArrayLengths* dal = new osg::DrawArrayLengths( GL_POLYGON );
    
    domPRef p = (domP*)(daeElement*)domP::_Meta->create(); //I don't condone creating elements like this but I don't care
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
    return geode;
}

void daeReader::processP( domP *p, osg::Geometry *&/*geom*/, IndexMap &index_map, osg::DrawArrayLengths* dal /*GLenum mode*/ )
{
    //osg::DrawArrayLengths* dal = new osg::DrawArrayLengths( mode );
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
    //geom->addPrimitiveSet( dal );
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

    if ( findInputSourceBySemantic( inputs, "VERTEX", tmp_el, &tmp_input ) ) {
        vertices = daeSafeCast< domVertices >( tmp_el );
        if ( vertices == NULL ) {
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
        
        if ( index_map[offset] == NULL ) {
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

    
    if ( findInputSourceBySemantic( inputs, "COLOR", color_source, &tmp_input ) ) {
        
        offset = tmp_input->getOffset();
        if ( index_map[offset] == NULL ) {
            index_map[offset] = new osg::IntArray();
        }
        geom->setColorIndices( index_map[offset] );
    }

    if ( color_source != NULL ) {
        geom->setColorArray( sources[color_source].getVec4Array() );
        geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );
    }

    if ( findInputSourceBySemantic( inputs, "NORMAL", normal_source, &tmp_input ) ) {
        
        offset = tmp_input->getOffset();
        if ( index_map[offset] == NULL ) {
            index_map[offset] = new osg::IntArray();
        }
        geom->setNormalIndices( index_map[offset] );
    }

    if ( normal_source ) {
        geom->setNormalArray( sources[normal_source].getVec3Array() );
        geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
    }

    int unit = 0;
    while ( findInputSourceBySemantic( inputs, "TEXCOORD", texcoord_source, &tmp_input, unit ) ) {
        
        offset = tmp_input->getOffset();
        set = tmp_input->getSet();

        if ( index_map[offset] == NULL ) {
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
