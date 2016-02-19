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
#include <osg/ValueObject>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

using namespace osgDAE;

#ifdef COLLADA_DOM_2_4_OR_LATER
#include <dom/domAny.h>
using namespace ColladaDOM141;
#endif

daeReader::Options::Options() :
    strictTransparency(false),
    precisionHint(0),
    usePredefinedTextureUnits(true),
    tessellateMode(TESSELLATE_POLYGONS_AS_TRIFAN)   // Use old tessellation behaviour as default
{
}

daeReader::daeReader(DAE *dae_, const Options * pluginOptions) :
                  _dae(dae_),
                  _rootNode(NULL),
                  _document(NULL),
                  _visualScene(NULL),
                  _numlights(0),
                  _currentInstance_effect(NULL),
                  _currentEffect(NULL),
                  _authoringTool(UNKNOWN),
                  _invertTransparency(false),
                  _pluginOptions(pluginOptions ? *pluginOptions : Options()),
                  _assetUnitName("meter"),
                  _assetUnitMeter(1.0),
                  _assetUp_axis(UPAXISTYPE_Y_UP)
{
}

daeReader::~daeReader()
{
}

bool daeReader::processDocument( const std::string& fileURI)
{

    daeElement *colladaElement;


    daeInt count, result;

    if (!_document)
    {
        OSG_WARN << "Load failed in COLLADA DOM" << std::endl;
        return false;
    }
    OSG_INFO << "URI loaded: " << fileURI << std::endl;

    if ( !_document->getScene() || !_document->getScene()->getInstance_visual_scene() )
    {
        OSG_WARN << "No scene found!" << std::endl;
        return false;
    }

    if (_document->getAsset())
    {
        const domAsset::domContributor_Array& ContributorArray = _document->getAsset()->getContributor_array();
        size_t NumberOfContributors = ContributorArray.getCount();
        size_t CurrentContributor;
        for (CurrentContributor = 0; CurrentContributor < NumberOfContributors; CurrentContributor++)
        {
            if (ContributorArray[CurrentContributor]->getAuthoring_tool())
            {
                const char szBlender[] = "Blender";
                const char szDazStudio[] = "DAZ|Studio";
                const char szSketchup[] = "Google SketchUp";
                const char szFbx[] = "FBX";
                const char szMaya[] = "Maya";

                xsString Tool = ContributorArray[CurrentContributor]->getAuthoring_tool()->getValue();

                if (strncmp(Tool, szBlender, strlen(szBlender)) == 0)
                    _authoringTool = BLENDER;
                else if (strncmp(Tool, szDazStudio, strlen(szDazStudio)) == 0)
                    _authoringTool = DAZ_STUDIO;
                else if (strncmp(Tool, szFbx, strlen(szFbx)) == 0)
                    _authoringTool = FBX_CONVERTER;
                else if (strncmp(Tool, szSketchup, strlen(szSketchup)) == 0)
                    _authoringTool = GOOGLE_SKETCHUP;
                else if (strncmp(Tool, szMaya, strlen(szMaya)) == 0)
                    _authoringTool = MAYA;
            }
        }
        if (_document->getAsset()->getUnit())
        {
            if (NULL != _document->getAsset()->getUnit()->getName())
                _assetUnitName = std::string(_document->getAsset()->getUnit()->getName());
            if (0 != _document->getAsset()->getUnit()->getMeter())
                _assetUnitMeter = _document->getAsset()->getUnit()->getMeter();
        }
        if (_document->getAsset()->getUp_axis())
            _assetUp_axis = _document->getAsset()->getUp_axis()->getValue();
    }

    domInstanceWithExtra *ivs = _document->getScene()->getInstance_visual_scene();
    _visualScene = daeSafeCast< domVisual_scene >( getElementFromURI( ivs->getUrl() ) );
    if ( _visualScene == NULL )
    {
        OSG_WARN << "Unable to locate visual scene!" << std::endl;
        return false;
    }

    if (daeDatabase* database = _dae->getDatabase())
    {
        _invertTransparency = findInvertTransparency(database);

        // build a std::map for lookup if Group or PositionAttitudeTransform should be created,
        // i.e, make it easy to check if a instance_rigid_body targets a visual node
        domInstance_rigid_body *pDomInstanceRigidBody;
        count = database->getElementCount(NULL, COLLADA_TYPE_INSTANCE_RIGID_BODY, NULL);
        for (int i=0; i<count; i++)
        {
            result = database->getElement(&colladaElement, i, NULL, COLLADA_TYPE_INSTANCE_RIGID_BODY);

            if (result == DAE_OK)
            {
                pDomInstanceRigidBody = daeSafeCast<domInstance_rigid_body>(colladaElement);
                if (pDomInstanceRigidBody)
                {
                    domNode *node = daeSafeCast<domNode>(pDomInstanceRigidBody->getTarget().getElement());
                    if (node && node->getId())
                    {
                        _targetMap[ std::string(node->getId()) ] = true;
                    }
                }
            }
        }

        // Build a map of elements that are targeted by animations
        count = database->getElementCount(NULL, COLLADA_TYPE_CHANNEL, NULL);
        for (int i=0; i<count; i++)
        {
            result = database->getElement(&colladaElement, i, NULL, COLLADA_TYPE_CHANNEL);

            if (result == DAE_OK)
            {
                domChannel* pDomChannel = daeSafeCast<domChannel>(colladaElement);
                if (pDomChannel)
                {
                    std::string target = pDomChannel->getTarget();
                    size_t openparenthesis = target.find_first_of('(');
                    if (openparenthesis != std::string::npos) target.erase(openparenthesis);
                    daeSIDResolver resolver(pDomChannel, target.c_str());
                    daeElement *pDaeElement = resolver.getElement();
                    if (pDaeElement)
                    {
                        _daeElementDomChannelMap.insert(daeElementDomChannelMap::value_type(pDaeElement, pDomChannel));
                    }
                    else
                    {
                        OSG_WARN << "Could not locate <channel> target "  << pDomChannel->getTarget()<< std::endl;
                    }
                }
            }
        }

        // Find all nodes that are used as bones. Note that while many files
        // identify nodes with type="JOINT", some don't do this, while others
        // identify every node as a joint, making it meaningless.
        std::vector<domInstance_controller*> instanceControllers;
        database->typeLookup(instanceControllers);
        for (size_t i = 0; i < instanceControllers.size(); ++i)
        {
            domInstance_controller* pInstanceController = instanceControllers[i];

            domController *pDomController = daeSafeCast<domController>(getElementFromURI(pInstanceController->getUrl()));
            if (!pDomController)
            {
                OSG_WARN << "Failed to locate controller " << pInstanceController->getUrl().getURI() << std::endl;
                continue;
            }

            const domInstance_controller::domSkeleton_Array& domSkeletonURIs = pInstanceController->getSkeleton_array();
            std::vector<daeElement*> searchIn;

            for (size_t i = 0; i < domSkeletonURIs.getCount(); ++i)
            {
                if (daeElement* el = getElementFromURI(domSkeletonURIs[i]->getValue()))
                {
                    searchIn.push_back(el);
                    if (domNode* pJoint = daeSafeCast<domNode>(el))
                    {
                        _jointSet.insert(pJoint);
                    }
                }
            }

            if (searchIn.empty())
            {
                searchIn.push_back(_visualScene);
            }

            const domSkin* pSkin = pDomController->getSkin();
            if (!pSkin) continue;
            const domSkin::domJoints* pJoints = pSkin->getJoints();
            if (!pJoints) continue;
            const domInputLocal_Array& inputURIs = pJoints->getInput_array();

            domSource* pDomJointsSource = NULL;
            for (size_t i=0; i < inputURIs.getCount(); i++)
            {
                if (!strcmp(inputURIs[i]->getSemantic(), COMMON_PROFILE_INPUT_JOINT))
                {
                    pDomJointsSource = daeSafeCast<domSource>(getElementFromURI(inputURIs[i]->getSource()));
                    if (!pDomJointsSource)
                    {
                        OSG_WARN << "Could not find skin joints source '" << inputURIs[i]->getSource().getURI() << "'" <<std::endl;
                    }
                }
            }

            if (!pDomJointsSource)
            {
            }
            else if (domIDREF_array* pDomIDREFs = pDomJointsSource->getIDREF_array())
            {
                for (size_t i = 0; i < pDomIDREFs->getCount(); ++i)
                {
                    if (domNode* pJoint = daeSafeCast<domNode>(getElementFromIDRef(pDomIDREFs->getValue().get(i))))
                    {
                        _jointSet.insert(pJoint);
                    }
                }
            }
            else if (domName_array* pDomNames = pDomJointsSource->getName_array())
            {
                for (size_t i = 0; i < pDomNames->getCount(); ++i)
                {
                    daeString target = pDomNames->getValue().get(i);
                    for (size_t j = 0; j < searchIn.size(); ++j)
                    {
                        daeSIDResolver resolver(searchIn[j], target);
                        if (domNode* pJoint = daeSafeCast<domNode>(resolver.getElement()))
                        {
                            _jointSet.insert(pJoint);
                        }
                    }
                }
            }
        }
    }

    // Build the actual scene graph based on the visual scene
    _rootNode = processVisualScene( _visualScene );

    osgAnimation::BasicAnimationManager* pOsgAnimationManager = processAnimationLibraries(_document);
    if (pOsgAnimationManager)
    {
        _rootNode->addUpdateCallback(pOsgAnimationManager);
    }

    return true;
}

void daeReader::clearCaches ()
{
    _geometryMap.clear();
    _materialMap.clear();
    _materialMap2.clear();
}

bool daeReader::convert( std::istream& fin )
{
    clearCaches();

    // set fileURI to null device
    const std::string fileURI("from std::istream");

    // get the size of the file and rewind
    fin.seekg(0, std::ios::end);
    std::streampos length = fin.tellg();
    fin.seekg(0, std::ios::beg);

    // use a vector as buffer and read from stream
    std::vector<char> buffer(length);
    fin.read(&buffer[0], length);

    domElement* loaded_element = _dae->openFromMemory(fileURI, &buffer[0]);
    _document = dynamic_cast<domCOLLADA*>(loaded_element);

    return processDocument (fileURI);
}

bool daeReader::convert( const std::string &fileURI )
{
    clearCaches();

    domElement* loaded_element = _dae->open(fileURI);
    _document = dynamic_cast<domCOLLADA*>(loaded_element);

    return processDocument (fileURI);
}

void daeReader::addChild(osg::Group* group, osg::Node* node)
{
    if (dynamic_cast<osgAnimation::Bone*>(node))
    {
        unsigned index = 0;
        while (index < group->getNumChildren() &&
            dynamic_cast<osgAnimation::Bone*>(group->getChild(index)))
        {
            ++index;
        }
        group->insertChild(index, node);
    }
    else
    {
        group->addChild(node);
    }
}

osg::Group* daeReader::turnZUp()
{
    osg::PositionAttitudeTransform* pat = NULL;

    // If not Z axis up we need to rotate scene to bring the Z axis up
    if (_assetUp_axis != UPAXISTYPE_Z_UP)
    {
        pat = new osg::PositionAttitudeTransform();
        if (_assetUp_axis == UPAXISTYPE_Y_UP)
        {
            pat->setAttitude(osg::Quat(osg::inDegrees(90.0f), osg::Vec3(1.0f,0.0f,0.0f)));
        }
        else //(m_AssetUp_axis == UPAXISTYPE_X_UP)
        {
            pat->setAttitude(osg::Quat(osg::inDegrees(90.0f), osg::Vec3(0.0f,1.0f,0.0f)));
        }
    }

    _assetUp_axis = UPAXISTYPE_Z_UP;
    return pat;
}

osg::Group* daeReader::processVisualScene( domVisual_scene *scene )
{
    osg::Group *retVal;
    _rootStateSet = new osg::StateSet();

    unsigned int nbVisualSceneGroup=scene->getNode_array().getCount();
    if (nbVisualSceneGroup==0)
    {
        OSG_WARN << "No visual scene group found !" << std::endl;
        retVal = new osg::Group();
        retVal->setName("Empty Collada scene");
    }
    else
    {
        retVal = turnZUp();

        if (!retVal)
        {
            retVal = new osg::Group;
        }

        _skinInstanceControllers.clear();

        const domNode_Array& node_array = scene->getNode_array();
        for (size_t i = 0; i < node_array.getCount(); i++)
        {
            if (osg::Node* node = processNode(node_array[i], false))
            {
                addChild(retVal, node);
            }
        }

        processSkins();

        if (retVal->getName().empty())
        {
            if (retVal->getNumChildren())
            {
                retVal->setName("Collada visual scene group");
            }
            else
            {
                retVal->setName("Empty Collada scene (import failure)");
            }
        }
    }
    retVal->setStateSet(_rootStateSet.get());

    return retVal;
}



osg::Group* daeReader::processExtras(domNode *node)
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
                                OSG_WARN << "Child of element 'Descriptions' is not of type 'Description'" << std::endl;
                            }
                        }
                        else
                        {
                            OSG_WARN << "Element 'Descriptions' does not contain expected elements." << std::endl;
                        }
                    }
                    osgNode->setDescriptions(descriptions);
                }
                else
                {
                    OSG_WARN << "Expected element 'Descriptions' not found" << std::endl;
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
osg::Node* daeReader::processNode( domNode *node, bool skeleton)
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

    // See if it is targeted by an animation
    bool targeted = false;
    if (node->getId())
    {
        targeted = _targetMap[std::string(node->getId())];
    }


    osg::Group *resultNode = NULL;

    bool isBone = skeleton || isJoint(node);

    if (coordcount > 0 || targeted || isBone)
    {
        // TODO
        // single matrix -> MatrixTransform
        // scale, euler, translate -> PositionAttitudeTransform
        // if targeted -> StackedTransform
        // otherwise a flattened -> MatrixTransform
        resultNode = processOsgMatrixTransform(node, isBone);
    }
    else
    {
        // No transform data, determine node type based on it's available extra data
        resultNode = processExtras(node);
    }

    // See if there is generic node info attached as extra
    processNodeExtra(resultNode, node);

    if (resultNode->getName().empty())
    {
        std::string name = "";
        if (node->getId())
        {
            name = node->getId();
            resultNode->setUserValue("dae_node_id", name);
        }
        if (node->getName())
            name = node->getName();
        resultNode->setName( name );
    }

    osg::Group* attachTo = resultNode;

    if (!skeleton && isJoint(node))
    {
        skeleton = true;
        osgAnimation::Skeleton* pOsgSkeleton = getOrCreateSkeleton(node);
        pOsgSkeleton->addChild(resultNode);
        attachTo = resultNode;
        resultNode = pOsgSkeleton;
    }

    // 0..* <instance_camera>
    const domInstance_camera_Array& cameraInstanceArray = node->getInstance_camera_array();
    for ( size_t i = 0; i < cameraInstanceArray.getCount(); i++ )
    {
        daeElement *el = getElementFromURI( cameraInstanceArray[i]->getUrl());
        domCamera *c = daeSafeCast< domCamera >( el );

        if (c)
            addChild(attachTo, processCamera( c ));
        else
            OSG_WARN << "Failed to locate camera " << cameraInstanceArray[i]->getUrl().getURI() << std::endl;
    }

    // 0..* <instance_controller>
    const domInstance_controller_Array& controllerInstanceArray = node->getInstance_controller_array();
    for ( size_t i = 0; i < controllerInstanceArray.getCount(); i++ )
    {
        osg::Node* pOsgNode = processInstanceController( controllerInstanceArray[i]);

        // A skin controller may return NULL,  since the RigGeometry is added as
        // child of the skeleton and the skeleton already is added to the scenegraph
        if (pOsgNode)
        {
            addChild(attachTo, pOsgNode);
        }
    }

    // 0..* <instance_geometry>
    const domInstance_geometry_Array& geometryInstanceArray = node->getInstance_geometry_array();
    for ( size_t i = 0; i < geometryInstanceArray.getCount(); i++ )
    {
        addChild(attachTo, processInstanceGeometry( geometryInstanceArray[i] ));
    }

    // 0..* <instance_light>
    const domInstance_light_Array& lightInstanceArray = node->getInstance_light_array();
    for ( size_t i = 0; i < lightInstanceArray.getCount(); i++ )
    {
        daeElement *el = getElementFromURI( lightInstanceArray[i]->getUrl());
        domLight *pDomLight = daeSafeCast< domLight >( el );

        if (pDomLight)
            addChild(attachTo, processLight(pDomLight));
        else
            OSG_WARN << "Failed to locate light " << lightInstanceArray[i]->getUrl().getURI() << std::endl;
    }

    // 0..* <instance_node>
    const domInstance_node_Array& nodeInstanceArray = node->getInstance_node_array();
    for ( size_t i = 0; i < nodeInstanceArray.getCount(); i++ )
    {
        daeElement *el = getElementFromURI( nodeInstanceArray[i]->getUrl());
        domNode *n = daeSafeCast< domNode >( el );

        if (n)
            // Recursive call
            addChild(attachTo, processNode( n, skeleton ));
        else
            OSG_WARN << "Failed to locate node " << nodeInstanceArray[i]->getUrl().getURI() << std::endl;
    }

    // 0..* <node>
    const domNode_Array& nodeArray = node->getNode_array();
    for ( size_t i = 0; i < nodeArray.getCount(); i++ )
    {
        // Recursive call
        addChild(attachTo, processNode( nodeArray[i], skeleton ));
    }

    return resultNode;
}
