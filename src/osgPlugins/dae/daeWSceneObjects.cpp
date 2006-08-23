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
#include <dom/domConstants.h>
#include <dom/domLibrary_cameras.h>
#include <dom/domLibrary_lights.h>

//#include <dom/domVisual_scene.h>
//#include <dom/domLibrary_visual_scenes.h>

using namespace osgdae;


//GROUP
void daeWriter::apply( osg::Group &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    while ( lastDepth >= _nodePath.size() )
    {
        //We are not a child of previous node
        currentNode = daeSafeCast< domNode >( currentNode->getParentElement() );
        lastDepth--;
    }
    currentNode = daeSafeCast< domNode >(currentNode->createAndPlace( COLLADA_ELEMENT_NODE ) );
        currentNode->setId(getNodeName(node,"group").c_str());
    
    lastDepth = _nodePath.size();

    lastVisited = GROUP;
    
    traverse( node );
}


//SWITCH
void daeWriter::apply( osg::Switch &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    while ( lastDepth >= _nodePath.size() )
    {
        //We are not a child of previous node
        currentNode = daeSafeCast< domNode >( currentNode->getParentElement() );
        lastDepth--;
    }
    currentNode = daeSafeCast< domNode >(currentNode->createAndPlace( COLLADA_ELEMENT_NODE ) );
    currentNode->setId(getNodeName(node,"switch").c_str());
    
    lastDepth = _nodePath.size();

    lastVisited = SWITCH;

    unsigned int cnt = node.getNumChildren();
    for ( unsigned int i = 0; i < cnt; i++ ) 
    {
        if ( node.getValue( i ) )
        {
            node.getChild( i )->accept( *this );
        }
    }
}

//LOD
void daeWriter::apply( osg::LOD &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    while ( lastDepth >= _nodePath.size() )
    {
        //We are not a child of previous node
        currentNode = daeSafeCast< domNode >( currentNode->getParentElement() );
        lastDepth--;
    }
    currentNode = daeSafeCast< domNode >(currentNode->createAndPlace( COLLADA_ELEMENT_NODE ) );
    lastDepth = _nodePath.size();
    currentNode->setId(getNodeName(node,"LOD").c_str());
    lastVisited = LOD;

    //TODO : get the most or less detailed node, not only the first one
    
    /*for ( unsigned int i = 0; i < node.getNumChildren(); i++ )
    {
        if ( node.getChild( i ) != NULL )
        {
            node.getChild( i )->accept( *this );
            break;
        }
    }*/
    //unsigned int cnt = node.getNumChildren();
    node.getChild( 0 )->accept( *this );
}

void daeWriter::apply( osg::ProxyNode &node ) 
{
    osg::notify( osg::WARN ) << "ProxyNode. Missing " << node.getNumChildren() << " children\n";
}



//LIGHT
void daeWriter::apply( osg::LightSource &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    domInstance_light *il = daeSafeCast< domInstance_light >( currentNode->createAndPlace( "instance_light" ) );
    std::string name = node.getName();
    if ( name.empty() )
    {
        name = uniquify( "light" );
    }
    std::string url = "#" + name;
    il->setUrl( url.c_str() );

    if ( lib_lights == NULL )
    {
        lib_lights = daeSafeCast< domLibrary_lights >( dom->createAndPlace( COLLADA_ELEMENT_LIBRARY_LIGHTS ) );
    }
    domLight *light = daeSafeCast< domLight >( lib_lights->createAndPlace( COLLADA_ELEMENT_LIGHT ) );
    light->setId( name.c_str() );
    
    lastVisited = LIGHT;

    traverse( node );
}

//CAMERA
void daeWriter::apply( osg::CameraNode &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    domInstance_camera *ic = daeSafeCast< domInstance_camera >( currentNode->createAndPlace( "instance_camera" ) );
    std::string name = node.getName();
    if ( name.empty() )
    {
        name = uniquify( "camera" );
    }
    std::string url = "#" + name;
    ic->setUrl( url.c_str() );

    if ( lib_cameras == NULL )
    {
        lib_cameras = daeSafeCast< domLibrary_cameras >( dom->createAndPlace( COLLADA_ELEMENT_LIBRARY_CAMERAS ) );
    }
    domCamera *cam = daeSafeCast< domCamera >( lib_cameras->createAndPlace( COLLADA_ELEMENT_CAMERA ) );
    cam->setId( name.c_str() );

    lastVisited = CAMERA;

    traverse( node );
}
