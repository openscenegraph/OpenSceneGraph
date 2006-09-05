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

#include <sstream>

using namespace osgdae;

daeWriter::daeWriter( const std::string &fname,bool _usePolygons ) : osg::NodeVisitor( TRAVERSE_ALL_CHILDREN ),
usePolygons (_usePolygons) 
{
    success = true;

    dae = new DAE();
    dae->setDatabase( NULL );
    dae->setIOPlugin( NULL );
    //create document
    dae->getDatabase()->createDocument( fname.c_str(), &doc );
    dom = (domCOLLADA*)doc->getDomRoot();
    //create scene and instance visual scene
    domCOLLADA::domScene *scene = daeSafeCast< domCOLLADA::domScene >( dom->createAndPlace( COLLADA_ELEMENT_SCENE ) );
    domInstanceWithExtra *ivs = daeSafeCast< domInstanceWithExtra >( scene->createAndPlace( "instance_visual_scene" ) );
    ivs->setUrl( "#defaultScene" );
    //create library visual scenes and a visual scene and the root node
    lib_vis_scenes = daeSafeCast<domLibrary_visual_scenes>( dom->createAndPlace( COLLADA_ELEMENT_LIBRARY_VISUAL_SCENES ) );
    vs = daeSafeCast< domVisual_scene >( lib_vis_scenes->createAndPlace( COLLADA_ELEMENT_VISUAL_SCENE ) );
    vs->setId( "defaultScene" );
    currentNode = daeSafeCast< domNode >( vs->createAndPlace( COLLADA_ELEMENT_NODE ) );
    currentNode->setId( "sceneRoot" );

    //create Asset
    createAssetTag();

    lib_cameras = NULL;
    lib_effects = NULL;
    lib_geoms = NULL;
    lib_lights = NULL;
    lib_mats = NULL;

    lastDepth = 0;
}

daeWriter::~daeWriter()
{
    if ( dae != NULL )
    {
        delete dae;
        DAE::cleanup();
        dae = 0;
    }
}

void daeWriter::debugPrint( osg::Node &node )
{
    std::string indent = "";
    for ( unsigned int i = 0; i < _nodePath.size(); i++ )
    {
        indent += "  ";
    }
    osg::notify( osg::INFO ) << indent << node.className() << std::endl;
}

bool daeWriter::writeFile()
{
    if ( dae->save( (daeUInt)0 ) != DAE_OK )
    {
        success = false;
    }
    return success;
}

void daeWriter::setRootNode( const osg::Node &node )
{
    std::string fname = osgDB::findDataFile( node.getName() );
    //rootName = fname.c_str();
    //rootName.validate();
}

//### provide a name to node
std::string daeWriter::getNodeName(const osg::Node & node,const std::string & defaultName)
{
    std::string nodeName;
    if ((node.getName().empty()) || (node.getName()!=""))
        nodeName=uniquify(defaultName);
    else
        nodeName=node.getName();
    return nodeName;
}

//NODE
void daeWriter::apply( osg::Node &node )
{
#ifdef _DEBUG
    debugPrint( node );
#endif

    osg::notify( osg::INFO ) << "generic node\n";
    lastVisited = NODE;

    traverse( node );
}


std::string daeWriter::uniquify( const std::string &name )
{
    std::map< std::string, int >::iterator iter = uniqueNames.find( name );
    if ( iter != uniqueNames.end() )
    {
        iter->second++;
        std::ostringstream num;
        num << std::dec << iter->second;
        return name + "_" + num.str();
    }
    else
    {
        uniqueNames.insert( std::make_pair( name, 0 ) );
        return name;
    }
    return "";
}

void daeWriter::createAssetTag()
{
    domAsset *asset = daeSafeCast< domAsset >(dom->createAndPlace( COLLADA_ELEMENT_ASSET ) );
    domAsset::domCreated *c = daeSafeCast< domAsset::domCreated >( asset->createAndPlace( "created" ) );
    domAsset::domModified *m = daeSafeCast< domAsset::domModified >( asset->createAndPlace( "modified" ) );
    domAsset::domUnit *u = daeSafeCast< domAsset::domUnit >( asset->createAndPlace( "unit" ) );

    //TODO : set date and time
    c->setValue( "2006-07-25T00:00:00Z" );
    m->setValue( "2006-07-25T00:00:00Z" );
    
    u->setName( "meter" );
    u->setMeter( 1 );
}
