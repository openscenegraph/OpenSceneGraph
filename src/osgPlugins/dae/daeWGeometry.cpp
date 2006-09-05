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

#include <dom/domCOLLADA.h>
#include <dom/domNode.h>
#include <dom/domLibrary_geometries.h>
#include <dom/domSource.h>
#include <dom/domGeometry.h>
#include <dom/domConstants.h>

#include <sstream>

using namespace osgdae;

//GEODE
void daeWriter::apply( osg::Geode &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    unsigned int count = node.getNumDrawables();
    for ( unsigned int i = 0; i < count; i++ )
    {
        osg::Geometry *g = node.getDrawable( i )->asGeometry();
        if ( g != NULL )
        {
            std::map< osg::Geometry*, domGeometry *>::iterator iter = geometryMap.find( g );
            if ( iter != geometryMap.end() )
            {
                domInstance_geometry *ig = daeSafeCast< domInstance_geometry >( currentNode->createAndPlace( "instance_geometry" ) );
                    
                std::string url = "#" + std::string( iter->second->getId() );
                ig->setUrl( url.c_str() );
                //osg::notify( osg::WARN ) << "Found a duplicate Geometry " << url << std::endl;
                if ( node.getStateSet() != NULL )
                {
                    processMaterial( node.getStateSet(), ig, iter->second->getId() );
                }
            }
            else
            {
                if ( lib_geoms == NULL )
                {
                    lib_geoms = daeSafeCast< domLibrary_geometries >( dom->createAndPlace( COLLADA_ELEMENT_LIBRARY_GEOMETRIES ) );
                }
                std::string name = node.getName();
                if ( name.empty() ) name = "geometry";
                name = uniquify( name );

                domGeometryRef geo = daeSafeCast< domGeometry >( lib_geoms->createAndPlace( COLLADA_ELEMENT_GEOMETRY ) );
                geo->setId( name.c_str() );

                if ( !processGeometry( g, geo, name ) )
                {
                    daeElement::removeFromParent( geo );
                    continue;
                }

                domInstance_geometry *ig = daeSafeCast< domInstance_geometry >( currentNode->createAndPlace( "instance_geometry" ) );
                    
                std::string url = "#" + name;
                ig->setUrl( url.c_str() );

#ifndef EARTH_GEO
                geometryMap.insert( std::make_pair( g, geo ) );
#endif

                if ( node.getStateSet() != NULL )
                {
                    processMaterial( node.getStateSet(), ig, name );
                }
            }
        }
    }

    lastVisited = GEODE;

    traverse( node );
}

/** append elements (verts, normals, colors and texcoord) for file write */
void daeWriter::appendGeometryIndices(osg::Geometry *geom,
                    domP * p,
                    unsigned int vindex,
                    domSource * norm,
                    domSource * color,
                    const ArrayNIndices & verts,
                    const ArrayNIndices & normals,
                    const ArrayNIndices & colors,
                    const std::vector<ArrayNIndices> & texcoords,
                    unsigned int  ncount,
                    unsigned int  ccount)
{
  p->getValue().append( verts.inds!=NULL?verts.inds->index( vindex ):vindex );

  if ( norm != NULL )
    if ( geom->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX )
      p->getValue().append( normals.inds!=NULL?normals.inds->index( vindex ):vindex );
    else
      p->getValue().append( normals.inds!=NULL?normals.inds->index( ncount ):ncount );

  if ( color != NULL )
    if ( geom->getColorBinding() == osg::Geometry::BIND_PER_VERTEX )
      p->getValue().append( colors.inds!=NULL?colors.inds->index( vindex ):vindex );
    else
      p->getValue().append( colors.inds!=NULL?colors.inds->index( ccount ):ccount );

  for ( unsigned int ti = 0; ti < texcoords.size(); ti++ )
  {
    //ArrayNIndices &tc = texcoords[ti];
    p->getValue().append( texcoords[ti].inds!=NULL?texcoords[ti].inds->index(vindex):vindex );
  }
    

}
    
bool daeWriter::processGeometry( osg::Geometry *geom, domGeometry *geo, const std::string &name )
{   
    domMesh *mesh = daeSafeCast< domMesh >( geo->createAndPlace( COLLADA_ELEMENT_MESH ) );
    domSource *pos = NULL;
    domSource *norm = NULL;
    domSource *color = NULL;
    std::vector< domSource * >texcoord;
    domLines *lines = NULL;
    domLinestrips *linestrips = NULL;
    domTriangles *tris = NULL;
    domTristrips *tristrips = NULL;
    domTrifans *trifans = NULL;
    domPolygons *polys = NULL;
        domPolylist *polylist = NULL;
    
    //TODO: Make sure the assumptions about arrays is correct.
    // Probably not so I should make each thing more flexible so arrays can be different sizes.

    /*osg::Vec3Array *verts = (osg::Vec3Array *)geom->getVertexArray();
    osg::IndexArray *vertInds = geom->getVertexIndices();

    osg::Vec3Array *normals = (osg::Vec3Array *)geom->getNormalArray();
    osg::IndexArray *normalInds = geom->getNormalIndices();

    osg::Vec4Array *colors = (osg::Vec4Array *)geom->getColorArray();
    osg::IndexArray *colorInds = geom->getColorIndices();*/

    ArrayNIndices verts( geom->getVertexArray(), geom->getVertexIndices() );
    ArrayNIndices normals( geom->getNormalArray(), geom->getNormalIndices() );
    ArrayNIndices colors( geom->getColorArray(), geom->getColorIndices() );
    std::vector<ArrayNIndices> texcoords;
    for ( unsigned int i = 0; i < geom->getNumTexCoordArrays(); i++ )
    {
        texcoords.push_back( ArrayNIndices( geom->getTexCoordArray( i ), geom->getTexCoordIndices( i ) ) );
    }
        
    //process POSITION
    std::string sName = name + "-positions";
    pos = createSource( mesh, sName, verts.mode );

    switch( verts.mode )
    {
    case ArrayNIndices::VEC2:
        pos->getFloat_array()->setCount( verts.vec2->size() *2 );
        pos->getTechnique_common()->getAccessor()->setCount( verts.vec2->size() );
        for ( unsigned int i = 0; i < verts.vec2->size(); i++ )
        {
            pos->getFloat_array()->getValue().append( (*verts.vec2)[i].x() );
            pos->getFloat_array()->getValue().append( (*verts.vec2)[i].y() );
        }
        break;
    case ArrayNIndices::VEC3:
        pos->getFloat_array()->setCount( verts.vec3->size() *3 );
        pos->getTechnique_common()->getAccessor()->setCount( verts.vec3->size() );
        for ( unsigned int i = 0; i < verts.vec3->size(); i++ )
        {
            pos->getFloat_array()->getValue().append( (*verts.vec3)[i].x() );
            pos->getFloat_array()->getValue().append( (*verts.vec3)[i].y() );
            pos->getFloat_array()->getValue().append( (*verts.vec3)[i].z() );
        }
        break;
    case ArrayNIndices::VEC4:
        pos->getFloat_array()->setCount( verts.vec4->size() *4 );
        pos->getTechnique_common()->getAccessor()->setCount( verts.vec4->size() );
        for ( unsigned int i = 0; i < verts.vec4->size(); i++ )
        {
            pos->getFloat_array()->getValue().append( (*verts.vec4)[i].x() );
            pos->getFloat_array()->getValue().append( (*verts.vec4)[i].y() );
            pos->getFloat_array()->getValue().append( (*verts.vec4)[i].z() );
            pos->getFloat_array()->getValue().append( (*verts.vec4)[i].w() );
        }
        break;
    default:
        osg::notify( osg::WARN ) << "Invalid array type for vertices" << std::endl;
        break;
    }

    //create a vertices element
    domVertices *vertices = daeSafeCast< domVertices >( mesh->createAndPlace( COLLADA_ELEMENT_VERTICES ) );
    std::string vName = name + "-vertices";
    vertices->setId( vName.c_str() );

    //make a POSITION input in it
    domInputLocal *il = daeSafeCast< domInputLocal >( vertices->createAndPlace( "input" ) );
    il->setSemantic( "POSITION" );
    std::string url = "#" + std::string( pos->getId() );
    il->setSource( url.c_str() );

    //process NORMAL
    if ( normals.mode != ArrayNIndices::NONE )
    {
        sName = name + "-normals";
        norm = createSource( mesh, sName, normals.mode );
        
        switch( normals.mode )
        {
        case ArrayNIndices::VEC2:
            norm->getFloat_array()->setCount( normals.vec2->size() *2 );
            norm->getTechnique_common()->getAccessor()->setCount( normals.vec2->size() );
            for ( unsigned int i = 0; i < normals.vec2->size(); i++ )
            {
                norm->getFloat_array()->getValue().append( (*normals.vec2)[i].x() );
                norm->getFloat_array()->getValue().append( (*normals.vec2)[i].y() );
            }
            break;
        case ArrayNIndices::VEC3:
            norm->getFloat_array()->setCount( normals.vec3->size() *3 );
            norm->getTechnique_common()->getAccessor()->setCount( normals.vec3->size() );
            for ( unsigned int i = 0; i < normals.vec3->size(); i++ )
            {
                norm->getFloat_array()->getValue().append( (*normals.vec3)[i].x() );
                norm->getFloat_array()->getValue().append( (*normals.vec3)[i].y() );
                norm->getFloat_array()->getValue().append( (*normals.vec3)[i].z() );
            }
            break;
        case ArrayNIndices::VEC4:
            norm->getFloat_array()->setCount( normals.vec4->size() *4 );
            norm->getTechnique_common()->getAccessor()->setCount( normals.vec4->size() );
            for ( unsigned int i = 0; i < normals.vec4->size(); i++ )
            {
                norm->getFloat_array()->getValue().append( (*normals.vec4)[i].x() );
                norm->getFloat_array()->getValue().append( (*normals.vec4)[i].y() );
                norm->getFloat_array()->getValue().append( (*normals.vec4)[i].z() );
                norm->getFloat_array()->getValue().append( (*normals.vec4)[i].w() );
            }
            break;

        //no normals
        case ArrayNIndices::NONE:
          osg::notify( osg::WARN ) << "No array type for normals"<<std::endl;
        default:
          osg::notify( osg::WARN ) << "Invalid array type for normals"<<std::endl;
          break;
        }

        //if NORMAL shares same indices as POSITION put it in the vertices
        /*if ( normalInds == vertInds && vertInds != NULL ) {
            il = daeSafeCast< domInputLocal >( vertices->createAndPlace( "input" ) );
            il->setSemantic( "NORMAL" );
            url = "#" + std::string(md->norm->getId());
            il->setSource( url.c_str() );
        }*/
    }

    //process COLOR
    if ( colors.mode != ArrayNIndices::NONE )
    {
        sName = name + "-colors";
        color = createSource( mesh, sName, colors.mode, true );
        
        switch( colors.mode )
        {
        case ArrayNIndices::VEC2:
            color->getFloat_array()->setCount( colors.vec2->size() *2 );
            color->getTechnique_common()->getAccessor()->setCount( colors.vec2->size() );
            for ( unsigned int i = 0; i < colors.vec2->size(); i++ )
            {
                color->getFloat_array()->getValue().append( (*colors.vec2)[i].x() );
                color->getFloat_array()->getValue().append( (*colors.vec2)[i].y() );
            }
            break;
        case ArrayNIndices::VEC3:
            color->getFloat_array()->setCount( colors.vec3->size() *3 );
            color->getTechnique_common()->getAccessor()->setCount( colors.vec3->size() );
            for ( unsigned int i = 0; i < colors.vec3->size(); i++ )
            {
                color->getFloat_array()->getValue().append( (*colors.vec3)[i].x() );
                color->getFloat_array()->getValue().append( (*colors.vec3)[i].y() );
                color->getFloat_array()->getValue().append( (*colors.vec3)[i].z() );
            }
            break;
        case ArrayNIndices::VEC4:
            color->getFloat_array()->setCount( colors.vec4->size() *4 );
            color->getTechnique_common()->getAccessor()->setCount( colors.vec4->size() );
            for ( unsigned int i = 0; i < colors.vec4->size(); i++ )
            {
                color->getFloat_array()->getValue().append( (*colors.vec4)[i].r() );
                color->getFloat_array()->getValue().append( (*colors.vec4)[i].g() );
                color->getFloat_array()->getValue().append( (*colors.vec4)[i].b() );
                color->getFloat_array()->getValue().append( (*colors.vec4)[i].a() );
            }
            break;
        default:
            osg::notify( osg::WARN ) << "Invalid array type for colors" << std::endl;
            break;
        }
        //if COLOR shares same indices as POSITION put it in the vertices
        /*if ( colorInds == vertInds && vertInds != NULL ) {
            il = daeSafeCast< domInputLocal >( vertices->createAndPlace( "input" ) );
            il->setSemantic( "COLOR" );
            url = "#" + std::string(md->color->getId());
            il->setSource( url.c_str() );
        }*/
    }

    //process TEXCOORD
    //TODO: Do the same as normal and colors for texcoods. But in a loop since you can have many
    for ( unsigned int ti = 0; ti < texcoords.size(); ti++ )
    {
        std::ostringstream intstr;
        intstr << std::dec << ti;
        sName = name + "-texcoord_" + intstr.str();

        domSource *t = createSource( mesh, sName, texcoords[ti].mode, false, true );
        switch( texcoords[ti].mode )
        {
        case ArrayNIndices::VEC2:
            t->getFloat_array()->setCount( texcoords[ti].vec2->size() *2 );
            t->getTechnique_common()->getAccessor()->setCount( texcoords[ti].vec2->size() );
            for ( unsigned int i = 0; i < texcoords[ti].vec2->size(); i++ )
            {
                t->getFloat_array()->getValue().append( (*texcoords[ti].vec2)[i].x() );
                t->getFloat_array()->getValue().append( (*texcoords[ti].vec2)[i].y() );
            }
            break;
        case ArrayNIndices::VEC3:
            t->getFloat_array()->setCount( texcoords[ti].vec3->size() *3 );
            t->getTechnique_common()->getAccessor()->setCount( texcoords[ti].vec3->size() );
            for ( unsigned int i = 0; i < texcoords[ti].vec3->size(); i++ )
            {
                t->getFloat_array()->getValue().append( (*texcoords[ti].vec3)[i].x() );
                t->getFloat_array()->getValue().append( (*texcoords[ti].vec3)[i].y() );
                t->getFloat_array()->getValue().append( (*texcoords[ti].vec3)[i].z() );
            }
            break;
        case ArrayNIndices::VEC4:
            t->getFloat_array()->setCount( texcoords[ti].vec4->size() *4 );
            t->getTechnique_common()->getAccessor()->setCount( texcoords[ti].vec4->size() );
            for ( unsigned int i = 0; i < texcoords[ti].vec4->size(); i++ )
            {
                t->getFloat_array()->getValue().append( (*texcoords[i].vec4)[ti].x() );
                t->getFloat_array()->getValue().append( (*texcoords[i].vec4)[ti].y() );
                t->getFloat_array()->getValue().append( (*texcoords[i].vec4)[ti].z() );
                t->getFloat_array()->getValue().append( (*texcoords[i].vec4)[ti].w() );
            }
            break;
        default:
          //##ti and not i
            osg::notify( osg::WARN ) << "Invalid array type for texcoord" << ti << std::endl;
            break;
        }
        texcoord.push_back( t );
    }

    //process each primitive group
    unsigned int ncount = 0; //Normal index counter
    unsigned int ccount = 0; //Color index counter
    if ( geom->getNumPrimitiveSets() == 0 )
    {
        osg::notify( osg::WARN ) << "NO PRIMITIVE SET!!" << std::endl;
        return false;
    }
    bool valid = false;
    //for each primitive group
    for ( unsigned int i = 0; i < geom->getNumPrimitiveSets(); i++ )
    {
        osg::PrimitiveSet *ps = geom->getPrimitiveSet( i );
        GLenum mode = ps->getMode();
        unsigned int primLength;
        //unsigned int offset = 0;
        //domInputLocalOffset *ilo = NULL;

        //process primitive group
        switch( mode )
        {
            case GL_POINTS:    
            {
                osg::notify( osg::WARN ) << "Geometry contains points rendering. COLLADA does not" << std::endl; 
                continue;
            }
            case GL_LINES:     
            {
                if ( lines == NULL )
                {
                    lines = createPrimGroup<domLines>( COLLADA_ELEMENT_LINES, mesh, norm, color, texcoord );
                    lines->createAndPlace( COLLADA_ELEMENT_P );
                    std::string mat = name + "_material";
                    lines->setMaterial( mat.c_str() );
                }
                primLength = 2; 
                valid = true;
                break;
            }
            case GL_TRIANGLES: 
            {
                if ( tris == NULL )
                {
                    tris = createPrimGroup<domTriangles>( COLLADA_ELEMENT_TRIANGLES, mesh, norm, color, texcoord );
                    tris->createAndPlace( COLLADA_ELEMENT_P );
                    std::string mat = name + "_material";
                    tris->setMaterial( mat.c_str() );
                }
                primLength = 3; 
                valid = true;
                break;
            }
            case GL_QUADS:     
            {
                if ( polys == NULL )
                {
                    if (usePolygons)
                    {
                          polys = createPrimGroup<domPolygons>( COLLADA_ELEMENT_POLYGONS, mesh, norm, color, texcoord );
                          polys->createAndPlace( COLLADA_ELEMENT_P );
                          std::string mat = name + "_material";
                          polys->setMaterial( mat.c_str() );
                    }
                    else
                    {
                          polylist = createPrimGroup<domPolylist>( COLLADA_ELEMENT_POLYLIST, mesh, norm, color, texcoord );
                            
                          polylist->createAndPlace( COLLADA_ELEMENT_VCOUNT );
                          polylist->createAndPlace( COLLADA_ELEMENT_P );
                          std::string mat = name + "_material";
                          polylist->setMaterial( mat.c_str() );
                    }
                }
                primLength = 4; 
                valid = true;
                break;
            }
            case GL_LINE_STRIP:
            {
                if ( linestrips == NULL )
                {
                    linestrips = createPrimGroup<domLinestrips>( COLLADA_ELEMENT_LINESTRIPS, mesh, norm, color, texcoord );
                    std::string mat = name + "_material";
                    linestrips->setMaterial( mat.c_str() );
                }
                primLength = 0;
                valid = true;
                break;
            }
            case GL_TRIANGLE_STRIP:
            {
                if ( tristrips == NULL )
                {
                    tristrips = createPrimGroup<domTristrips>( COLLADA_ELEMENT_TRISTRIPS, mesh, norm, color, texcoord );
                    std::string mat = name + "_material";
                    tristrips->setMaterial( mat.c_str() );
                }
                primLength = 0;
                valid = true;
                break;
            }
            case GL_TRIANGLE_FAN:
            {
                if ( trifans == NULL )
                {
                    trifans = createPrimGroup<domTrifans>( COLLADA_ELEMENT_TRIFANS, mesh, norm, color, texcoord );
                    std::string mat = name + "_material";
                    trifans->setMaterial( mat.c_str() );
                }
                primLength = 0;
                valid = true;
                break;
            }
            default:           
            {
                if ( polys == NULL )
                {
                    if (usePolygons)
                    {
                        polys = createPrimGroup<domPolygons>( COLLADA_ELEMENT_POLYGONS, mesh, norm, color, texcoord );
                        polys->createAndPlace( COLLADA_ELEMENT_P );
                        std::string mat = name + "_material";
                        polys->setMaterial( mat.c_str() );
                    }
                    else
                    {
                        polylist = createPrimGroup<domPolylist>( COLLADA_ELEMENT_POLYLIST, mesh, norm, color, texcoord );

                        polylist->createAndPlace( COLLADA_ELEMENT_VCOUNT );
                        polylist->createAndPlace( COLLADA_ELEMENT_P );
                        std::string mat = name + "_material";
                        polylist->setMaterial( mat.c_str() );
                    }
                }
                primLength = 0; 
                valid = true;
                break; // compute later when =0.
            }
        }

                //process upon primitive set type
                // 1- set data source,accumulate count of primitives and write it to file
                // 2- read data source for primitive set and write it to file
        switch( ps->getType() )
        {
                //draw arrays (array of contiguous vertices)
    
                       //(primitive type+begin+end),(primitive type+begin+end)...
            case osg::PrimitiveSet::DrawArraysPrimitiveType:
            {
                //osg::notify( osg::WARN ) << "DrawArrays" << std::endl;

                if ( primLength == 0 ) 
                {
                    primLength = ps->getNumIndices();
                }
                osg::DrawArrays* drawArray = static_cast< osg::DrawArrays* >( ps );
                unsigned int vcount = 0;
                unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();

                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                                                p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawArray->getCount()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                                                p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawArray->getCount()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                                                p.push_back(daeSafeCast<domP>( linestrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                                                p.push_back(daeSafeCast<domP>( tristrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                                                p.push_back(daeSafeCast<domP>( trifans->createAndPlace( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:           
                    {
                        //TODO : test this case
                        unsigned int nbPolygons=drawArray->getCount()/primLength;
                        if (usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx )
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            for( unsigned int idx = 0; idx < nbPolygons; ++idx )
                                    polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons);
                        }
                        break;
                    }
                }

                unsigned int indexBegin = drawArray->getFirst();
                unsigned int nbVerticesPerPoly=indexEnd/p.size();
                unsigned int indexPolyEnd = indexBegin+nbVerticesPerPoly;
                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {
                    for (unsigned int vindex=indexBegin; vindex< indexPolyEnd;vindex++)
                    {

                       appendGeometryIndices(geom,p[iPoly],vindex,
                                     norm,color,
                                     verts,normals,colors,texcoords,
                                     ncount,ccount);

                       if ( vcount % primLength == 0 )
                       {
                         if ( geom->getNormalBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                         {
                           ncount++;
                         }
                         if ( geom->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                         {
                           ccount++;
                         }
                       }
                       vcount++;
                    }
                    indexBegin+=nbVerticesPerPoly;
                    indexPolyEnd+=nbVerticesPerPoly;
                }
                break;
            }
            //(primitive type) + (end1),(end2),(end3)...
            case osg::PrimitiveSet::DrawArrayLengthsPrimitiveType:
            {
                //osg::notify( osg::WARN ) << "DrawArrayLengths" << std::endl;

                osg::DrawArrayLengths* drawArrayLengths = static_cast<osg::DrawArrayLengths*>( ps );

                unsigned int vindex = drawArrayLengths->getFirst();
                for( osg::DrawArrayLengths::iterator primItr = drawArrayLengths->begin();
                    primItr != drawArrayLengths->end();
                    ++primItr )
                {
                    unsigned int localPrimLength;
                    if ( primLength == 0 ) localPrimLength = *primItr;
                    else localPrimLength = primLength;

                    std::vector<domP *> p;
                    switch ( mode )
                    {
                        case GL_LINES:
                        {
                            p.push_back(lines->getP());
                            lines->setCount( lines->getCount() + (*primItr)/localPrimLength );
                            break;
                        }
                        case GL_TRIANGLES:
                        {
                            p.push_back(tris->getP());
                            tris->setCount( tris->getCount() + (*primItr)/localPrimLength );
                            break;
                        }
                        case GL_LINE_STRIP:
                        {
                            p.push_back(daeSafeCast<domP>( linestrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                            linestrips->setCount( linestrips->getCount() + 1 );
                            break;
                        }
                        case GL_TRIANGLE_STRIP:
                        {
                            p.push_back(daeSafeCast<domP>( tristrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                            tristrips->setCount( tristrips->getCount() + 1 );
                            break;
                        }
                        case GL_TRIANGLE_FAN:
                        {
                            p.push_back(daeSafeCast<domP>( trifans->createAndPlace( COLLADA_ELEMENT_P ) ));
                            trifans->setCount( trifans->getCount() + 1 );
                            break;
                        }
                        default:           
                        {

                            if (usePolygons)
                            {
                                //for( unsigned int idx = 0; idx < nbPolygons; ++idx )
                                p.push_back(polys->getP_array()[0]);
                                polys->setCount( polys->getCount() + 1 );
                            }
                            else
                            {
                                polylist->getVcount()->getValue().append( localPrimLength );
                                p.push_back(polylist->getP());
                                polylist->setCount( polylist->getCount() + 1);
                            }
                            break;
                        }
                    }

                    unsigned int indexBegin = 0;
                    unsigned int nbVerticesPerPoly=*primItr/p.size();
                    unsigned int indexEnd=indexBegin+nbVerticesPerPoly;
                    for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                    {
                       // printf("indexBegin %d,indexPolyEnd %d \n",indexBegin,indexEnd);
                       for( unsigned int primCount = indexBegin; primCount < indexEnd; ++primCount )
                        {
                          appendGeometryIndices(geom,p[iPoly],vindex,
                                           norm,color,
                                           verts,normals,colors,texcoords,
                                           ncount,ccount);
                          
                          if ( primCount % localPrimLength == 0 )
                                                  {
                             if ( geom->getNormalBinding() == osg::Geometry::BIND_PER_PRIMITIVE ) 
                             {
                                    ncount++;
                             }
                             if ( geom->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE ) 
                             {
                                    ccount++;
                             }
                          }
                                             vindex++;
                       }
                       indexBegin+=nbVerticesPerPoly;
                       indexEnd+=nbVerticesPerPoly;
                       }
                }
                break;
            }

            //draw elements (array of shared vertices + array of indices)
           case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
                     {
                //osg::notify( osg::WARN ) << "DrawElementsUByte" << std::endl;

                if ( primLength == 0 ) primLength = ps->getNumIndices();

                osg::DrawElementsUByte* drawElements = static_cast<osg::DrawElementsUByte*>( ps );
                
                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                        p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                        p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( linestrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( tristrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                        p.push_back(daeSafeCast<domP>( trifans->createAndPlace( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:           
                    {
                        unsigned int nbPolygons=drawElements->size()/primLength;
                        if (usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx ) /*idx*/
                            //huh ? why only one ?
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons );
                        }
                        break;
                    }
                }


                unsigned int primCount = 0;
                osg::DrawElementsUByte::iterator primItrBegin = drawElements->begin();
                unsigned int nbVerticesPerPoly= drawElements->size()/p.size();
                osg::DrawElementsUByte::iterator primItrEnd=primItrBegin+nbVerticesPerPoly;
                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {
                    for( osg::DrawElementsUByte::iterator primItr = primItrBegin;primItr != primItrEnd;
                    ++primCount, ++primItr )
                    {

                        unsigned int vindex = *primItr;
    
                        appendGeometryIndices(geom,p[iPoly],vindex,
                                              norm,color,
                                              verts,normals,colors,texcoords,
                                              ncount,ccount);
                        
                        if ( primCount % primLength == 0 )
                        {
                            if ( geom->getNormalBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                            {
                              ncount++;
                            }
                            if ( geom->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                            {
                              ccount++;
                            }
                        }
                    }

                    primItrBegin+=nbVerticesPerPoly;
                    primItrEnd+=nbVerticesPerPoly;
                }
                break;
            }
            case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
            {
                //osg::notify( osg::WARN ) << "DrawElementsUShort" << std::endl;
                if ( primLength == 0 ) primLength = ps->getNumIndices();

                osg::DrawElementsUShort* drawElements = static_cast<osg::DrawElementsUShort*>( ps );
                
                                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                        p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                        p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( linestrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( tristrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                        p.push_back(daeSafeCast<domP>( trifans->createAndPlace( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:           
                    {
                        unsigned int nbPolygons=drawElements->size()/primLength;
                        if (usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx ) /*idx*/
                            //huh ? why only one ?
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons );
                        }
                        break;
                    }
                }
                
                unsigned int primCount = 0;
                osg::DrawElementsUShort::iterator primItrBegin = drawElements->begin();
                unsigned int nbVerticesPerPoly= drawElements->size()/p.size();
                osg::DrawElementsUShort::iterator primItrEnd=primItrBegin+nbVerticesPerPoly;

                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {
                  for( osg::DrawElementsUShort::iterator primItr = primItrBegin;primItr != primItrEnd;
                       ++primCount, ++primItr )
                  {

                    unsigned int vindex = *primItr;
    
                    appendGeometryIndices(geom,p[iPoly],vindex,
                                       norm,color,
                                       verts,normals,colors,texcoords,
                                       ncount,ccount);
                        
                    if ( primCount % primLength == 0 )
                    {
                      if ( geom->getNormalBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                      {
                        ncount++;
                      }
                      if ( geom->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                      {
                        ccount++;
                      }
                    }

                  }
                  primItrBegin+=nbVerticesPerPoly;
                  primItrEnd+=nbVerticesPerPoly;      
                }
      
                break;
            }
            case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
            {
                //osg::notify( osg::WARN ) << "DrawElementsUInt" << std::endl;

                if ( primLength == 0 ) primLength = ps->getNumIndices();

                osg::DrawElementsUInt* drawElements = static_cast<osg::DrawElementsUInt*>( ps );
                
                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                        p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                        p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( linestrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( tristrips->createAndPlace( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                        p.push_back(daeSafeCast<domP>( trifans->createAndPlace( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:           
                    {
                        unsigned int nbPolygons=drawElements->size()/primLength;
                        if (usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx ) /*idx*/
                            //huh ? why only one ?
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons );
                        }
                        break;
                    }
                }

                unsigned int primCount = 0;
                osg::DrawElementsUInt::iterator primItrBegin = drawElements->begin();
                unsigned int nbVerticesPerPoly= drawElements->size()/p.size();
                osg::DrawElementsUInt::iterator primItrEnd=primItrBegin+nbVerticesPerPoly;

                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {

                  for( osg::DrawElementsUInt::iterator primItr = primItrBegin;primItr != primItrEnd;
                       ++primCount, ++primItr )
                  {

                    unsigned int vindex = *primItr;
    
                    appendGeometryIndices(geom,p[iPoly],vindex,
                                       norm,color,
                                       verts,normals,colors,texcoords,
                                       ncount,ccount);
                        
                    if ( primCount % primLength == 0 )
                    {
                      if ( geom->getNormalBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                      {
                        ncount++;
                      }
                      if ( geom->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE )
                      {
                        ccount++;
                      }
                    }
                  }
                  primItrBegin+=nbVerticesPerPoly;
                  primItrEnd+=nbVerticesPerPoly;      
                }
                break;
            }
            default:
                osg::notify( osg::WARN ) << "Unsupported primitiveSet" << std::endl;
                break;
        }

        if ( geom->getNormalBinding() == osg::Geometry::BIND_PER_PRIMITIVE_SET ) 
        {
            ncount++;
        }
        if ( geom->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE_SET ) 
        {
            ccount++;
        }
    }
    return valid;
}

domSource *daeWriter::createSource( daeElement *parent, const std::string &baseName, int size, bool color, bool uv )
{
    domSource *src = daeSafeCast< domSource >( parent->createAndPlace( COLLADA_ELEMENT_SOURCE ) );
    if ( src == NULL )
    {
        return NULL;
    }
    src->setId( baseName.c_str() );

    domFloat_array *fa = daeSafeCast< domFloat_array >( src->createAndPlace( COLLADA_ELEMENT_FLOAT_ARRAY ) );
    std::string fName = baseName + "-array";
    fa->setId( fName.c_str() );

    domSource::domTechnique_common *teq = daeSafeCast< domSource::domTechnique_common >( src->createAndPlace( "technique_common" ) );
    domAccessor *acc = daeSafeCast< domAccessor >( teq->createAndPlace( COLLADA_ELEMENT_ACCESSOR ) );
    std::string url = "#" + fName;
    acc->setSource( url.c_str() );
    domParam *param;
    if ( color )
    {
        acc->setStride( size );
        param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
        param->setName( "R" );
        param->setType( "float" );
        
        param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
        param->setName( "G" );
        param->setType( "float" );

        param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
        param->setName( "B" );
        param->setType( "float" );

        if ( size == 4 ) {
            param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
            param->setName( "A" );
            param->setType( "float" );
        }

    }
    else if ( uv )
    {
        acc->setStride( size );
        param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
        param->setName( "S" );
        param->setType( "float" );

        param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
        param->setName( "T" );
        param->setType( "float" );

        if ( size >=3 )
        {
            param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
            param->setName( "P" );
            param->setType( "float" );
        }
    }
    else
    {
        acc->setStride( size );
        param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
        param->setName( "X" );
        param->setType( "float" );

        param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
        param->setName( "Y" );
        param->setType( "float" );

        if ( size >=3 )
        {
            param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
            param->setName( "Z" );
            param->setType( "float" );

            if ( size == 4 )
            {
                param = daeSafeCast< domParam >( acc->createAndPlace( COLLADA_ELEMENT_PARAM ) );
                param->setName( "W" );
                param->setType( "float" );
            }
        }
    }

    return src;
}

template < typename Ty >
Ty *daeWriter::createPrimGroup( daeString type, domMesh *mesh, domSource *norm, domSource *color, const std::vector< domSource* > &texcoord )
{
    unsigned int offset = 0;
    Ty *retVal = daeSafeCast< Ty >( mesh->createAndPlace( type ) );
    domInputLocalOffset *ilo = daeSafeCast< domInputLocalOffset >( retVal->createAndPlace( "input" ) );
    ilo->setOffset( offset++ );
    ilo->setSemantic( "VERTEX" );
    std::string url = "#" + std::string(mesh->getVertices()->getId());
    ilo->setSource( url.c_str() );
    if ( norm != NULL )
    {
        ilo = daeSafeCast< domInputLocalOffset >( retVal->createAndPlace( "input" ) );
        ilo->setOffset( offset++ );
        ilo->setSemantic( "NORMAL" );
        url = "#" + std::string( norm->getId() );
        ilo->setSource( url.c_str() );
    }
    if ( color != NULL )
    {
        ilo = daeSafeCast< domInputLocalOffset >( retVal->createAndPlace( "input" ) );
        ilo->setOffset( offset++ );
        ilo->setSemantic( "COLOR" );
        url = "#" + std::string( color->getId() );
        ilo->setSource( url.c_str() );
    }
    for ( unsigned int i = 0; i < texcoord.size(); i++ )
    {
        ilo = daeSafeCast< domInputLocalOffset >( retVal->createAndPlace( "input" ) );
        ilo->setOffset( offset++ );
        ilo->setSemantic( "TEXCOORD" );
        ilo->setSet( i );
        url = "#" + std::string( texcoord[i]->getId() );
        ilo->setSource( url.c_str() );
    }

    return retVal;
}
