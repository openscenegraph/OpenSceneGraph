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
#include <osg/PositionAttitudeTransform>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/UpdateMatrixTransform>

using namespace osgDAE;

domNode* daeReader::getRootJoint(domNode* joint) const
{
    int depth = 0;
    while (domNode* parent = daeSafeCast<domNode>(joint->getParent()))
    {
        if (isJoint(parent))
        {
            joint = parent;
            ++depth;
        }
        else
        {
            break;
        }
    }
    return joint;
}

domNode* daeReader::findJointNode(daeElement* searchFrom, domInstance_controller* pDomInstanceController) const
{
    domController *pDomController = daeSafeCast<domController>(getElementFromURI(pDomInstanceController->getUrl()));
    domSkin::domJoints* pDomSkinJoints = pDomController->getSkin()->getJoints();

    domInputLocal_Array domInputs = pDomSkinJoints->getInput_array();

    domSource* pDomJointsSource = NULL;
    for (size_t i=0; i < domInputs.getCount(); i++)
    {
        if (!strcmp(domInputs[i]->getSemantic(), COMMON_PROFILE_INPUT_JOINT))
        {
            pDomJointsSource = daeSafeCast<domSource>(getElementFromURI(domInputs[i]->getSource()));
            if (!pDomJointsSource)
            {
                OSG_WARN << "Could not find skin joints source '" << domInputs[i]->getSource().getURI() << "'" <<std::endl;
                return NULL;
            }
        }
    }

    if (domIDREF_array* pDomIDREFs = pDomJointsSource->getIDREF_array())
    {
        if (pDomIDREFs->getCount())
        {
            return daeSafeCast< domNode >(getElementFromIDRef(pDomIDREFs->getValue().get(0)));
        }
    }
    else if (domName_array* pDomNames = pDomJointsSource->getName_array())
    {
        if (pDomNames->getCount())
        {
            daeString target = pDomNames->getValue().get(0);
            daeSIDResolver resolver(searchFrom, target);
            return daeSafeCast<domNode>(resolver.getElement());
        }
    }

    OSG_WARN << "No valid names or IDREFS array in <skin>" <<std::endl;
    return NULL;
}

domNode* daeReader::findSkeletonNode(daeElement* searchFrom, domInstance_controller* pDomInstanceController) const
{
    domNode* pDomNode = findJointNode(searchFrom, pDomInstanceController);

    if (!pDomNode)
    {
        return NULL;
    }

    return getRootJoint(pDomNode);
}

void daeReader::processSkins()
{
    if (_skinInstanceControllers.empty() || _skeletonMap.empty())
    {
        return;
    }

    typedef std::map<domNode* /*Skeleton root*/, domInstance_controllerList> SkelSkinMap;
    SkelSkinMap skelSkinMap;

    //group the skins according to which group of joints they're attached to.
    for (size_t i = 0; i < _skinInstanceControllers.size(); ++i)
    {
        domInstance_controller* pDomInstanceController = _skinInstanceControllers[i];

        const domInstance_controller::domSkeleton_Array& pDomSkeletons =
            pDomInstanceController->getSkeleton_array();

        if (pDomSkeletons.getCount() == 0)
        {
            domNode* skelNode = findSkeletonNode(_skeletonMap.begin()->first, pDomInstanceController);
            if (skelNode)
            {
                skelSkinMap[skelNode].push_back(pDomInstanceController);
            }
        }
        else
        {
            if (daeElement* pDaeElement = pDomSkeletons.get(0)->getValue().getElement())
            {
                if (domNode* skelNode = findSkeletonNode(pDaeElement, pDomInstanceController))
                {
                    skelSkinMap[skelNode].push_back(pDomInstanceController);
                }
            }
        }
    }

    for (SkelSkinMap::iterator it = skelSkinMap.begin(); it != skelSkinMap.end(); ++it)
    {
        processSkeletonSkins(it->first, it->second);
    }
}

void getJointsAndInverseObjectspaceBindMatrices(domInstance_controller* pDomInstanceController,
    domNode* pDomSkeletonNode,
    std::vector<std::pair<domNode*, osg::Matrix> >& jointsAndBindMatrices)
{
    domController* pDomController = daeSafeCast< domController >(getElementFromURI(pDomInstanceController->getUrl()));

    domSkin* pDomSkin = pDomController->getSkin();

    domSkin::domJoints* pDomSkinJoints = pDomSkin->getJoints();
    domInputLocal_Array domInputs = pDomSkinJoints->getInput_array();

    if (domInputs.getCount() > 2)
    {
        OSG_WARN << "Only a single pair of skin joints inputs is supported." << std::endl;
    }

    domSource* pDomJointsSource = NULL;
    domSource* pDomInvBindMatricesSource = NULL;
    for (size_t i=0; i < domInputs.getCount(); i++)
    {
        if (!strcmp(domInputs[i]->getSemantic(), COMMON_PROFILE_INPUT_JOINT))
        {
            pDomJointsSource = daeSafeCast<domSource>(getElementFromURI(domInputs[i]->getSource()));
            if (!pDomJointsSource)
            {
                OSG_WARN << "Could not find skin joints source '" << domInputs[i]->getSource().getURI() << "'" <<std::endl;
                return;
            }
        }
        else if (!strcmp(domInputs[i]->getSemantic(), COMMON_PROFILE_INPUT_INV_BIND_MATRIX))
        {
            pDomInvBindMatricesSource = daeSafeCast<domSource>(getElementFromURI(domInputs[i]->getSource()));
            if (!pDomInvBindMatricesSource)
            {
                OSG_WARN << "Could not find skin inverse bind matrices source '" << domInputs[i]->getSource().getURI() << "'" <<std::endl;
                return;
            }
        }
    }

    domFloat_array* pDomFloatArray = pDomInvBindMatricesSource->getFloat_array();
    domListOfFloats matrices = pDomFloatArray->getValue();

    osg::Matrix parentInverseSkeletonBindMatrix;

    if (domIDREF_array* pDomIDREFs = pDomJointsSource->getIDREF_array())
    {
        // IDREFS refer to an absolute joint and therefore do not allow a different skeleton
        xsIDREFS* pIDREFS = &(pDomIDREFs->getValue());
        for (size_t i=0; i < pIDREFS->getCount(); i++)
        {
            domNode* pDomNode = daeSafeCast< domNode >(getElementFromIDRef(pIDREFS->get(i)));

            if (pDomNode)
            {
                jointsAndBindMatrices.push_back(std::pair<domNode*, osg::Matrix>(pDomNode, osg::Matrix()));
            }
            else
            {
                OSG_WARN << "Failed to locate joint '" << pIDREFS->get(i).getID() << "'" << std::endl;
            }
        }
    }
    else if (domName_array* pDomNames = pDomJointsSource->getName_array())
    {
        // Using a list of names is the preferred way of referring to joints, because
        // this refers to a joint relative to the given skeletons
        domListOfNames* pNames = &(pDomNames->getValue());
        for (size_t i=0; i < pNames->getCount(); i++)
        {
            daeSIDResolver resolver(pDomSkeletonNode, pNames->get(i));
            domNode* pDomNode = daeSafeCast< domNode >(resolver.getElement());

            if (pDomNode)
            {
                jointsAndBindMatrices.push_back(std::pair<domNode*, osg::Matrix>(pDomNode, osg::Matrix()));
            }
            else
            {
                OSG_WARN << "Failed to locate joint '" << pNames->get(i) << "'" << std::endl;
            }
        }
    }
    else
    {
        OSG_WARN << "No valid names or IDREFS array in <skin>" <<std::endl;
    }

    for (size_t i = 0; i < jointsAndBindMatrices.size(); ++i)
    {
        osg::Matrix invMat(
            matrices.get(i*16 + 0), matrices.get(i*16 + 4), matrices.get(i*16 + 8), matrices.get(i*16 + 12),
            matrices.get(i*16 + 1), matrices.get(i*16 + 5), matrices.get(i*16 + 9), matrices.get(i*16 + 13),
            matrices.get(i*16 + 2), matrices.get(i*16 + 6), matrices.get(i*16 + 10), matrices.get(i*16 + 14),
            matrices.get(i*16 + 3), matrices.get(i*16 + 7), matrices.get(i*16 + 11), matrices.get(i*16 + 15));
        jointsAndBindMatrices[i].second = invMat;
    }
}

void daeReader::processSkeletonSkins(domNode* skeletonRoot, const domInstance_controllerList& instanceControllers)
{
    for (size_t i = 0; i < instanceControllers.size(); ++i)
    {
        domInstance_controller* instanceController = instanceControllers[i];

        std::vector<std::pair<domNode*, osg::Matrix> > jointsAndInverseBindMatrices;
        getJointsAndInverseObjectspaceBindMatrices(instanceController, skeletonRoot, jointsAndInverseBindMatrices);

        for (size_t j = 0; j < jointsAndInverseBindMatrices.size(); ++j)
        {
            osgAnimation::Bone* pOsgBone = getOrCreateBone(jointsAndInverseBindMatrices[j].first);
            pOsgBone->setInvBindMatrixInSkeletonSpace(jointsAndInverseBindMatrices[j].second);
        }
    }

    osgAnimation::Skeleton* skeleton = getOrCreateSkeleton(skeletonRoot);

    for (size_t i = 0; i < instanceControllers.size(); ++i)
    {
        domInstance_controller *pDomInstanceController = instanceControllers[i];
        domController *pDomController = daeSafeCast< domController >(getElementFromURI(pDomInstanceController->getUrl()));
        processSkin(pDomController->getSkin(), skeletonRoot, skeleton, pDomInstanceController->getBind_material());
    }
}

osgAnimation::VertexInfluence& getVertexInfluence(
    osgAnimation::VertexInfluenceMap& vim, const std::string& name)
{
    osgAnimation::VertexInfluenceMap::iterator it = vim.lower_bound(name);
    if (it == vim.end() || name != it->first)
    {
        it = vim.insert(it, osgAnimation::VertexInfluenceMap::value_type(
            name, osgAnimation::VertexInfluence()));
        it->second.setName(name);
    }
    return it->second;
}

// <skin source>
// 0..1    <bind_shape_matrix>
// 3..*    <source>
// 1    <joints>
//        2..*    <input semantic source>
//        0..*    <extra>
// 1    <vertex_weights count>
//        2..*    <input semantic source>
//        0..1    <vcount>
//        0..1    <v>
//        0.*        <extra>
// 0..*    <extra>
void daeReader::processSkin(domSkin* pDomSkin, domNode* skeletonRoot, osgAnimation::Skeleton* pOsgSkeleton, domBind_material* pDomBindMaterial)
{
    daeElement* pDaeSkinSource = getElementFromURI( pDomSkin->getSource());

    if (!pDaeSkinSource)
    {
        OSG_WARN << "Failed to locate geometry " << pDomSkin->getSource().getURI() << std::endl;
        return;
    }

    domGeometry* pDomGeometry = daeSafeCast< domGeometry >(pDaeSkinSource);

    if (!pDomGeometry)
    {
        OSG_WARN << "Skin source is of type " << pDaeSkinSource->getTypeName() << " which is not supported." << std::endl;
        return;
    }

    // Base mesh
    const osg::Geode* pOriginalGeode = NULL;
    osg::Geode* pOsgGeode = getOrCreateGeometry(pDomGeometry, pDomBindMaterial, &pOriginalGeode);
    if (!pOsgGeode)
        return;

    domMesh* pDomMesh = pDomGeometry->getMesh();

    osg::Geode* pOsgRigGeode = new osg::Geode;
    pOsgRigGeode->setDataVariance(osg::Object::DYNAMIC);

    typedef std::map<const osg::Geometry*, osgAnimation::RigGeometry*> GeometryRigGeometryMap;
    GeometryRigGeometryMap old2newGeometryMap;

    for (unsigned i = 0; i < pOsgGeode->getNumDrawables(); ++i)
    {
        if (osg::Geometry* pOsgGeometry = dynamic_cast<osg::Geometry*>(pOsgGeode->getDrawable(i)))
        {
            const osg::Geometry* pOriginalGeometry = dynamic_cast<const osg::Geometry*>(pOriginalGeode->getDrawable(i));

            osgAnimation::RigGeometry* pOsgRigGeometry = new osgAnimation::RigGeometry();
            pOsgRigGeometry->setSourceGeometry(pOsgGeometry);
            pOsgRigGeometry->copyFrom(*pOsgGeometry);
            old2newGeometryMap.insert(GeometryRigGeometryMap::value_type(pOriginalGeometry, pOsgRigGeometry));
            pOsgRigGeometry->setDataVariance(osg::Object::DYNAMIC);
            pOsgRigGeometry->setUseDisplayList( false );
            pOsgRigGeode->addDrawable(pOsgRigGeometry);
        }
        else
        {
            pOsgRigGeode->addDrawable(pOsgGeode->getDrawable(i));
        }
    }

    pOsgSkeleton->addChild(pOsgRigGeode);

    // <bind_shape_matrix>
    if (domSkin::domBind_shape_matrix* pDomBindShapeMatrix = pDomSkin->getBind_shape_matrix())
    {
        domFloat4x4 matrix = pDomBindShapeMatrix->getValue();
        osg::Matrix bindMatrix(
            matrix.get(0), matrix.get(4), matrix.get(8), matrix.get(12),
            matrix.get(1), matrix.get(5), matrix.get(9), matrix.get(13),
            matrix.get(2), matrix.get(6), matrix.get(10), matrix.get(14),
            matrix.get(3), matrix.get(7), matrix.get(11), matrix.get(15));

        for (unsigned d = 0; d < pOsgRigGeode->getNumDrawables(); ++d)
        {
            osgAnimation::RigGeometry* pOsgRigGeometry = dynamic_cast<osgAnimation::RigGeometry*>(pOsgRigGeode->getDrawable(d));
            if (!pOsgRigGeometry)
                continue;

            osg::Array * vert = pOsgRigGeometry->getVertexArray();
            osg::Vec3Array * vertf = NULL;
            osg::Vec3dArray* vertd = NULL;
            if (vert->getType() == osg::Array::Vec3ArrayType)
            {
                vertf = static_cast<osg::Vec3Array*>(vert);
                for (size_t i = 0; i < vertf->size(); ++i)
                {
                    (*vertf)[i] = (*vertf)[i] * bindMatrix;
                }
            }
            else if (vert->getType() == osg::Array::Vec3dArrayType)
            {
                vertd = static_cast<osg::Vec3dArray*>(vert);
                for (size_t i = 0; i < vertd->size(); ++i)
                {
                    (*vertd)[i] = (*vertd)[i] * bindMatrix;
                }
            }
            else
            {
                OSG_NOTIFY(osg::WARN) << "Vertices vector type isn't supported." << std::endl;
                continue;
            }

            osg::Array * norm = pOsgRigGeometry->getNormalArray();
            if (norm)
            {
                osg::Vec3Array * normf = NULL;
                osg::Vec3dArray* normd = NULL;
                if (norm->getType() == osg::Array::Vec3ArrayType)
                {
                    normf = static_cast<osg::Vec3Array*>(norm);
                    for (size_t i = 0; i < normf->size(); ++i)
                    {
                        (*normf)[i] = osg::Matrix::transform3x3((*normf)[i], bindMatrix);
                    }
                }
                else if (norm->getType() == osg::Array::Vec3dArrayType)
                {
                    normd = static_cast<osg::Vec3dArray*>(norm);
                    for (size_t i = 0; i < normd->size(); ++i)
                    {
                        (*normd)[i] = osg::Matrix::transform3x3((*normd)[i], bindMatrix);
                    }
                }
                else
                {
                    OSG_NOTIFY(osg::WARN) << "Normals vector type isn't supported." << std::endl;
                    //continue;
                }
            }

        }
    }

    // 1    <vertex_weights count>

    domSkin::domVertex_weights* pDomVertexWeights = pDomSkin->getVertex_weights();
    domInputLocalOffset_Array domInputs = pDomVertexWeights->getInput_array();

    if (domInputs.getCount() > 2)
    {
        OSG_WARN << "Only a single pair of skin vertex weights inputs is supported." << std::endl;
    }

    domSource* pDomJointsSource = NULL;
    domSource* pDomWeightsSource = NULL;
    for (size_t i=0; i < 2; i++)
    {
        if (!strcmp(domInputs[i]->getSemantic(), COMMON_PROFILE_INPUT_JOINT))
        {
            pDomJointsSource = daeSafeCast<domSource>(getElementFromURI(domInputs[i]->getSource()));
            if (!pDomJointsSource)
            {
                OSG_WARN << "Could not find skin joints source '" << domInputs[i]->getSource().getURI() << "'" <<std::endl;
                return;
            }
        }
        else if (!strcmp(domInputs[i]->getSemantic(), COMMON_PROFILE_INPUT_WEIGHT))
        {
            pDomWeightsSource = daeSafeCast<domSource>(getElementFromURI(domInputs[i]->getSource()));
            if (!pDomWeightsSource)
            {
                OSG_WARN << "Could not find skin weights source '" << domInputs[i]->getSource().getURI() << "'" <<std::endl;
                return;
            }
        }
    }

    domFloat_array* pDomFloatArray = pDomWeightsSource->getFloat_array();
    domListOfFloats weights = pDomFloatArray->getValue();

    domSkin::domVertex_weights::domVcount* pDomVcount = pDomVertexWeights->getVcount();
    domListOfUInts influenceCounts = pDomVcount->getValue();

    domSkin::domVertex_weights::domV* pDomV= pDomVertexWeights->getV();
    domListOfInts jointWeightIndices = pDomV->getValue();

    std::vector<std::string> jointNames;

    if (domName_array* pDomNames = pDomJointsSource->getName_array())
    {
        domListOfNames* pNames = &(pDomNames->getValue());

        jointNames.reserve(pNames->getCount());

        for (size_t i = 0; i < pNames->getCount(); ++i)
        {
            const char* szName = pNames->get(i);
            daeSIDResolver resolver(skeletonRoot, szName);
            osgAnimation::Bone* pOsgBone = _jointMap[daeSafeCast<domNode>(resolver.getElement())].get();
            if (pOsgBone)
            {
                jointNames.push_back(pOsgBone->getName());
            }
            else
            {
                jointNames.push_back(szName);
                OSG_WARN << "Cannot find bone " << szName << std::endl;
            }
        }
    }
    else if (domIDREF_array* pDomIDREFs = pDomJointsSource->getIDREF_array())
    {
        xsIDREFS* pIDREFs = &(pDomIDREFs->getValue());

        jointNames.reserve(pIDREFs->getCount());

        for (size_t i = 0; i < pIDREFs->getCount(); ++i)
        {
            osgAnimation::Bone* pOsgBone = _jointMap[daeSafeCast<domNode>(pIDREFs->get(i).getElement())].get();
            if (pOsgBone)
            {
                jointNames.push_back(pOsgBone->getName());
            }
            else
            {
                jointNames.push_back(pIDREFs->get(i).getID());
                OSG_WARN << "Cannot find bone " << pIDREFs->get(i).getID() << std::endl;
            }
        }
    }
    else
    {
        OSG_WARN << "No valid names or IDREFS array in <skin>" <<std::endl;
        return;
    }

    for (size_t i = 0, vIndex = 0; i < influenceCounts.getCount(); i++)
    {
        OldToNewIndexMap::key_type indexID(pDomMesh, i);
        const OldToNewIndexMap::const_iterator start = _oldToNewIndexMap.find(indexID);

        const size_t nInfluences = influenceCounts[i];

        if (start == _oldToNewIndexMap.end())
        {
            vIndex += nInfluences * 2;
            //this vertex isn't used
            continue;
        }

        const OldToNewIndexMap::const_iterator end = _oldToNewIndexMap.upper_bound(indexID);

        for (size_t j = 0; j < nInfluences; ++j)
        {
            if (vIndex + 2 > jointWeightIndices.getCount())
            {
                OSG_WARN << "vIndex is larger than number of v values" <<std::endl;
                break;
            }

            size_t jointIndex = jointWeightIndices[vIndex++];
            size_t weightIndex = jointWeightIndices[vIndex++];

            if (jointIndex >= jointNames.size())
            {
                OSG_WARN << "Joint index is larger the number of joints" <<std::endl;
                break;
            }
            if (weightIndex >= weights.getCount())
            {
                OSG_WARN << "Weight index is larger the number of weights" <<std::endl;
                break;
            }

            float weight = weights[weightIndex];
            if (weight > 0.0f)
            {
                const std::string& name = jointNames[jointIndex];

                for (OldToNewIndexMap::const_iterator it = start; it != end; ++it)
                {
                    osgAnimation::RigGeometry* pRigGeometry = old2newGeometryMap[it->second.first.get()];

                    osgAnimation::VertexInfluenceMap* vim = pRigGeometry->getInfluenceMap();
                    if (!vim)
                    {
                        pRigGeometry->setInfluenceMap(vim = new osgAnimation::VertexInfluenceMap);
                    }

                    getVertexInfluence(*vim, name).push_back(
                        osgAnimation::VertexIndexWeight(it->second.second, weight));
                }
            }
        }
    }
}
