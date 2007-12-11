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
#include <dom/domConstants.h>

using namespace osgdae;

daeReader::daeReader(DAE *dae_) : dae(dae_),
                  rootNode(NULL),
                  m_numlights(0),
                  currentEffect(NULL),
                  currentInstance_effect(NULL),
                  geometryMap(),
                  materialMap(),
                  m_AuthoringTool(UNKNOWN),
                  m_AssetUnitName("meter"),
                  m_AssetUnitMeter(1.0),
                  m_AssetUp_axis(UPAXISTYPE_Y_UP)
{
}

daeReader::~daeReader()
{
}

bool daeReader::convert( const std::string &fileURI ) 
{
    daeElement *colladaElement;
    domInstance_rigid_body *irb;

    daeInt count, result;

    daeInt res = dae->load( fileURI.c_str() );
    
    if( res != DAE_OK && res != DAE_ERR_COLLECTION_ALREADY_EXISTS) 
    {
        osg::notify( osg::WARN ) << "Load failed in COLLADA DOM" << std::endl;
        return false;
    }
    osg::notify( osg::INFO ) << "URI loaded: " << fileURI << std::endl;

    domCOLLADA* document = dae->getDom( fileURI.c_str() );

    if ( !document->getScene() || !document->getScene()->getInstance_visual_scene() ) 
    {
        osg::notify( osg::WARN ) << "No scene found!" << std::endl;
        return false;
    }

    if (document->getAsset())
    {
        const domAsset::domContributor_Array& ContributorArray = document->getAsset()->getContributor_array();
        size_t NumberOfContributors = ContributorArray.getCount();
        size_t CurrentContributor;
        for (CurrentContributor = 0; CurrentContributor < NumberOfContributors; CurrentContributor++)
        {
            if (ContributorArray[CurrentContributor]->getAuthoring_tool())
            {
                xsString Tool = ContributorArray[CurrentContributor]->getAuthoring_tool()->getValue();
                if (strncmp(Tool, "Google SketchUp", 15) == 0)
                    m_AuthoringTool = GOOGLE_SKETCHUP;
            }
        }
        if (document->getAsset()->getUnit())
        {
            if (NULL != document->getAsset()->getUnit()->getName())
                m_AssetUnitName = std::string(document->getAsset()->getUnit()->getName());
            if (0 != document->getAsset()->getUnit()->getMeter())
                m_AssetUnitMeter = document->getAsset()->getUnit()->getMeter();
        }
        if (document->getAsset()->getUp_axis())
            m_AssetUp_axis = document->getAsset()->getUp_axis()->getValue();
    }

    if (dae->getDatabase()) {
        count = dae->getDatabase()->getElementCount(NULL, COLLADA_TYPE_INSTANCE_RIGID_BODY, NULL);

        // build a std::map for lookup if Group or PositionAttitudeTransform should be created, 
        // i.e, make it easy to check if a instance_rigid_body targets a visual node
        for (int i=0; i<count; i++) {
            result = dae->getDatabase()->getElement(&colladaElement, i, NULL, COLLADA_TYPE_INSTANCE_RIGID_BODY);

            if (result == DAE_OK) {
                irb = daeSafeCast<domInstance_rigid_body>(colladaElement);
                if (irb) {
                        domNode *node = daeSafeCast<domNode>(irb->getTarget().getElement());
                    if (node && node->getId()) {
                        _targetMap[ std::string(node->getId()) ] = true;          
                    }
                }
            }
        }
    }

    domInstanceWithExtra *ivs = document->getScene()->getInstance_visual_scene();
    domVisual_scene *vs = daeSafeCast< domVisual_scene >( getElementFromURI( ivs->getUrl() ) );
    if ( vs == NULL ) 
    {
        osg::notify( osg::WARN ) << "Unable to locate visual scene!" << std::endl;
        return false;
    }
    rootNode = processVisualScene( vs );

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
    osg::Node *retVal;
    osg::PositionAttitudeTransform *pat; 

    int patcount = node->getRotate_array().getCount() +
                node->getScale_array().getCount() +
                node->getTranslate_array().getCount();

    bool targeted = false;

    if (node->getId()) {
        targeted = _targetMap[std::string(node->getId())];
    }


    if (patcount > 0 || targeted ) 
    {
        pat = new osg::PositionAttitudeTransform();
        retVal = pat;
    } 
    else 
    {
        retVal = new osg::Group();
    }

    osg::Node *current = retVal;

    retVal->setName( node->getId() ? node->getId() : "" );


    // Handle rotate, translate and scale first..
    // will make the hierarchy less deep

    // <rotate>
    osg::Quat osgRot;
    for (unsigned int i=0; i<node->getRotate_array().getCount(); i++) 
    {
        daeSmartRef<domRotate> rot = node->getRotate_array().get(i);
        if (rot->getValue().getCount() != 4 ) {
            osg::notify(osg::WARN)<<"Data is wrong size for rotate"<<std::endl;
            continue;
        }

        domFloat4& r = rot->getValue();

        osg::Vec3 axis;
        axis.set(r[0],r[1],r[2]);
        osgRot =  osg::Quat(osg::DegreesToRadians(r[3]),axis) * osgRot;
        pat->setAttitude(osgRot);
    }

    // <scale>
    osg::Vec3 osgScale = osg::Vec3(1.0, 1.0, 1.0);
    for (unsigned int i=0; i<node->getScale_array().getCount(); i++) 
    {
        daeSmartRef<domScale> scale = node->getScale_array().get(i);

        if (scale->getValue().getCount() != 3 ) {
            osg::notify(osg::WARN)<<"Data is wrong size for scale"<<std::endl;
            continue;
        }
        domFloat3& s = scale->getValue();

        osgScale[0] *= s[0];
        osgScale[1] *= s[1];
        osgScale[2] *= s[2];
        pat->setScale(osgScale);
    }

    // <translate>
    osg::Vec3 osgTrans = osg::Vec3(0.0, 0.0, 0.0);
    for (unsigned int i=0; i<node->getTranslate_array().getCount(); i++) 
    {
        daeSmartRef<domTranslate> trans = node->getTranslate_array().get(i);

        if (trans->getValue().getCount() != 3 ) {
            osg::notify(osg::WARN)<<"Data is wrong size for translate"<<std::endl;
            continue;
        }

        domFloat3& t = trans->getValue();
        osgTrans += osg::Vec3(t[0],t[1],t[2]);
        pat->setPosition(osgTrans);
    }



    size_t count = node->getContents().getCount();
    for ( size_t i = 0; i < count; i++ ) 
    {
        osg::Node *trans = NULL;

        //I'm using daeSafeCast to check type because the pointer comparisons are a lot faster
        //than a strcmp
        domTranslate * t = daeSafeCast< domTranslate >( node->getContents()[i] );
        if ( t != NULL )
        {
            continue;
        }

        domRotate * r = daeSafeCast< domRotate >( node->getContents()[i] );
        if ( r != NULL ) {
            continue;
        }

        domScale * s = daeSafeCast< domScale >( node->getContents()[i] );
        if ( s != NULL ) {
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
