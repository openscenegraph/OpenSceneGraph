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
#include <dom/domCOLLADA.h>
#include <dom/domInstanceWithExtra.h>

using namespace osgdae;

daeReader::daeReader()
{
    dae = new DAE;
    m_numlights = 0;
    currentEffect = NULL;
}

daeReader::~daeReader()
{
    if ( dae )
    {
        delete dae;
        DAE::cleanup();
        dae = NULL;
    }
}

bool daeReader::convert( const std::string &fileURI ) 
{
    std::string fURI;
    if ( fileURI[1] == ':' )
    {
        fURI = "/" + fileURI;
    }
    else
    {
        fURI = fileURI;
    }
    if( dae->load( fURI.c_str() ) != DAE_OK ) 
    {
        osg::notify( osg::WARN ) << "Load failed in COLLADA DOM" << std::endl;
        return false;
    }
    osg::notify( osg::INFO ) << "URI loaded: " << fURI << std::endl;

    domCOLLADA* document = dae->getDom( fURI.c_str() );

    if ( !document->getScene() || !document->getScene()->getInstance_visual_scene() ) 
    {
        osg::notify( osg::WARN ) << "No scene found!" << std::endl;
        return false;
    }

    domInstanceWithExtra *ivs = document->getScene()->getInstance_visual_scene();
    domVisual_scene *vs = daeSafeCast< domVisual_scene >( getElementFromURI( ivs->getUrl() ) );
    if ( vs == NULL ) 
    {
        osg::notify( osg::WARN ) << "Unable to locate visual scene!" << std::endl;
        return false;
    }
    rootNode = processVisualScene( vs );

    delete dae;
    DAE::cleanup();
    dae = NULL;
    return true;
}

osg::Node* daeReader::processVisualScene( domVisual_scene *scene )
{
    osg::Node *retVal; 
    //### do not add an empty group if there is only one
    unsigned int nbVisualSceneGroup=scene->getNode_array().getCount();
    if (nbVisualSceneGroup==0)
    {
        osg::notify( osg::WARN ) << "No visual scene group found !" << std::endl;
        retVal = new osg::Group();
        retVal->setName("Empty Collada scene");
    }
    else if (nbVisualSceneGroup==1)
    {
        osg::Node *node = processNode( scene->getNode_array()[0] );
        if ( node != NULL )
           retVal = node;
        else
        {
           retVal = new osg::Group();
           retVal->setName("Empty Collada scene (import failure)");
        }
    }
    else
    { 
       retVal = new osg::Group();
       retVal->setName("Collada visual scene group");
       for ( size_t i = 0; i < scene->getNode_array().getCount(); i++ )
       {
          osg::Node *node = processNode( scene->getNode_array()[i] );
          if ( node != NULL )
          {
              retVal->asGroup()->addChild( node );
          }
       }
    }
    return retVal;
    
}

osg::Node* daeReader::processNode( domNode *node )
{
    osg::Node *retVal = new osg::Group();
    osg::Node *current = retVal;

    size_t count = node->getContents().getCount();
    for ( size_t i = 0; i < count; i++ ) 
    {
        osg::Node *trans = NULL;

        //I'm using daeSafeCast to check type because the pointer comparisons are a lot faster
        //than a strcmp

        //TODO: I am doing transforms wrong. A Transform is itself a group node. They need to be
        //consolodated and then the children of the collada node need to be the children of the
        //transform node.
        domTranslate * t = daeSafeCast< domTranslate >( node->getContents()[i] );
        if ( t != NULL )
        {
            trans = processTranslate( t );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
                current = trans;
            }
            continue;
        }

        domRotate * r = daeSafeCast< domRotate >( node->getContents()[i] );
        if ( r != NULL ) {
            trans = processRotate( r );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
                current = trans;
            }
            continue;
        }

        domScale * s = daeSafeCast< domScale >( node->getContents()[i] );
        if ( s != NULL ) {
            trans = processScale( s );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
                current = trans;
            }
            continue;
        }

        domMatrix * m = daeSafeCast< domMatrix >( node->getContents()[i] );
        if ( m != NULL ) {
            trans = processMatrix( m );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
                current = trans;
            }
            continue;
        }

        domSkew *sk = daeSafeCast< domSkew >( node->getContents()[i] );
        if ( sk != NULL ) {
            trans = processSkew( sk );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
                current = trans;
            }
            continue;
        }

                domInstance_geometry *ig = daeSafeCast< domInstance_geometry >( node->getContents()[i] );
        if ( ig != NULL )
        {
            trans = processInstance_geometry( ig );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
            }
            continue;
        }
        
        domInstance_controller *ictrl = daeSafeCast< domInstance_controller >( node->getContents()[i] );
        if ( ictrl != NULL )
        {
            trans = processInstance_controller( ictrl );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
            }
            continue;
        }

        domInstance_camera *ic = daeSafeCast< domInstance_camera >( node->getContents()[i] );
        if ( ic != NULL )
        {
            daeElement *el = getElementFromURI( ic->getUrl() );
            domCamera *c = daeSafeCast< domCamera >( el );
            if ( c == NULL )
            {
                osg::notify( osg::WARN ) << "Failed to locate camera " << ic->getUrl().getURI() << std::endl;
            }
            trans = processCamera( c );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
            }
            continue;
        }
        
        domInstance_light *il = daeSafeCast< domInstance_light >( node->getContents()[i] );
        if ( il != NULL )
        {
            daeElement *el = getElementFromURI( il->getUrl() );
            domLight *l = daeSafeCast< domLight >( el );
            if ( l == NULL )
            {
                osg::notify( osg::WARN ) << "Failed to locate light " << il->getUrl().getURI() << std::endl;
            }
            trans = processLight( l );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
            }
            continue;
        }

        domInstance_node *instn = daeSafeCast< domInstance_node >( node->getContents()[i] );
        if ( instn != NULL )
        {
            daeElement *el = getElementFromURI( instn->getUrl() );
            domNode *n = daeSafeCast< domNode >( el );
            if ( n == NULL )
            {
                osg::notify( osg::WARN ) << "Failed to locate camera " << ic->getUrl().getURI() << std::endl;
            }
            trans = processNode( n );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
            }
            continue;
        }

        domNode *n = daeSafeCast< domNode >( node->getContents()[i] );
        if ( n != NULL )
        {
            trans = processNode( n );
            if ( trans != NULL )
            {
                current->asGroup()->addChild( trans );
            }
            continue;
        }

        const char *name = node->getContents()[i]->getElementName();
        if ( name == NULL ) name = node->getContents()[i]->getTypeName();
        osg::notify( osg::WARN ) << "Unsupported element type: " << name << " in COLLADA scene!" << std::endl;

    }

    return retVal;
}
