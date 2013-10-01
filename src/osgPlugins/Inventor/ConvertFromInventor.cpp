#include "ConvertFromInventor.h"

#include "PendulumCallback.h"
#include "ShuttleCallback.h"

// OSG headers
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Notify>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/TexEnv>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/Depth>
#include <osg/LightModel>
#include <osg/LightSource>
#include <osg/ShadeModel>
#include <osg/LOD>
#include <osgDB/ReadFile>
#include <osgUtil/TransformCallback>

// Inventor headers
#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/nodes/SoLight.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoSpotLight.h>
#include <Inventor/nodes/SoPointLight.h>
#include <Inventor/nodes/SoRotor.h>
#include <Inventor/nodes/SoPendulum.h>
#include <Inventor/nodes/SoShuttle.h>
#include <Inventor/nodes/SoLOD.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoEnvironment.h>
#include <Inventor/misc/SoChildList.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/SbLinear.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>

#ifdef __COIN__
#include <Inventor/VRMLnodes/SoVRMLImageTexture.h>
#include <Inventor/VRMLnodes/SoVRMLTransform.h>
#include <Inventor/VRMLnodes/SoVRMLAppearance.h>
#include <Inventor/VRMLnodes/SoVRMLMaterial.h>
#include <Inventor/lists/SbStringList.h>
#endif

#if defined(__COIN__) && (COIN_MAJOR_VERSION >= 3 || \
    (COIN_MAJOR_VERSION == 2 && COIN_MINOR_VERSION>=5))
#define INVENTOR_SHADERS_AVAILABLE
#endif

#ifdef INVENTOR_SHADERS_AVAILABLE
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoGeometryShader.h>
#include <Inventor/nodes/SoFragmentShader.h>
#endif

#include <map>
#include <assert.h>
#include <math.h>
#include <string.h>
#ifdef __linux
#include <values.h>
#endif
#ifdef __APPLE__
#include <float.h>
#endif

#define DEBUG_IV_PLUGIN
#define NOTIFY_HEADER "Inventor Plugin (reader): "

///////////////////////////////////////////
ConvertFromInventor::ConvertFromInventor()
{
    numPrimitives = 0;
}
///////////////////////////////////////////
ConvertFromInventor::~ConvertFromInventor()
{
}
///////////////////////////////////////////////////////////////////
static bool
nodePreservesState(const SoNode *node)
{
    return node->isOfType(SoSeparator::getClassTypeId()) ||
           (node->getChildren() != NULL && node->affectsState() == FALSE);
}
////////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::restructure(void* data, SoCallbackAction* action,
                                 const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "restructure() "
              << node->getTypeId().getName().getString();
#endif

    int childrenTotal = 0;
    int numModifiedChildren = 0;
    int numRemovedNodes = 0;
    std::vector<std::vector<int> > &stack = *((std::vector<std::vector<int> >*)data);

    if (node->isOfType(SoGroup::getClassTypeId())) {

        SoGroup *group = (SoGroup*)node;
        SoGroup *affectedScene = NULL;
        childrenTotal = group->getNumChildren();

        for (int i=0, c=group->getNumChildren(); i<c; i++) {
            SoNode *child = group->getChild(i);
            if (!child->isOfType(SoSeparator::getClassTypeId()) &&
                child->affectsState()) {

                // Put the node bellow separator
                SoSeparator *s = new SoSeparator;
                s->addChild(group->getChild(i));
                group->replaceChild(i, s);
                numModifiedChildren++;

                // Create the scene that may be affected by the node
                if (!affectedScene) {

                    // Create the graph of nodes that may be influenced
                    // by the node
                    const SoFullPath *path = (const SoFullPath*)action->getCurPath();
                    //assert(path->getLength() == 0 ||
                    //       path->getNode(path->getLength()-1) == group &&
                    //       "Group being restructured is not at the end of the path.");
                    int stackLevel = stack.size()-2;
                    for (int j=path->getLength()-2; j>=0; j--, stackLevel--) {

                        // Get the appropriate stack level of nodesToRemove
                        assert(stackLevel >=0);
                        std::vector<int> &nodesToRemove = stack[stackLevel];

                        // Get parent and index of the current group
                        SoNode *parent = path->getNode(j);
                        int childIndex = path->getIndex(j+1);
                        const SoChildList *chl = parent->getChildren();
                        assert(chl->operator[](childIndex) == path->getNode(j+1) &&
                               "Wrong indexing.");

                        // Create affected scene graph
                        if (!affectedScene)
                            affectedScene = new SoGroup;

                        // Copy nodes to the graph
                        for (int k=childIndex+1, n=chl->getLength(); k<n; k++) {
                            affectedScene->addChild(chl->operator[](k));
                            nodesToRemove.push_back(k);
                            numRemovedNodes++;
                        }

                        // Stop recursion if we reached separator
                        // or other state-preserving node.
                        if (nodePreservesState(parent))
                            break;
                    }
                }

                // Append the affected graph to the separator
                s->addChild(affectedScene);
            }
        }
    }

#ifdef DEBUG_IV_PLUGIN
    if (numModifiedChildren == 0)
    {
        OSG_DEBUG << ": no changes necessary" << std::endl;
    }
    else
    {
        OSG_DEBUG << ": " << numModifiedChildren <<
                  " nodes of " << childrenTotal << " restruc., " <<
                  numRemovedNodes << " removed" << std::endl;
    }
#endif

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::restructurePreNode(void* data, SoCallbackAction* action,
                             const SoNode* node)
{
    std::vector<std::vector<int> > &stack = *((std::vector<std::vector<int> >*)data);

    stack.push_back(std::vector<int>());

    return SoCallbackAction::CONTINUE;
}
////////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::restructurePostNode(void* data, SoCallbackAction* action,
                              const SoNode* node)
{
    std::vector<std::vector<int> > &stack = *((std::vector<std::vector<int> >*)data);

    assert(stack.size() > 0 && "Stack is empty");
    std::vector<int> &nodesToRemove = stack.back();

    if (nodesToRemove.size() > 0) {

#ifdef DEBUG_IV_PLUGIN
        OSG_DEBUG << NOTIFY_HEADER << "postNode()   "
                  << node->getTypeId().getName().getString()
                  << " (level " << stack.size() << ") removed "
                  << nodesToRemove.size() << " node(s)" << std::endl;
#endif

        assert(node->getChildren());
        for (int i=nodesToRemove.size()-1; i>=0; i--) {
            //assert(i==0 || nodesToRemove[i-1] < nodesToRemove[i] &&
            //       "Children to remove are not in order.");
            node->getChildren()->remove(nodesToRemove[i]);
        }
    }

    stack.pop_back();

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////////
void
ConvertFromInventor::preprocess(SoNode* root)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "Preprocessing..." << std::endl;
#endif

    SoCallbackAction action;
    std::vector<std::vector<int> > stackOfNodesToRemove;

    // Callbacks for troublesome nodes
    action.addPreCallback(SoNode::getClassTypeId(),
                          restructurePreNode, &stackOfNodesToRemove);
    action.addPostCallback(SoLOD::getClassTypeId(),
                           restructure, &stackOfNodesToRemove);
    action.addPostCallback(SoNode::getClassTypeId(),
                           restructurePostNode, &stackOfNodesToRemove);

    // Traverse the scene
    action.apply(root);

#if 0 // For debugging purposes: Write preprocessed scene to the file
    SoOutput out;
    out.openFile("preprocess.iv");
    SoWriteAction wa(&out);
    wa.apply(root);
#endif
}
///////////////////////////////////////////////////////////
osg::Node*
ConvertFromInventor::convert(SoNode* ivRootNode)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "Converting..." << std::endl;
#endif

    // Transformation matrix for converting Inventor coordinate system to OSG
    // coordinate system
    osg::Matrix ivToOSGMat(osg::Matrix(1.0, 0.0, 0.0, 0.0,
                                       0.0, 0.0, 1.0, 0.0,
                                       0.0,-1.0, 0.0, 0.0,
                                       0.0, 0.0, 0.0, 1.0));

    // Root of the scene
    osg::ref_ptr<osg::Group> osgRootNode = new osg::MatrixTransform(ivToOSGMat);

    // Propagate node name
    osgRootNode->setName(ivRootNode->getName().getString());

    // Initialize Inventor state stack
    // (ivStateStack is used to track the state that is not accessible by
    // SoCallbackAction functions)
    ivStateStack.push(IvStateItem(ivRootNode, osgRootNode.get()));

    // Create callback actions for the inventor nodes
    // These callback functions perform the conversion
    // note: if one class is derived from the other and both callbacks
    // are registered, both functions will be called
    SoCallbackAction cbAction;

    // Node callbacks are used for detecting which node
    // preserves state (like SoSeparator) and which not.
    // There are few nodes that behave like SoSeparator although they
    // are not derived from it.
    // Note: postNode callback is moved down, because it must be
    // called as the last callback.
    cbAction.addPreCallback(SoNode::getClassTypeId(), preNode, this);

    // SoTransformSeparator callbacks. Special handling of transformations.
    cbAction.addPreCallback(SoTransformSeparator::getClassTypeId(), preTransformSeparator, this);
    cbAction.addPostCallback(SoTransformSeparator::getClassTypeId(), postTransformSeparator, this);

    // LOD (Level of Detail) callbacks. Handles SoLOD nodes.
    // FIXME: SoLevelOfDetail needs to be implemented and tested.
    cbAction.addPreCallback(SoLOD::getClassTypeId(), preLOD, this);
    cbAction.addPostCallback(SoLOD::getClassTypeId(), postLOD, this);

    // Shape callbacks collects all triangles and all the geometry data.
    // Moreover, they handle transformations, ...
    cbAction.addPreCallback(SoShape::getClassTypeId(), preShape, this);
    cbAction.addPostCallback(SoShape::getClassTypeId(), postShape, this);

    // Handling of textures
    cbAction.addPostCallback(SoTexture2::getClassTypeId(),
                            postTexture, this);
#ifdef __COIN__
    cbAction.addPostCallback(SoVRMLImageTexture::getClassTypeId(),
                            postTexture, this);
    cbAction.addPostCallback(SoVRMLAppearance::getClassTypeId(),
                            postTexture, this);
#endif

#ifdef __COIN__
    cbAction.addPreCallback(SoInfo::getClassTypeId(), preInfo, this);
#endif

    // Lights
    cbAction.addPreCallback(SoLight::getClassTypeId(), preLight, this);

    // Environment (ambient light,...)
    cbAction.addPreCallback(SoEnvironment::getClassTypeId(), preEnvironment, this);

    // Shaders
#ifdef INVENTOR_SHADERS_AVAILABLE
    cbAction.addPreCallback(SoShaderProgram::getClassTypeId(), preShaderProgram, this);
#endif

    // Motion callbacks
    cbAction.addPreCallback(SoRotor::getClassTypeId(), preRotor, this);
    cbAction.addPreCallback(SoPendulum::getClassTypeId(), prePendulum, this);
    cbAction.addPreCallback(SoShuttle::getClassTypeId(), preShuttle, this);

    // Geometry callbacks
    cbAction.addTriangleCallback(SoShape::getClassTypeId(), addTriangleCB, this);
    cbAction.addLineSegmentCallback(SoShape::getClassTypeId(), addLineSegmentCB,
                                    this);
    cbAction.addPointCallback(SoShape::getClassTypeId(), addPointCB, this);

    // Post node callback
    cbAction.addPostCallback(SoNode::getClassTypeId(), postNode, this);

    // Traverse the inventor scene graph
    cbAction.apply(ivRootNode);

    // Remove superfluous group
    if (osgRootNode->getNumChildren() == 1) {
        osg::ref_ptr<osg::Group> toRemove = osgRootNode->getChild(0)->asGroup();
        assert(toRemove.get() &&
               strcmp(toRemove->className(), "Group") == 0 &&
               "IvStateStack osg graph is expected to be "
               "headed by osg::Group");
        osgRootNode->removeChild(0u);
        for (int i=0, c=toRemove->getNumChildren(); i<c; i++)
            osgRootNode->addChild(toRemove->getChild(i));
    }

    return osgRootNode.get();
}
///////////////////////////////////////////////////////////////////
static void
notifyAboutMatrixContent(const osg::NotifySeverity level, const SbMatrix &m)
{
    SbVec3f t,s;
    SbRotation r,so;
    m.getTransform(t, r, s, so);
    SbVec3f axis;
    float angle;
    r.getValue(axis, angle);
    OSG_NOTIFY(level) << NOTIFY_HEADER << "  Translation: " <<
              t[0] << "," << t[1] << "," << t[2] << std::endl;
    OSG_NOTIFY(level) << NOTIFY_HEADER << "  Rotation: (" <<
              axis[0] << "," << axis[1] << "," << axis[2] << ")," << angle << std::endl;
}
///////////////////////////////////////////////////////////////////
void
ConvertFromInventor::appendNode(osg::Node *n, const SoCallbackAction *action)
{
    IvStateItem &ivState = ivStateStack.top();
    SbMatrix currentMatrix = action->getModelMatrix();
    SbMatrix inheritedMatrix = ivState.inheritedTransformation;

    // Keep children order - this must be done for some nodes like
    // SoSwitch, SoLOD,...
    // We will append dummy nodes if the child is expected to be on
    // higher index.
    if (ivState.flags & IvStateItem::KEEP_CHILDREN_ORDER) {

        // Determine child index
        int childIndex = -1;
        const SoFullPath *path = (const SoFullPath*)(((SoCallbackAction*)action)->getCurPath());
        for (int i=path->getLength()-2; i>=0; i--)
            if (path->getNode(i) == ivState.keepChildrenOrderParent) {
                childIndex = path->getIndex(i+1);
                assert(ivState.keepChildrenOrderParent->getChildren());
                assert((ivState.keepChildrenOrderParent->getChildren()->operator[](childIndex) == path->getNode(i+1)) && "Indexing is wrong.");
                break;
            }
        assert(childIndex != -1 && "Node did not found.");

        // Append dummy nodes to keep children order
        assert(int(ivState.osgStateRoot->getNumChildren()) <= childIndex &&
               "Number of children in ivState.osgStateRoot is too big.");
        while (int(ivState.osgStateRoot->getNumChildren()) < childIndex)
            ivState.osgStateRoot->addChild(new osg::Node);
    }

#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "appendNode: "
              << n->className();
#endif

    if (currentMatrix == inheritedMatrix) {

        // just append node to the current group in osg scene graph
        ivState.osgStateRoot->addChild(n);
        ivState.lastUsedTransformation = inheritedMatrix;

#ifdef DEBUG_IV_PLUGIN
        if (osg::isNotifyEnabled(osg::DEBUG_INFO))
            OSG_DEBUG <<
                      " uses parent transformation" << std::endl;
#endif

    } else {

        if (!(ivState.flags & IvStateItem::KEEP_CHILDREN_ORDER) &&
            currentMatrix == ivState.lastUsedTransformation) {

            // Previous node has the same transformation. Let's use it.
            assert(ivState.osgStateRoot->getNumChildren() != 0 &&
                   "This should never happen - there is no item on "
                   "osgShapeGraphs list while want to use last one.");
            osg::Transform *t = ivState.osgStateRoot->getChild(ivState.osgStateRoot->getNumChildren()-1)->asTransform();
            assert(t && "This should never happen - want to use "
                        "transformation of previous scene geometry "
                        "and it does not have Transform node.");
            t->addChild(n);

#ifdef DEBUG_IV_PLUGIN
            if (osg::isNotifyEnabled(osg::DEBUG_INFO))
                OSG_DEBUG <<
                          " reuses previous transformation" << std::endl;
#endif

        } else {

            // We need a new transformation node
            osg::Matrix m(osg::Matrix(currentMatrix.operator float*()));
            osg::Matrix m2;
            m2.invert(osg::Matrix(inheritedMatrix.operator float*()));
            m.postMult(m2);
            osg::MatrixTransform *mt = new osg::MatrixTransform(m);
            mt->addChild(n);

            ivState.osgStateRoot->addChild(mt);
            ivState.lastUsedTransformation = currentMatrix;

#ifdef DEBUG_IV_PLUGIN
            if (osg::isNotifyEnabled(osg::DEBUG_INFO)) {
                OSG_DEBUG <<
                          " uses local transformation:" << std::endl;
                notifyAboutMatrixContent(osg::DEBUG_INFO,
                          SbMatrix((SbMat&)(*osg::Matrixf(m).ptr())));
            }
#endif
        }
    }
}
///////////////////////////////////////////////////////////////////
void
ConvertFromInventor::ivPushState(const SoCallbackAction *action,
                                 const SoNode *initiator, const int flags,
                                 osg::Group *root)
{
    assert(ivStateStack.size() >= 1 && "There must be at least one "
           "value in the ivStateStack to use ivPushState function.");

    // Propagate node name
    root->setName(initiator->getName().getString());

    // APPEND_AT_PUSH
    if (flags & IvStateItem::APPEND_AT_PUSH)
        appendNode(root, action);

    // Push state
    ivStateStack.push(IvStateItem(ivStateStack.top(), action, initiator, flags, root));

}
///////////////////////////////////////////////////////////////////
void
ConvertFromInventor::ivPopState(const SoCallbackAction *action,
                                const SoNode *initiator)
{
    bool multipop;
    do {
        assert(ivStateStack.size() >= 2 && "There must be at least two "
               "values in the ivStateStack to use ivPopState function.");

        // Get multipop value
        IvStateItem ivState = ivStateStack.top();
        multipop = ivState.flags & IvStateItem::MULTI_POP;
        //assert(multipop ||
        //       ivState.pushInitiator == initiator &&
        //       "ivStateStack push was initiated by different node.");

        // Get osgStateRoot (note: we HAVE TO reference it)
        osg::ref_ptr<osg::Group> r = ivState.osgStateRoot;

        // Pop state
        ivStateStack.pop();

        // Update state from already popped values
        if ((ivState.flags & (IvStateItem::UPDATE_STATE |
            IvStateItem::UPDATE_STATE_EXCEPT_TRANSFORM)) != 0) {
            IvStateItem &newTop = ivStateStack.top();
            newTop.currentTexture = ivState.currentTexture;
            newTop.currentLights = ivState.currentLights;
            newTop.currentGLProgram = ivState.currentGLProgram;
        }

        // APPEND_AT_PUSH
        if (!(ivState.flags & IvStateItem::APPEND_AT_PUSH))
            appendNode(r.get(), action);

    } while (multipop);

}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preNode(void* data, SoCallbackAction* action,
                             const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preNode()    "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    if (nodePreservesState(node)) {

        // push state
        ConvertFromInventor *thisPtr = (ConvertFromInventor *) data;
        thisPtr->ivPushState(action, node);
#ifdef DEBUG_IV_PLUGIN
        if (osg::isNotifyEnabled(osg::DEBUG_INFO)) {
            OSG_DEBUG << NOTIFY_HEADER << "push state, saved values: " << std::endl;
            notifyAboutMatrixContent(osg::DEBUG_INFO, action->getModelMatrix());
        }
#endif
    }

    return SoCallbackAction::CONTINUE;
}
////////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::postNode(void* data, SoCallbackAction* action,
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "postNode()   "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    if (nodePreservesState(node)) {

        // pop state
        ConvertFromInventor *thisPtr = (ConvertFromInventor *) data;
        assert(thisPtr->ivStateStack.size() > 0 && "ivStackState underflow");
        thisPtr->ivPopState(action, node);

#ifdef DEBUG_IV_PLUGIN
        if (osg::isNotifyEnabled(osg::DEBUG_INFO)) {
            OSG_DEBUG << NOTIFY_HEADER <<
                      "pop state, restored transformation: " << std::endl;
            notifyAboutMatrixContent(osg::DEBUG_INFO, action->getModelMatrix());
        }
#endif
    }

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preTransformSeparator(void* data, SoCallbackAction* action,
                             const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preTransformSeparator()    "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    // push state
    ConvertFromInventor *thisPtr = (ConvertFromInventor *) data;
    thisPtr->ivPushState(action, node, IvStateItem::UPDATE_STATE_EXCEPT_TRANSFORM,
                         new osg::Group());

    return SoCallbackAction::CONTINUE;
}
////////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::postTransformSeparator(void* data, SoCallbackAction* action,
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "postTransformSeparator()   "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    // pop state
    ConvertFromInventor *thisPtr = (ConvertFromInventor *) data;
    assert(thisPtr->ivStateStack.size() > 0 && "ivStackState underflow");
    thisPtr->ivPopState(action, node);

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preLOD(void* data, SoCallbackAction* action,
                            const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preLOD()   "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    // init values
    ConvertFromInventor* thisPtr = (ConvertFromInventor*)data;

    // SoLOD
    // Note: It is not possible to convert SoLOD to osg:LOD
    // in any non-complex algorithm, because SoLOD does not preserves
    // traversal state (like SoSeparator). Thus, following example
    // can not be easily converted:
    //
    // SoLOD {
    //   range [...]
    //   Complexity { value 0.1 }
    //   Complexity { value 0.2 }
    //   Complexity { value 0.3 }
    // }
    // Sphere {}
    //
    // It was decided that it is necessary to preprocess scene
    // in a way to avoid any state to come out of SoLOD. For example:
    //
    // SoLOD {
    //   range [...]
    //   Separator {
    //     Complexity { value 0.1 }
    //     DEF mySphere Sphere {}
    //   }
    //   Separator {
    //     Complexity { value 0.2 }
    //     USE mySphere
    //   }
    //   Separator {
    //     Complexity { value 0.3 }
    //     USE mySphere
    //   }
    // }
    //
    // Such scene can be converted easily to OSG.
    if (node->isOfType(SoLOD::getClassTypeId())) {

        thisPtr->ivPushState(action, node, IvStateItem::KEEP_CHILDREN_ORDER,
                             new osg::LOD);
        thisPtr->ivStateStack.top().keepChildrenOrderParent = node;

        return SoCallbackAction::CONTINUE;
    }

    return SoCallbackAction::CONTINUE;
}
//////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::postLOD(void* data, SoCallbackAction* action,
                             const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "postLOD()  "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    // SoGroup -> do nothing
    if (node->getTypeId() == SoGroup::getClassTypeId())
        return SoCallbackAction::CONTINUE;

    // init values
    ConvertFromInventor* thisPtr = (ConvertFromInventor*)data;
    IvStateItem &ivState = thisPtr->ivStateStack.top();

    // SoLOD
    if (node->isOfType(SoLOD::getClassTypeId())) {

        osg::LOD *lod = dynamic_cast<osg::LOD*>(ivState.osgStateRoot.get());
        SoLOD *ivLOD = (SoLOD*)node;

        // LOD center
        SbVec3f ivCenter = ivLOD->center.getValue();
        lod->setCenter(osg::Vec3(ivCenter[0], ivCenter[1], ivCenter[2]));

        // Verify the number of children and range values
        int num = lod->getNumChildren();
        if (ivLOD->range.getNum()+1 != num &&
            !(num == 0 && ivLOD->range.getNum() == 0)) {
            OSG_WARN << NOTIFY_HEADER <<
                      "Warning: SoLOD does not contain "
                      "correct data in range field." << std::endl;
            if (ivLOD->range.getNum()+1 < num) {
                lod->removeChildren(ivLOD->range.getNum() + 1,
                                    num - ivLOD->range.getNum() - 1);
                num = ivLOD->range.getNum() + 1;
            }
        }

        // Get the ranges and set it
        if (num > 0) {
            if (num == 1)
                lod->setRange(0, 0.0, FLT_MAX);
            else {
                lod->setRange(0, 0.0, ivLOD->range[0]);
                for (int i = 1; i < num-1; i++)
                    lod->setRange(i, ivLOD->range[i-1], ivLOD->range[i]);
                lod->setRange(num-1, ivLOD->range[num-2], FLT_MAX);
            }
        }

#ifdef DEBUG_IV_PLUGIN
        OSG_DEBUG << NOTIFY_HEADER <<
                  "Appending osg::LOD with " << num << " children." << std::endl;
#endif

        assert(ivState.keepChildrenOrderParent == node &&
               "Current node is not the root of keepChildrenOrder graph.");
        thisPtr->ivPopState(action, node);

        return SoCallbackAction::CONTINUE;
    }

    return SoCallbackAction::CONTINUE;
}

// g++ (at least) guarantees thread-safe method-local static initialization, so moving construction of these maps to exploit
class NormBindingMap : public std::map<SoNormalBinding::Binding, deprecated_osg::Geometry::AttributeBinding>
{
  public:
    NormBindingMap()
    {
        (*this)[SoNormalBinding::OVERALL]            = deprecated_osg::Geometry::BIND_OVERALL;
        (*this)[SoNormalBinding::PER_PART]           = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoNormalBinding::PER_PART_INDEXED]   = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoNormalBinding::PER_FACE]           = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoNormalBinding::PER_FACE_INDEXED]   = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoNormalBinding::PER_VERTEX]         = deprecated_osg::Geometry::BIND_PER_VERTEX;
        (*this)[SoNormalBinding::PER_VERTEX_INDEXED] = deprecated_osg::Geometry::BIND_PER_VERTEX;
    }
};

class ColBindingMap : public std::map<SoMaterialBinding::Binding, deprecated_osg::Geometry::AttributeBinding>
{
  public:
    ColBindingMap()
    {
        (*this)[SoMaterialBinding::OVERALL]            = deprecated_osg::Geometry::BIND_OVERALL;
        (*this)[SoMaterialBinding::PER_PART]           = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoMaterialBinding::PER_PART_INDEXED]   = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoMaterialBinding::PER_FACE]           = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoMaterialBinding::PER_FACE_INDEXED]   = deprecated_osg::Geometry::BIND_PER_PRIMITIVE;
        (*this)[SoMaterialBinding::PER_VERTEX]         = deprecated_osg::Geometry::BIND_PER_VERTEX;
        (*this)[SoMaterialBinding::PER_VERTEX_INDEXED] = deprecated_osg::Geometry::BIND_PER_VERTEX;
    }
};

///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preShape(void* data, SoCallbackAction* action,
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preShape()   "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Normal and color binding map from Inventor to OSG
    static NormBindingMap normBindingMap;
    static ColBindingMap  colBindingMap;

    // Get normal and color binding
    if (node->isOfType(SoVertexShape::getClassTypeId()))
    {
        thisPtr->normalBinding = normBindingMap[action->getNormalBinding()];
        thisPtr->colorBinding = colBindingMap[action->getMaterialBinding()];
    }
    else
    {
        thisPtr->normalBinding = deprecated_osg::Geometry::BIND_PER_VERTEX;
        thisPtr->colorBinding = deprecated_osg::Geometry::BIND_PER_VERTEX;
    }

    // Check vertex ordering
    if (action->getVertexOrdering() == SoShapeHints::CLOCKWISE)
        thisPtr->vertexOrder = CLOCKWISE;
    else
        thisPtr->vertexOrder = COUNTER_CLOCKWISE;

    // Clear the data from the previous shape callback
    thisPtr->numPrimitives = 0;
    thisPtr->vertices.clear();
    thisPtr->normals.clear();
    thisPtr->colors.clear();
    thisPtr->textureCoords.clear();

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////
// OSG doesn't seem to have a transpose function         //
//for matrices                                           //
///////////////////////////////////////////////////////////
void
ConvertFromInventor::transposeMatrix(osg::Matrix& mat)
{
    float tmp;
    for (int j = 0; j < 4; j++)
    {
        for (int i = j + 1; i < 4; i++)
        {
            tmp = mat.operator()(j,i);
            mat.operator()(j,i) = mat.operator()(i,j);
            mat.operator()(i,j) = tmp;
        }
    }

}
////////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::postShape(void* data, SoCallbackAction* action,
                               const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "postShape()  "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);


    // Create a new Geometry
    osg::ref_ptr<deprecated_osg::Geometry> geometry = new deprecated_osg::Geometry;


    osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array(thisPtr->vertices.size());
    for (unsigned int i = 0; i < thisPtr->vertices.size(); i++)
        (*coords)[i] = thisPtr->vertices[i];
    geometry->setVertexArray(coords.get());

    osg::ref_ptr<osg::Vec3Array> norms = NULL;
    if (thisPtr->normalBinding == deprecated_osg::Geometry::BIND_OVERALL)
    {
        norms = new osg::Vec3Array(1);
        const SbVec3f &norm = action->getNormal(0);
        (*norms)[0].set(norm[0], norm[1], norm[2]);
    }
    else
    {
        norms = new osg::Vec3Array(thisPtr->normals.size());
        for (unsigned int i = 0; i < thisPtr->normals.size(); i++)
        {
            (*norms)[i] = thisPtr->normals[i];
        }
    }
    geometry->setNormalArray(norms.get());
    geometry->setNormalBinding(thisPtr->normalBinding);

    // Set the colors
    osg::ref_ptr<osg::Vec4Array> cols;
    if (thisPtr->colorBinding == deprecated_osg::Geometry::BIND_OVERALL)
    {
        cols = new osg::Vec4Array(1);
        SbColor ambient, diffuse, specular, emission;
        float transparency, shininess;
        action->getMaterial(ambient, diffuse, specular, emission, shininess,
                            transparency, 0);
        (*cols)[0].set(diffuse[0], diffuse[1], diffuse[2], 1.0 - transparency);
    }
    else
    {
        cols = new osg::Vec4Array(thisPtr->colors.size());
        for (unsigned int i = 0; i < thisPtr->colors.size(); i++)
            (*cols)[i] = thisPtr->colors[i];
    }
    geometry->setColorArray(cols.get());
    geometry->setColorBinding(thisPtr->colorBinding);


    if (thisPtr->textureCoords.empty())
    {
        OSG_DEBUG<<"tex coords not found"<<std::endl;
    }
    else
    {

        // report texture coordinate conditions
        if (action->getNumTextureCoordinates()>0)
        {
            OSG_DEBUG<<"tex coords found"<<std::endl;
        }
        else
        {
           OSG_DEBUG<<"tex coords generated"<<std::endl;
        }

        // Get the texture transformation matrix
        osg::Matrix textureMat;
        textureMat.set((float *) action->getTextureMatrix().getValue());

        // Transform texture coordinates if texture matrix is not an identity mat
        osg::Matrix identityMat;
        identityMat.makeIdentity();
        osg::ref_ptr<osg::Vec2Array> texCoords
            = new osg::Vec2Array(thisPtr->textureCoords.size());
        if (textureMat == identityMat)
        {
            // Set the texture coordinates
            for (unsigned int i = 0; i < thisPtr->textureCoords.size(); i++)
                (*texCoords)[i] = thisPtr->textureCoords[i];
        }
        else
        {
            // Transform and set the texture coordinates
            for (unsigned int i = 0; i < thisPtr->textureCoords.size(); i++)
            {
                osg::Vec3 transVec = textureMat.preMult(
                        osg::Vec3(thisPtr->textureCoords[i][0],
                                  thisPtr->textureCoords[i][1],
                                  0.0));
                (*texCoords)[i].set(transVec.x(), transVec.y());
            }
        }

        geometry->setTexCoordArray(0, texCoords.get());
    }

    // Set the parameters for the geometry

    geometry->addPrimitiveSet(new osg::DrawArrays(thisPtr->primitiveType,0,
                                                  coords->size()));
    // Get the StateSet for the geoset
    osg::ref_ptr<osg::StateSet> stateSet = thisPtr->getStateSet(action);
    geometry->setStateSet(stateSet.get());

    // Add the geoset to a geode
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geometry.get());

    // Set object names
    const char* name = node->getName().getString();
    geometry->setName(name);
    geode->setName(strlen(name)>0 ? name : stateSet->getName());

    // Transformation and scene graph building
    thisPtr->appendNode(geode.get(), action);

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////

#ifdef __COIN__

//
//  Following classes can be used for redirecting texture loading from Coin routines
//  that use simage library to native OSG routines. This removes the dependency
//  on simage and using the same routines by OSG and Coin may provide some
//  advantages to programmer as well.
//
//  Classes that are loading textures: SoTexture2, SoTexture3 (since Coin 2.0),
//  SoVRMLImageTexture (since Coin 2.0), SoVRMLMovieTexture (not yet implemented in Coin)
//
//  Principle: SoType::overrideType() can be used for changing
//  of the class instantiation method. So let's change it and create our own
//  API compatible class that will just not load texture from file.
//

#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTexture3.h>
#include <Inventor/VRMLnodes/SoVRMLImageTexture.h>


// This macro gives the common API for all overriding classes.
#define OVERRIDE_HEADER(_class_,_parent_) \
public: \
\
    static void overrideClass() \
    { \
        if (overrideCounter == 0) { \
            SoType t = _parent_::getClassTypeId(); \
            oldMethod = t.getInstantiationMethod(); \
            SoType::overrideType(t, _class_::createInstance); \
        } \
        overrideCounter++; \
    } \
\
    static SbBool cancelOverrideClass() \
    { \
        assert(overrideCounter > 0 && #_class_ "::cancelOverride called more times\n" \
               "than " #_class_ "::override"); \
        assert(_parent_::getClassTypeId().getInstantiationMethod() == _class_::createInstance && \
               "Somebody changed " #_parent_ " instantiation method\n" \
               "(probably through SoType::overrideType) and did not restored it."); \
\
        overrideCounter--; \
        if (overrideCounter == 0) \
            SoType::overrideType(_parent_::getClassTypeId(), oldMethod); \
\
        return overrideCounter==0; \
    } \
\
private: \
\
    static int overrideCounter; \
\
    static SoType::instantiationMethod oldMethod; \
\
    static void* createInstance() \
    { \
        return new _class_; \
    }

#define OVERRIDE_SRC(_class_) \
\
int _class_::overrideCounter = 0; \
SoType::instantiationMethod _class_::oldMethod


class SoTexture2Osg : public SoTexture2 {
    OVERRIDE_HEADER(SoTexture2Osg, SoTexture2);
protected:
    virtual SbBool readInstance(SoInput *in, unsigned short flags);
};


class SoTexture3Osg : public SoTexture3 {
    OVERRIDE_HEADER(SoTexture3Osg, SoTexture3);
protected:
    virtual SbBool readInstance(SoInput *in, unsigned short flags);
};


class SoVRMLImageTextureOsg : public SoVRMLImageTexture {
    OVERRIDE_HEADER(SoVRMLImageTextureOsg, SoVRMLImageTexture);
protected:
    virtual SbBool readInstance(SoInput *in, unsigned short flags);
};

OVERRIDE_SRC(SoTexture2Osg);
OVERRIDE_SRC(SoTexture3Osg);
OVERRIDE_SRC(SoVRMLImageTextureOsg);

static osgDB::ReaderWriter::Options* createOptions()
{
    const SbStringList &soInputDirectories = SoInput::getDirectories();
    osgDB::ReaderWriter::Options *options = new osgDB::ReaderWriter::Options;
    osgDB::FilePathList &pathList = options->getDatabasePathList();
    int c = soInputDirectories.getLength();
    for (int i=0; i<c; i++)
        pathList.push_back(soInputDirectories[i]->getString());

    return options;
}

static osg::Image* loadImage(const char *fileName, osgDB::ReaderWriter::Options *options)
{
    osg::ref_ptr<osg::Image> osgImage = osgDB::readImageFile(fileName, options);

    if (!osgImage)
    {
        OSG_WARN << NOTIFY_HEADER << "Could not read texture file '" << fileName << "'.";
        return 0;
    }

    if (!osgImage->isDataContiguous())
    {
        OSG_WARN << NOTIFY_HEADER << "Inventor cannot handle non contiguous image data found in texture file '" << fileName << "'.";
        return 0;
    }

    return osgImage.release();
}

SbBool SoTexture2Osg::readInstance(SoInput *in, unsigned short flags)
{
    // disable notification and read inherited fields
    SbBool oldNotify = filename.enableNotify(FALSE);
    SbBool readOK = SoNode::readInstance(in, flags);
    this->setReadStatus((int) readOK);

    // if file name given
    if (readOK && !filename.isDefault() && filename.getValue() != "")
    {
        // create options and read the file
        osgDB::ReaderWriter::Options *options = createOptions();
        osg::ref_ptr<osg::Image> image = loadImage(filename.getValue().getString(), options);

        if (image.valid())
        {
            // get image dimensions and data
            int nc = osg::Image::computeNumComponents(image->getPixelFormat());
            SbVec2s size(image->s(), image->t());
            unsigned char *bytes = image->data();

            // disable notification on image while setting data from filename
            // as a notify will cause a filename.setDefault(TRUE)
            SbBool oldnotify = this->image.enableNotify(FALSE);
            this->image.setValue(size, nc, bytes);
            this->image.enableNotify(oldnotify);
            // PRIVATE(this)->glimagevalid = FALSE; -> recreate GL image in next GLRender()
            // We can safely ignore this as we are not going to render the scene graph.
        }
        else
        {
            // image loading failed -> set readOK
            readOK = FALSE;
            this->setReadStatus(FALSE);
        }

        // write filename, not image
        this->image.setDefault(TRUE);
    }

    filename.enableNotify(oldNotify);
    return readOK;
}

SbBool SoTexture3Osg::readInstance(SoInput *in, unsigned short flags)
{
    // disable notification and read inherited fields
    SbBool oldNotify = filenames.enableNotify(FALSE);
    SbBool readOK = SoNode::readInstance(in, flags);
    this->setReadStatus((int) readOK);

    // if file name given
    int numImages = filenames.getNum();
    if (readOK && !filenames.isDefault() && numImages > 0)
    {
        // Fail on empty filenames
        SbBool sizeError = FALSE;
        SbBool retval = FALSE;
        SbVec3s volumeSize(0,0,0);
        int volumenc = -1;
        int i;
        for (i=0; i<numImages; i++)
            if (this->filenames[i].getLength()==0) break;

        if (i==numImages)
        {
            // create options
            osgDB::ReaderWriter::Options *options = createOptions();

            for (int n=0; n<numImages && !sizeError; n++)
            {
                // read the file
                osg::ref_ptr<osg::Image> image = loadImage(filenames[n].getString(), options);

                if (!image.valid())
                {
                    OSG_WARN << NOTIFY_HEADER << "Could not read texture file #" << n << ": "
                             << filenames[n].getString() << "\n";
                    retval = FALSE;
                }
                else
                {
                    // get image dimensions and data
                    int nc = osg::Image::computeNumComponents(image->getPixelFormat());
                    SbVec3s size(image->s(), image->t(), image->r());
                    if (size[2]==0)
                        size[2]=1;
                    unsigned char *imgbytes = image->data();

                    if (this->images.isDefault()) { // First time => allocate memory
                        volumeSize.setValue(size[0],
                                            size[1],
                                            size[2]*numImages);
                        volumenc = nc;
                        this->images.setValue(volumeSize, nc, NULL);
                    }
                    else { // Verify size & components
                        if (size[0] != volumeSize[0] ||
                            size[1] != volumeSize[1] ||
                            //FIXME: always 1 or what? (kintel 20020110)
                            size[2] != (volumeSize[2]/numImages) ||
                            nc != volumenc)
                        {
                            sizeError = TRUE;
                            retval = FALSE;

                            OSG_WARN << NOTIFY_HEADER << "Texture file #" << n << " ("
                                     << filenames[n].getString() << ") has wrong size: "
                                     << "Expected (" << volumeSize[0] << "," << volumeSize[1] << ","
                                     << volumeSize[2] << "," << volumenc << ") got ("
                                     << size[0] << "," << size[1] << "," << size[2] << "," << nc << ")\n";
                        }
                    }

                    if (!sizeError)
                    {
                        // disable notification on images while setting data from the
                        // filenames as a notify will cause a filenames.setDefault(TRUE).
                        SbBool oldnotify = this->images.enableNotify(FALSE);
                        unsigned char *volbytes = this->images.startEditing(volumeSize,
                                                                            volumenc);
                        memcpy(volbytes+int(size[0])*int(size[1])*int(size[2])*nc*n,
                               imgbytes, int(size[0])*int(size[1])*int(size[2])*nc);
                        this->images.finishEditing();
                        this->images.enableNotify(oldnotify);
                        // PRIVATE(this)->glimagevalid = FALSE; -> recreate GL image in next GLRender()
                        // We can safely ignore this as we are not going to render the scene graph.
                        retval = TRUE;
                    }
                }
            }
        }

        if (!retval)
        {
            // if image loading failed, set read status,
            // but not set readOK to false (according to Coin source code)
            this->setReadStatus(FALSE);
        }

        // write filename, not image
        this->images.setDefault(TRUE);
    }

    filenames.enableNotify(oldNotify);
    return readOK;
}

SbBool SoVRMLImageTextureOsg::readInstance(SoInput *in, unsigned short flags)
{
    // disable notification and read inherited fields
    SbBool oldNotify = url.enableNotify(FALSE);
    SbBool readOK = SoNode::readInstance(in, flags);
    this->setReadStatus((int) readOK);

    if (readOK) {

        // create options and read the file
        osgDB::ReaderWriter::Options *options = createOptions();

        if (url.getNum() && url[0].getLength())
        {
            osg::ref_ptr<osg::Image> image = loadImage(url[0].getString(), options);
            if (!image->valid())
            {
                OSG_WARN << "Could not read texture file: " << url[0].getString() << std::endl;
                this->setReadStatus(FALSE);
            }
            else
            {
                // get image dimensions and data
                int nc = osg::Image::computeNumComponents(image->getPixelFormat());
                SbVec2s size(image->s(), image->t());
                unsigned char *bytes = image->data();

                SbImage ivImage(bytes, size, nc);

                // disable notification on image while setting data from filename
                // as a notify will cause a filename.setDefault(TRUE)
                //SbBool oldnotify = this->image.enableNotify(FALSE); <- difficult to implement for SoVRMLImageTexture
                this->setImage(ivImage);
                //this->image.enableNotify(oldnotify); <- difficult to implement for SoVRMLImageTexture
                // PRIVATE(this)->glimagevalid = FALSE; -> recreate GL image in next GLRender()
                // We can safely ignore this as we are not going to render the scene graph.
            }
        }
    }

    url.enableNotify(oldNotify);
    return readOK;
}

#endif /* __COIN__ */

///////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::postTexture(void* data, SoCallbackAction *,
                                 const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "postTexture()  "
              << node->getTypeId().getName().getString();
    if (node->isOfType(SoTexture2::getClassTypeId())) {
        SoTexture2 *t = (SoTexture2*)node;
        if (t->filename.getValue().getLength())
            OSG_DEBUG << "  "  << t->filename.getValue().getString();
    }
    OSG_DEBUG << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);
    bool texturingEnabled = false;

    // Texture2
    if (node->isOfType(SoTexture2::getClassTypeId())) {

        // Check whether texturing was enabled by the texture node
        SoTexture2 *t = (SoTexture2*)node;
        SbVec2s size;
        int nc;
        const unsigned char *data = t->image.getValue(size, nc);
        texturingEnabled = t->filename.getValue().getLength() ||
                           (data && size != SbVec2s(0,0));
    }

#ifdef __COIN__

    // SoVRMLImageTexture
    if (node->isOfType(SoVRMLImageTexture::getClassTypeId())) {

        // Check whether texturing was enabled by the texture node
        SoVRMLImageTexture *t = (SoVRMLImageTexture*)node;
        texturingEnabled = t->url.getNum() > 1 || (t->url.getNum() == 1 && t->url[0].getLength() > 0);
    }

    // SoVRMLAppearance
    if (node->isOfType(SoVRMLAppearance::getClassTypeId())) {

        // If SoVRMLAppearance is present and there is no texture
        // inside, disable texturing
        // FIXME: should SoVRMLAppearance really disable texturing
        // when not containing SoVRMLImageTexture? Coin is not doing that,
        // but it can be Coin bug.
        SoVRMLAppearance *a = (SoVRMLAppearance*)node;
        if (a->texture.getValue() == NULL)
            thisPtr->ivStateStack.top().currentTexture = NULL;

        // Do not try to "optimize" this code by removing the return
    // and use the one at the end of the function.
    // It would break the case when there is texture inside
    // the appearance node.
        return SoCallbackAction::CONTINUE;
    }

#endif /* __COIN__ */

    // Set current texture
    if (texturingEnabled)
        thisPtr->ivStateStack.top().currentTexture = node;
    else
        thisPtr->ivStateStack.top().currentTexture = NULL;

    return SoCallbackAction::CONTINUE;
}
//////////////////////////////////////////////////////////////////
void ConvertFromInventor::transformLight(SoCallbackAction* action,
                                         const SbVec3f& vec,
                                         osg::Vec3& transVec)
{
    osg::Matrix modelMat;
    modelMat.set((float *)action->getModelMatrix().getValue());

    transVec.set(vec[0], vec[1], vec[2]);
    transVec = modelMat.preMult(transVec);
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preLight(void* data, SoCallbackAction* action,
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preLight()   "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Return if the light is not on
    const SoLight* ivLight = (const SoLight*) node;
    if (!ivLight->on.getValue())
        return SoCallbackAction::CONTINUE;

    // Create new OSG light
    IvStateItem &ivState = thisPtr->ivStateStack.top();
    osg::ref_ptr<osg::Light> osgLight = new osg::Light;

    // Get color and intensity
    SbVec3f lightColor = ivLight->color.getValue();
    float intensity = ivLight->intensity.getValue();

    // Set color and intensity
    osgLight->setAmbient(osg::Vec4(0.f, 0.f, 0.f, 1.f));
    osgLight->setDiffuse(osg::Vec4(lightColor[0] * intensity,
                                   lightColor[1] * intensity,
                                   lightColor[2] * intensity, 1));
    osgLight->setSpecular(osg::Vec4(lightColor[0] * intensity,
                                    lightColor[1] * intensity,
                                    lightColor[2] * intensity, 1));

    // Light type
    if (node->isOfType(SoDirectionalLight::getClassTypeId()))
    {
        SoDirectionalLight *dirLight = (SoDirectionalLight *) node;

#if 1 // Let's place the light to its place in scene graph instead of
      // old approach of global light group.
        SbVec3f l(dirLight->direction.getValue());
        osgLight->setPosition(osg::Vec4(-l[0], -l[1], -l[2] , 0.));
#else
        osg::Vec3 transVec;
        thisPtr->transformLight(action, dirLight->direction.getValue(), transVec);
        osgLight->setPosition(osg::Vec4(transVec.x(), transVec.y(),
                                        transVec.z(), 0.));
#endif
    }
    else if (node->isOfType(SoPointLight::getClassTypeId()))
    {
        SoPointLight* ptLight = (SoPointLight *) node;

#if 1 // Let's place the light to its place in scene graph instead of
      // old approach of global light group.
        SbVec3f l(ptLight->location.getValue());
        osgLight->setPosition(osg::Vec4(l[0], l[1], l[2] , 1.));
#else
        osg::Vec3 transVec;
        thisPtr->transformLight(action, ptLight->location.getValue(), transVec);
        osgLight->setPosition(osg::Vec4(transVec.x(), transVec.y(),
                                        transVec.z(), 1.));
#endif
    }
    else if (node->isOfType(SoSpotLight::getClassTypeId()))
    {
        SoSpotLight* spotLight = (SoSpotLight *) node;

        osgLight->setSpotExponent(spotLight->dropOffRate.getValue() * 128.0);
        osgLight->setSpotCutoff(spotLight->cutOffAngle.getValue()*180.0/osg::PI);

#if 1 // Let's place the light to its place in scene graph instead of
      // old approach of global light group.
        SbVec3f l(spotLight->location.getValue());
        osgLight->setPosition(osg::Vec4(l[0], l[1], l[2] , 1.));
        l = spotLight->direction.getValue();
        osgLight->setDirection(osg::Vec3(l[0], l[1], l[2]));
#else
        osg::Vec3 transVec;
        thisPtr->transformLight(action, spotLight->location.getValue(), transVec);
        osgLight->setPosition(osg::Vec4(transVec.x(), transVec.y(),
                                        transVec.z(), 1.));

        thisPtr->transformLight(action, spotLight->direction.getValue(),transVec);
        osgLight->setDirection(osg::Vec3(transVec.x(), transVec.y(),
                                         transVec.z()));
#endif
    }

    // Attenuation
    if (!node->isOfType(SoDirectionalLight::getClassTypeId())) {
        SbVec3f att = action->getLightAttenuation();
        osgLight->setConstantAttenuation(att[2]);
        osgLight->setLinearAttenuation(att[1]);
        osgLight->setQuadraticAttenuation(att[0]);
    } else {
        // keep default light settings for directional light, e.g.
        // no attenuation
    }

    // Append the light into the scene and onto the state stack
    osgLight->setLightNum(ivState.currentLights.size());
    ivState.currentLights.push_back(osgLight);

    // Create LightSource
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource();
    ls->setLight(osgLight.get());

    // Set object names
    const char* name = ivLight->getName().getString();
    osgLight->setName(name);
    //ls->setName(name); -> this will be handled bellow in ivPushState

#if 1 // Let's place the light to its place in scene graph instead of
      // old approach of global light group.
    thisPtr->ivPushState(action, node,
              IvStateItem::MULTI_POP | IvStateItem::UPDATE_STATE |
              IvStateItem::APPEND_AT_PUSH, ls.get());
#else
    if (!(thisPtr->lightGroup.get()))
        thisPtr->lightGroup = new osg::Group();
    thisPtr->lightGroup->addChild(ls);
#endif

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preEnvironment(void* data, SoCallbackAction* action,
                                    const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preEnvironment()   "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);
    IvStateItem &ivState = thisPtr->ivStateStack.top();

    ivState.currentAmbientLight = ((SoEnvironment*)node)->ambientColor.getValue() *
                                  ((SoEnvironment*)node)->ambientIntensity.getValue();

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////////
#ifdef INVENTOR_SHADERS_AVAILABLE
static bool
convertShader(osg::Shader::Type osgShaderType,
              const SoShaderObject *ivShader,
              osg::Program *osgProgram)
{
    // NULL shader is not converted while returning success
    if (ivShader == NULL)
        return true;

    // Create shader
    osg::ref_ptr<osg::Shader> osgShader = new osg::Shader(osgShaderType);
    if (ivShader->sourceType.getValue() == SoShaderObject::FILENAME)
        osgShader->loadShaderSourceFromFile(ivShader->sourceProgram.getValue().getString());
    else
    if (ivShader->sourceType.getValue() == SoShaderObject::GLSL_PROGRAM)
        osgShader->setShaderSource(ivShader->sourceProgram.getValue().getString());
    else {
        OSG_WARN << NOTIFY_HEADER << "Can not convert "
                  << "shader. Unsupported shader language." << std::endl;
        return false;
    }

    // Set shader name
    osgShader->setName(ivShader->getName().getString());

    return osgProgram->addShader(osgShader.get());
}
#endif // INVENTOR_SHADERS_AVAILABLE
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preShaderProgram(void* data, SoCallbackAction* action,
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preShaderProgram()  "
              << node->getTypeId().getName().getString() << std::endl;
#endif

#ifdef INVENTOR_SHADERS_AVAILABLE

    ConvertFromInventor *thisPtr = (ConvertFromInventor*)data;
    IvStateItem &ivState = thisPtr->ivStateStack.top();

    // Get Inventor nodes
    // Note: Shaders are available since Coin 2.5 (including
    // geometry shader)
    const SoShaderProgram *ivProgram = (const SoShaderProgram*)node;
    const SoVertexShader *ivVertexShader = NULL;
    const SoGeometryShader *ivGeometryShader = NULL;
    const SoFragmentShader *ivFragmentShader = NULL;

    for (int i=0, c=ivProgram->shaderObject.getNum(); i<c; i++) {

        const SoShaderObject *shader = (const SoShaderObject*)ivProgram->shaderObject[i];
        if (!shader->isOfType(SoShaderObject::getClassTypeId()))
            continue;
        if (shader->isActive.getValue() == FALSE)
            continue;

        if (shader->isOfType(SoVertexShader::getClassTypeId()))
            ivVertexShader = (const SoVertexShader*)shader;
        if (shader->isOfType(SoGeometryShader::getClassTypeId()))
            ivGeometryShader = (const SoGeometryShader*)shader;
        if (shader->isOfType(SoFragmentShader::getClassTypeId()))
            ivFragmentShader = (const SoFragmentShader*)shader;
    }

    // Create OSG shader program
    osg::Program *osgProgram = new osg::Program();
    if (!convertShader(osg::Shader::VERTEX, ivVertexShader, osgProgram))
        OSG_WARN << NOTIFY_HEADER
                  << "Failed to add vertex shader." << std::endl;
    if (!convertShader(osg::Shader::GEOMETRY, ivGeometryShader, osgProgram))
        OSG_WARN << NOTIFY_HEADER
                  << "Failed to add geometry shader." << std::endl;
    if (!convertShader(osg::Shader::FRAGMENT, ivFragmentShader, osgProgram))
        OSG_WARN << NOTIFY_HEADER
                  << "Failed to add fragment shader." << std::endl;

    // Set program name
    osgProgram->setName(ivProgram->getName().getString());

    // Put shader to the state stack
    ivState.currentGLProgram = osgProgram;

#else

    OSG_WARN << NOTIFY_HEADER << "Warning: The model "
              "contains shaders while your Inventor does not support "
              "them." << std::endl;
#endif

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::StateSet>
ConvertFromInventor::getStateSet(SoCallbackAction* action)
{
    osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet;

    // Inherit modes from the global state
    stateSet->clear();

    // Inventor State Stack
    IvStateItem &ivState = ivStateStack.top();

    // Convert the IV texture to OSG texture if any
    osg::ref_ptr<osg::Texture2D> texture;
    const SoNode *ivTexture = ivState.currentTexture;
    if (ivTexture)
    {
        // Found a corresponding OSG texture object
        if (ivToOsgTexMap[ivTexture])
            texture = ivToOsgTexMap[ivTexture];
        else
        {
            // Create a new osg texture
            texture = convertIVTexToOSGTex(ivTexture, action);

            // Add the new texture to the database
            ivToOsgTexMap[ivTexture] = texture.get();
        }

        stateSet->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);

        // propogate name
        if(texture.valid())
        {
            std::string name = texture->getName();
            if (name != "")
                stateSet->setName(name);
        }
        // Set the texture environment
        osg::ref_ptr<osg::TexEnv> texEnv = new osg::TexEnv;
        switch (action->getTextureModel())
        {
            case SoTexture2::MODULATE:
                texEnv->setMode(osg::TexEnv::MODULATE);
                break;
            case SoTexture2::DECAL:
                texEnv->setMode(osg::TexEnv::DECAL);
                break;
            case SoTexture2::BLEND: {
                texEnv->setMode(osg::TexEnv::BLEND);
                SbColor c(action->getTextureBlendColor());
                texEnv->setColor(osg::Vec4(c[0], c[1], c[2], 1.f));
                break;
            }
            // SGI's Inventor does not have REPLACE mode, but the Coin 3D library does.
            // Coin supports REPLACE since 2.2 release, TGS Inventor from 4.0.
            // Let's convert to the TexEnv anyway.
            case 0x1E01: //SoTexture2::REPLACE:
                texEnv->setMode(osg::TexEnv::REPLACE);
                break;
            default:
                OSG_WARN << "Unsupported TexEnv mode." << std::endl;
                break;

        }
        stateSet->setTextureAttributeAndModes(0,texEnv.get(),osg::StateAttribute::ON);
    }

    SbColor ambient, diffuse, specular, emission;
    float shininess, transparency;

    // Get the material colors
    action->getMaterial(ambient, diffuse, specular, emission,
                shininess, transparency, 0);

    // Set transparency
    SbBool hasTextureTransparency = FALSE;
    if (ivTexture) {
      SbVec2s size(0, 0);
      int bpp = 0;
      const unsigned char *data = NULL;
      if (ivTexture->isOfType(SoTexture2::getClassTypeId()))
        data = ((SoTexture2*)ivTexture)->image.getValue(size, bpp);
#ifdef __COIN__
      else
      if (ivTexture->isOfType(SoVRMLImageTexture::getClassTypeId())) {
        const SbImage *img = ((SoVRMLImageTexture*)ivTexture)->getImage();
        if (img)
          data = img->getValue(size, bpp);
      }
#endif

      // look whether texture really contains transparency
      if ((bpp==4 || bpp==2) && data) {
        data += bpp - 1;
        for (int y=0; y<size[1]; y++)
          for (int x=0; x<size[0]; x++, data += bpp)
            if (*data != 255) {
              hasTextureTransparency = TRUE;
              goto finished;
            }
      finished:;
      }
    }

    if (transparency > 0 || hasTextureTransparency)
    {
        // Blending to SRC_APLHA and ONE_MINUS_SRC_ALPHA
        stateSet->setAttributeAndModes(new osg::BlendFunc);

        // Disable depth writes
        stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0., 1., false));

        // Enable depth sorting for transparent objects
        stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        stateSet->setNestRenderBins(false);
    }

    // Set linewidth
    if (action->getLineWidth())
    {
        osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth;
        lineWidth->setWidth(action->getLineWidth());
        stateSet->setAttributeAndModes(lineWidth.get(), osg::StateAttribute::ON);
    }

    // Set pointsize
    if (action->getPointSize())
    {
        osg::ref_ptr<osg::Point> point = new osg::Point;
        point->setSize(action->getPointSize());
        stateSet->setAttributeAndModes(point.get(), osg::StateAttribute::ON);
    }

    // Set draw mode
    switch (action->getDrawStyle())
    {
        case SoDrawStyle::FILLED:
        {
#if 0
// OSG defaults to filled draw style, so no need to set redundent state.
            osg::PolygonMode *polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
                                 osg::PolygonMode::FILL);
            stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::ON);
#endif
            break;
        }
        case SoDrawStyle::LINES:
        {
            osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
                                 osg::PolygonMode::LINE);
            stateSet->setAttributeAndModes(polygonMode.get(), osg::StateAttribute::ON);
            break;
        }
        case SoDrawStyle::POINTS:
        {
            osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
                                 osg::PolygonMode::POINT);
            stateSet->setAttributeAndModes(polygonMode.get(), osg::StateAttribute::ON);
            break;
        }
        case SoDrawStyle::INVISIBLE:
            // check how to handle this in osg.
            break;
    }

    // Set back face culling
    if (action->getShapeType() == SoShapeHints::SOLID)
    {
        osg::ref_ptr<osg::CullFace> cullFace = new osg::CullFace;
        cullFace->setMode(osg::CullFace::BACK);
        stateSet->setAttributeAndModes(cullFace.get(), osg::StateAttribute::ON);
    }

    // Set lighting
    if (action->getLightModel() == SoLightModel::BASE_COLOR)
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    else
    {
        // Set the material
        osg::ref_ptr<osg::Material> material = new osg::Material;

        material->setAmbient(osg::Material::FRONT_AND_BACK,
                             osg::Vec4(ambient[0], ambient[1], ambient[2],
                                       1.0 - transparency));
        material->setDiffuse(osg::Material::FRONT_AND_BACK,
                             osg::Vec4(diffuse[0], diffuse[1], diffuse[2],
                                       1.0 - transparency));
        material->setSpecular(osg::Material::FRONT_AND_BACK,
                              osg::Vec4(specular[0], specular[1], specular[2],
                                        1.0 - transparency));
        material->setEmission(osg::Material::FRONT_AND_BACK,
                              osg::Vec4(emission[0], emission[1], emission[2],
                                        1.0 - transparency));
        material->setTransparency(osg::Material::FRONT_AND_BACK, transparency);
        if (specular[0] || specular[1] || specular[2])
            material->setShininess(osg::Material::FRONT_AND_BACK,
                                   shininess*128.0);
        else
            material->setShininess(osg::Material::FRONT_AND_BACK, 0.0);

        material->setColorMode(osg::Material::DIFFUSE);

        stateSet->setAttributeAndModes(material.get(), osg::StateAttribute::ON);
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);

        // Set global ambient light
        // note on osg::LightModel default values:
        //   colorControl: SINGLE_COLOR, localViewer: false, twoSided: false
        osg::LightModel *lightModel = new osg::LightModel();
        const SbColor &c = ivState.currentAmbientLight;
        lightModel->setAmbientIntensity(osg::Vec4(c[0], c[1], c[2], 1.0));
#if 0
// disable as two sided lighting causes problem under NVidia, and the above osg::Material settings are single sided anway..
update: The mentioned bug is probably just for very old NVidia drivers (commit time of the comment is 2005-03-18).
        The proper solution should be to set two sided lighting based on SoShapeHints node. Need to be developed. PCJohn-2010-01-20
        // Set two sided lighting
        lightModel->setTwoSided(true);
#endif
        stateSet->setAttributeAndModes(lightModel, osg::StateAttribute::ON);

        // Set lights
        for (unsigned int i = 0; i < ivState.currentLights.size(); i++)
            stateSet->setAttributeAndModes(ivState.currentLights[i].get(),
                                           osg::StateAttribute::ON);

    }

    // Shader program setup
    if (ivState.currentGLProgram.get() != NULL) {
        stateSet->setAttributeAndModes(ivState.currentGLProgram.get(),
                                       osg::StateAttribute::ON);
    }

    // Shader program uniforms
    if (ivState.currentGLProgram.get() != NULL) {
        for (int i=0, c=ivState.currentGLProgram->getNumShaders(); i<c; i++) {
             const std::string &shaderCode = ivState.currentGLProgram->getShader(i)->getShaderSource();
             if (shaderCode.find("coin_texunit0_model") != std::string::npos) {
                 int mode = (ivTexture!=NULL) ? action->getTextureModel() : 0;
                 stateSet->addUniform(new osg::Uniform("coin_texunit0_model", mode));
                 break;
             }
        }
    }

    return stateSet;
}

class TexWrapMap : public std::map<SoTexture2::Wrap, osg::Texture2D::WrapMode>
{
  public:
    TexWrapMap()
    {
        (*this)[SoTexture2::CLAMP] = osg::Texture2D::CLAMP;
        (*this)[SoTexture2::REPEAT] = osg::Texture2D::REPEAT;
    }
};

////////////////////////////////////////////////////////////////////
osg::Texture2D*
ConvertFromInventor::convertIVTexToOSGTex(const SoNode* soNode,
                                          SoCallbackAction* action)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER
              << "convertIVTexToOSGTex ("
              << soNode->getTypeId().getName().getString()
              << ")" << std::endl;
#endif

    SbVec2s soSize;
    int soNC;

    // Get the texture size and components
    const unsigned char* soImageData = action->getTextureImage(soSize, soNC);
    if (!soImageData) {
        OSG_WARN << NOTIFY_HEADER
                  << "Warning: Error while loading texture data." << std::endl;
        return NULL;
    }

    // Allocate memory for image data
    unsigned char* osgImageData = new unsigned char[soSize[0] * soSize[1] * soNC];

    // Copy the texture image data from the inventor texture
    memcpy(osgImageData, soImageData, soSize[0] * soSize[1] * soNC);

    // File name
    std::string fileName;
    if (soNode->isOfType(SoTexture2::getClassTypeId()))
        fileName = ((SoTexture2*)soNode)->filename.getValue().getString();
#ifdef __COIN__
    else
    if (soNode->isOfType(SoVRMLImageTexture::getClassTypeId()))
        fileName = ((SoVRMLImageTexture*)soNode)->url.getNum() >= 1 ?
                   ((SoVRMLImageTexture*)soNode)->url.getValues(0)[0].getString() : "";
#endif
    else
      OSG_WARN << NOTIFY_HEADER
                << " Warning: Unsupported texture type: "
                << soNode->getTypeId().getName().getString() << std::endl;

#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER
              << "  Converting file name: " << fileName << " -> ";
#endif
    if (fileName[0]=='\"') fileName.erase(fileName.begin());
    if (fileName.size() > 0 && fileName[fileName.size()-1]=='\"')
        fileName.erase(fileName.begin()+fileName.size()-1);
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << fileName << std::endl;
#endif

    // Create the osg::Image
    osg::ref_ptr<osg::Image> osgImage = new osg::Image;
    osgImage->setFileName(fileName);
    GLenum formats[] = {GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
    osgImage->setImage(soSize[0], soSize[1], 1, soNC, formats[soNC-1],
                       GL_UNSIGNED_BYTE, osgImageData, osg::Image::USE_NEW_DELETE);

    // Create the osg::Texture2D
    osg::Texture2D *osgTex = new osg::Texture2D;
    osgTex->setImage(osgImage.get());

    // Set name
    osgTex->setName(soNode->getName().getString());

    static TexWrapMap texWrapMap;

    // Set texture wrap mode
#ifdef __COIN__
    if (soNode->isOfType(SoVRMLImageTexture::getClassTypeId())) {
        // It looks like there is a high probability of bug in Coin (investigated on version 2.4.6).
        // action->getTextureWrap() returns correct value on SoTexture2 (SoTexture2::CLAMP = 0x2900,
        // and REPEAT = 0x2901), but SoVRMLImageTexture returns incorrect value of
        // SoGLImage::REPEAT = 0, CLAMP = 1, CLAMP_TO_EDGE = 2).
        // So, let's not use action and try to get correct value directly from texture node.
        // PCJohn-2007-04-22
        osgTex->setWrap(osg::Texture2D::WRAP_S, ((SoVRMLImageTexture*)soNode)->repeatS.getValue() ?
            osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE);
        osgTex->setWrap(osg::Texture2D::WRAP_T, ((SoVRMLImageTexture*)soNode)->repeatT.getValue() ?
            osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE);
    }
    else
#endif
    {
        // Proper way to determine wrap mode
        osgTex->setWrap(osg::Texture2D::WRAP_S, texWrapMap[action->getTextureWrapS()]);
        osgTex->setWrap(osg::Texture2D::WRAP_T, texWrapMap[action->getTextureWrapT()]);
    }

    return osgTex;
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preInfo(void* data, SoCallbackAction* action,
                             const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preInfo()    "
              << node->getTypeId().getName().getString() << std::endl;
#endif

#if 0 // FIXME: Not handled properly yet. There is no Info node in OSG.
      // Append probably empty Node and set its name to info->string.getValue();
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);
    SoInfo* info = (SoInfo*)node;
#endif

    return SoCallbackAction::CONTINUE;
}
/////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preRotor(void *data, SoCallbackAction *action,
                              const SoNode *node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preRotor()  "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Get the parameters for the inventor Rotor
    SoRotor *ivRotor = (SoRotor *) node;
    SbVec3f ivAxis;
    float angle;
    ivRotor->rotation.getValue(ivAxis, angle);

    // Create a new osg::MatrixTransform
    osg::ref_ptr<osg::MatrixTransform> rotorTransform = new osg::MatrixTransform;

    // Create a Rotor Callback equivalent to the inventor Rotor
    osg::Vec3 pivot(0, 0, 0);
    osg::Vec3 axis(ivAxis[0], ivAxis[1], ivAxis[2]);
    osg::ref_ptr<osgUtil::TransformCallback> rotorCallback
        = new osgUtil::TransformCallback(pivot, axis,
                                         2 * osg::PI * ivRotor->speed.getValue());

    // Set the app callback
    rotorTransform->setUpdateCallback(rotorCallback.get());

    // Push the rotor onto the state stack
    thisPtr->ivPushState(action, node,
              IvStateItem::MULTI_POP | IvStateItem::UPDATE_STATE |
              IvStateItem::APPEND_AT_PUSH, rotorTransform.get());

    // Append initial rotation to the model matrix
    if (!ivRotor->rotation.isIgnored()) {
        SoModelMatrixElement::rotateBy(action->getState(), ivRotor,
                                       ivRotor->rotation.getValue());
    }

    // Don't do the traversal of the SoShuttle
    // since it was seen on Coin that is does not append just
    // initial shuttle position, but some interpolated one,
    // resulting in incorrect animation.
    return SoCallbackAction::PRUNE;
}
////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::prePendulum(void* data, SoCallbackAction *action,
                                 const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "prePendulum()  "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Get the parameters for the inventor Pendulum
    SoPendulum *ivPendulum = (SoPendulum *) node;
    SbVec3f ivAxis0, ivAxis1;
    float startAngle, endAngle;
    ivPendulum->rotation0.getValue(ivAxis0, startAngle);
    ivPendulum->rotation1.getValue(ivAxis1, endAngle);
    ivAxis0.normalize();
    ivAxis1.normalize();

    // Reverse axis and direction if required
    // Actually, this will produce correct results only when axis is
    // opposite to each other, and approximate results when nearly
    // opposite and garbage otherwise.
    if ((ivAxis0+ivAxis1).length() < 0.5 ) {
        ivAxis1 = -ivAxis1;
        endAngle = -endAngle;
    }

    // Create a new osg::MatrixTransform
    osg::ref_ptr<osg::MatrixTransform> pendulumTransform = new osg::MatrixTransform;

    // Create a Pendulum Callback equivalent to the inventor Rotor
    // Use axis from of the bigger angle (to avoid lost axis when
    // angle is zero - see SbRotation and quaternion theory).
    osg::Vec3 axis;
    if (fabs(startAngle) > fabs(endAngle))
        axis = osg::Vec3(ivAxis0[0], ivAxis0[1], ivAxis0[2]);
    else
        axis = osg::Vec3(ivAxis1[0], ivAxis1[1], ivAxis1[2]);
    PendulumCallback* pendulumCallback
        = new PendulumCallback(axis, startAngle, endAngle,
                               ivPendulum->speed.getValue());

    // Set the app callback
    pendulumTransform->setUpdateCallback(pendulumCallback);

    // Push the pendulum onto the state stack
    thisPtr->ivPushState(action, node,
              IvStateItem::MULTI_POP | IvStateItem::UPDATE_STATE |
              IvStateItem::APPEND_AT_PUSH, pendulumTransform.get());

    // Don't do the traversal of the SoShuttle
    // since it was seen on Coin that is does not append just
    // initial shuttle position, but some interpolated one,
    // resulting in incorrect animation.
    return SoCallbackAction::PRUNE;
}
////////////////////////////////////////////////////////////////
SoCallbackAction::Response
ConvertFromInventor::preShuttle(void* data, SoCallbackAction *action,
                                const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    OSG_DEBUG << NOTIFY_HEADER << "preShuttle()  "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Get the parameters for the inventor Shuttle
    SoShuttle *ivShuttle = (SoShuttle *) node;
    SbVec3f ivStartPos, ivEndPos;
    ivStartPos = ivShuttle->translation0.getValue();
    ivEndPos = ivShuttle->translation1.getValue();

    // Create a new osg::MatrixTransform
    osg::ref_ptr<osg::MatrixTransform> shuttleTransform = new osg::MatrixTransform;

    // Create a shuttle Callback equivalent to the inventor Rotor
    osg::Vec3 startPos(ivStartPos[0], ivStartPos[1], ivStartPos[2]);
    osg::Vec3 endPos(ivEndPos[0], ivEndPos[1], ivEndPos[2]);
    ShuttleCallback* shuttleCallback
        = new ShuttleCallback(startPos, endPos, ivShuttle->speed.getValue());

    // Set the app callback
    shuttleTransform->setUpdateCallback(shuttleCallback);

    // Push the shuttle onto the state stack
    thisPtr->ivPushState(action, node,
              IvStateItem::MULTI_POP | IvStateItem::UPDATE_STATE |
              IvStateItem::APPEND_AT_PUSH, shuttleTransform.get());

    // Don't do the traversal of the SoShuttle
    // since it was seen on Coin that is does not append just
    // initial shuttle position, but some interpolated one,
    // resulting in incorrect animation.
    return SoCallbackAction::PRUNE;
}
////////////////////////////////////////////////////////////
void ConvertFromInventor::addVertex(SoCallbackAction* action,
                                    const SoPrimitiveVertex *v,
                                    int index)
{
    // Get the coordinates of the vertex
    SbVec3f pt = v->getPoint();
    vertices.push_back(osg::Vec3(pt[0], pt[1], pt[2]));

    // Get the normal of the vertex
    SbVec3f norm = v->getNormal();

    if ((normalBinding == deprecated_osg::Geometry::BIND_PER_VERTEX) ||
        (normalBinding == deprecated_osg::Geometry::BIND_PER_PRIMITIVE && index == 0))
    {
        // What is this? Why to invert normals at CLOCKWISE vertex ordering?
        // PCJohn 2009-12-13
        //if (vertexOrder == CLOCKWISE)
        //    normals.push_back(osg::Vec3(-norm[0], -norm[1], -norm[2]));
        //else
            normals.push_back(osg::Vec3(norm[0], norm[1], norm[2]));
    }

    if (colorBinding == deprecated_osg::Geometry::BIND_PER_VERTEX ||
            colorBinding == deprecated_osg::Geometry::BIND_PER_PRIMITIVE)
    {
        // Get the material/color
        SbColor ambient, diffuse, specular, emission;
        float transparency, shininess;
        action->getMaterial(ambient, diffuse, specular, emission, shininess,
                            transparency, v->getMaterialIndex());
        if (colorBinding == deprecated_osg::Geometry::BIND_PER_VERTEX)
            colors.push_back(osg::Vec4(diffuse[0], diffuse[1], diffuse[2],
                                       1.0 - transparency));
        else if (colorBinding == deprecated_osg::Geometry::BIND_PER_PRIMITIVE && index == 0)
            colors.push_back(osg::Vec4(diffuse[0], diffuse[1], diffuse[2],
                                       1.0 - transparency));
    }

    // Get the texture coordinates
    SbVec4f texCoord = v->getTextureCoords();
    textureCoords.push_back(osg::Vec2(texCoord[0], texCoord[1]));
}
////////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::addTriangleCB(void* data, SoCallbackAction* action,
                                        const SoPrimitiveVertex* v0,
                                        const SoPrimitiveVertex* v1,
                                        const SoPrimitiveVertex* v2)
{
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    switch (thisPtr->vertexOrder)
    {
        case CLOCKWISE:
            thisPtr->addVertex(action, v0, 0);
            thisPtr->addVertex(action, v2, 1);
            thisPtr->addVertex(action, v1, 2);
            break;
        case COUNTER_CLOCKWISE:
            thisPtr->addVertex(action, v0, 0);
            thisPtr->addVertex(action, v1, 1);
            thisPtr->addVertex(action, v2, 2);
            break;
    }

    thisPtr->numPrimitives++;
    thisPtr->primitiveType = osg::PrimitiveSet::TRIANGLES;
}
////////////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::addLineSegmentCB(void* data, SoCallbackAction* action,
                                           const SoPrimitiveVertex* v0,
                                           const SoPrimitiveVertex* v1)
{
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    thisPtr->addVertex(action, v0, 0);
    thisPtr->addVertex(action, v1, 1);

    thisPtr->numPrimitives++;
    thisPtr->primitiveType = osg::PrimitiveSet::LINES;
}
//////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::addPointCB(void* data, SoCallbackAction* action,
                                     const SoPrimitiveVertex* v0)
{
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    thisPtr->addVertex(action, v0, 0);

    thisPtr->numPrimitives++;
    thisPtr->primitiveType = osg::PrimitiveSet::POINTS;
}
//////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::init()
{
#ifdef __COIN__
    SoTexture2Osg::overrideClass();
    SoTexture3Osg::overrideClass();
    SoVRMLImageTextureOsg::overrideClass();
#endif
}
