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
#include <dae/domAny.h>
#include <dom/domCOLLADA.h>
#include <dom/domInstanceWithExtra.h>
#include <dom/domConstants.h>
#include <osg/MatrixTransform>

using namespace osgdae;

daeReader::daeReader(DAE *dae_, bool strictTransparency) :
                  m_AssetUnitName("meter"),
                  m_AssetUnitMeter(1.0),
                  m_AssetUp_axis(UPAXISTYPE_Y_UP),
                  dae(dae_),
                  rootNode(NULL),
                  m_numlights(0),
                  currentInstance_effect(NULL),
                  currentEffect(NULL),
                  m_AuthoringTool(UNKNOWN),
                  m_StrictTransparency(strictTransparency)
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

    if (dae->getDatabase()) 
    {
        count = dae->getDatabase()->getElementCount(NULL, COLLADA_TYPE_INSTANCE_RIGID_BODY, NULL);

        // build a std::map for lookup if Group or PositionAttitudeTransform should be created, 
        // i.e, make it easy to check if a instance_rigid_body targets a visual node
        for (int i=0; i<count; i++) 
        {
            result = dae->getDatabase()->getElement(&colladaElement, i, NULL, COLLADA_TYPE_INSTANCE_RIGID_BODY);

            if (result == DAE_OK) 
            {
                irb = daeSafeCast<domInstance_rigid_body>(colladaElement);
                if (irb) 
                {
                    domNode *node = daeSafeCast<domNode>(irb->getTarget().getElement());
                    if (node && node->getId()) 
                    {
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

osg::Node* daeReader::processExtras(domNode *node)
{
    // See if one of the extras contains OpenSceneGraph specific information
    unsigned int numExtras = node->getExtra_array().getCount();
    for (unsigned int currExtra=0; currExtra < numExtras; currExtra++)
    {
        domExtra* extra = node->getExtra_array()[currExtra];
        domTechnique* teq = NULL;

        daeString extraType = extra->getType();
        if (extraType)
        {
            if (strcmp(extraType, "Switch") == 0)
            {
                teq = getOpenSceneGraphProfile(extra);
                if (teq)
                {
                    return processOsgSwitch(teq);
                }
            }
            else if (strcmp(extraType, "MultiSwitch") == 0)
            {
                teq = getOpenSceneGraphProfile(extra);
                if (teq)
                {
                    return processOsgMultiSwitch(teq);
                }
            }
            else if (strcmp(extraType, "LOD") == 0)
            {
                teq = getOpenSceneGraphProfile(extra);
                if (teq)
                {
                    return processOsgLOD(teq);
                }
            }
            else if (strcmp(extraType, "DOFTransform") == 0)
            {
                teq = getOpenSceneGraphProfile(extra);
                if (teq)
                {
                    return processOsgDOFTransform(teq);
                }
            }
            else if (strcmp(extraType, "Sequence") == 0)
            {
                teq = getOpenSceneGraphProfile(extra);
                if (teq)
                {
                    return processOsgSequence(teq);
                }
            }
        }
    }
    return new osg::Group;
}

void daeReader::processNodeExtra(osg::Node* osgNode, domNode *node)
{
    // See if one of the extras contains OpenSceneGraph specific information
    unsigned int numExtras = node->getExtra_array().getCount();

    for (unsigned int currExtra=0; currExtra < numExtras; currExtra++)
    {
        domExtra* extra = node->getExtra_array()[currExtra];

        daeString extraType = extra->getType();
        if (extraType && (strcmp(extraType, "Node") == 0))
        {
            domTechnique* teq = getOpenSceneGraphProfile(extra);
            if (teq)
            {
                domAny* any = daeSafeCast< domAny >(teq->getChild("Descriptions"));
                if (any)
                {
                    osg::Node::DescriptionList descriptions;
                    unsigned int numChildren = any->getChildren().getCount();
                    for (unsigned int currChild = 0; currChild < numChildren; currChild++)
                    {
                        domAny* child = daeSafeCast<domAny>(any->getChildren()[currChild]);
                        if (child)
                        {
                            if (strcmp(child->getElementName(), "Description" ) == 0 )
                            {
                                std::string value = child->getValue();
                                descriptions.push_back(value);
                            }
                            else
                            {
                                osg::notify(osg::WARN) << "Child of element 'Descriptions' is not of type 'Description'" << std::endl;
                            }
                        }
                        else
                        {
                            osg::notify(osg::WARN) << "Element 'Descriptions' does not contain expected elements." << std::endl;
                        }
                    }
                    osgNode->setDescriptions(descriptions);
                }
                else
                {
                    osg::notify(osg::WARN) << "Expected element 'Descriptions' not found" << std::endl;
                }
            }
        }
    }
}

domTechnique* daeReader::getOpenSceneGraphProfile(domExtra* extra)
{
    unsigned int numTeqs = extra->getTechnique_array().getCount();

    for ( unsigned int currTeq = 0; currTeq < numTeqs; ++currTeq )
    {
        // Only interested in OpenSceneGraph technique
        if (strcmp( extra->getTechnique_array()[currTeq]->getProfile(), "OpenSceneGraph" ) == 0 )
        {
            return extra->getTechnique_array()[currTeq];
        }
    }
    return NULL;
}


// <node>
// attributes:
// id, name, sid, type, layer
// child elements:
// 0..1 <asset>
// 0..* <lookat>, <matrix>, <rotate>, <scale>, <skew>, <translate>
// 0..* <instance_camera>
// 0..* <instance_controller>
// 0..* <instance_geometry>
// 0..* <instance_light>
// 0..* <instance_node>
// 0..* <node>
// 0..* <extra>
osg::Node* daeReader::processNode( domNode *node )
{
    // First we need to determine what kind of OSG node we need
    // If there exist any of the <lookat>, <matrix>, <rotate>, <scale>, <skew>, <translate> elements
    // or if a COLLADA_TYPE_INSTANCE_RIGID_BODY targets this node we need a MatrixTransform
    int coordcount =    node->getRotate_array().getCount() +
                        node->getScale_array().getCount() +
                        node->getTranslate_array().getCount() +
                        node->getLookat_array().getCount() +
                        node->getMatrix_array().getCount() +
                        node->getSkew_array().getCount();

    // See if it is targeted
    bool targeted = false;
    if (node->getId()) 
    {
        targeted = _targetMap[std::string(node->getId())];
    }

    osg::Node *resultNode;
    if (coordcount > 0 || targeted ) 
    {
        resultNode = processOsgMatrixTransform(node);
    }
    else
    {
        // No transform data, determine node type based on it's available extra data
        resultNode = processExtras(node);
    }

    // See if there is generic node info attached as extra
    processNodeExtra(resultNode, node);

    resultNode->setName( node->getId() ? node->getId() : "" );

    osg::Group* groupNode = resultNode->asGroup();

    if (groupNode)
    {
        // 0..* <instance_camera>
        domInstance_camera_Array cameraInstanceArray = node->getInstance_camera_array();
        for ( size_t i = 0; i < cameraInstanceArray.getCount(); i++ ) 
        {
            daeElement *el = getElementFromURI( cameraInstanceArray[i]->getUrl());
            domCamera *c = daeSafeCast< domCamera >( el );

            if (c)
                groupNode->addChild( processCamera( c ));
            else
                osg::notify( osg::WARN ) << "Failed to locate camera " << cameraInstanceArray[i]->getUrl().getURI() << std::endl;
        }

        // 0..* <instance_controller>
        domInstance_controller_Array controllerInstanceArray = node->getInstance_controller_array();
        for ( size_t i = 0; i < controllerInstanceArray.getCount(); i++ ) 
        {
            groupNode->addChild( processInstanceController( controllerInstanceArray[i] ));
        }

        // 0..* <instance_geometry>
        domInstance_geometry_Array geometryInstanceArray = node->getInstance_geometry_array();
        for ( size_t i = 0; i < geometryInstanceArray.getCount(); i++ ) 
        {
            groupNode->addChild( processInstanceGeometry( geometryInstanceArray[i] ));
        }

        // 0..* <instance_light>
        domInstance_light_Array lightInstanceArray = node->getInstance_light_array();
        for ( size_t i = 0; i < lightInstanceArray.getCount(); i++ ) 
        {
            daeElement *el = getElementFromURI( lightInstanceArray[i]->getUrl());
            domLight *l = daeSafeCast< domLight >( el );
            
            if (l)
                groupNode->addChild( processLight( l ));
            else
                osg::notify( osg::WARN ) << "Failed to locate light " << lightInstanceArray[i]->getUrl().getURI() << std::endl;
        }

        // 0..* <instance_node>
        domInstance_node_Array nodeInstanceArray = node->getInstance_node_array();
        for ( size_t i = 0; i < nodeInstanceArray.getCount(); i++ ) 
        {
            daeElement *el = getElementFromURI( nodeInstanceArray[i]->getUrl());
            domNode *n = daeSafeCast< domNode >( el );

            if (n)
                // Recursive call
                groupNode->addChild( processNode( n ));
            else
                osg::notify( osg::WARN ) << "Failed to locate node " << nodeInstanceArray[i]->getUrl().getURI() << std::endl;
        }

        // 0..* <node>
        domNode_Array nodeArray = node->getNode_array();
        for ( size_t i = 0; i < nodeArray.getCount(); i++ ) 
        {
            // Recursive call
            groupNode->addChild( processNode( nodeArray[i] ));
        }
    }

    return resultNode;
}
