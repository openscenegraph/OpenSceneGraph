#include <sstream>
#include <memory>

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/Texture2D>
#include <osgDB/ConvertUTF>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Bone>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/Skeleton>
#include <osgAnimation/VertexInfluence>

#if defined(_MSC_VER)
    #pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

#include "ReaderWriterFBX.h"
#include "fbxRNode.h"
#include "fbxMaterialToOsgStateSet.h"
#include "WriterNodeVisitor.h"

/// Returns true if the given node is a basic root group with no special information.
/// Used in conjunction with UseFbxRoot option.
/// Identity transforms are considered as basic root nodes.
bool isBasicRootNode(const osg::Node& node)
{
    const osg::Group* osgGroup = node.asGroup();
    if (!osgGroup)
    {
        // Geodes & such are not basic root nodes
        return false;
    }

    // Test if we've got an empty transform (= a group!)
    const osg::Transform* transform = osgGroup->asTransform();
    if (transform)
    {
        if (const osg::MatrixTransform* matrixTransform = transform->asMatrixTransform())
        {
            if (!matrixTransform->getMatrix().isIdentity())
            {
                // Non-identity matrix transform
                return false;
            }
        }
        else if (const osg::PositionAttitudeTransform* pat = transform->asPositionAttitudeTransform())
        {
            if (pat->getPosition() != osg::Vec3d() ||
                pat->getAttitude() != osg::Quat() ||
                pat->getScale() != osg::Vec3d(1.0f, 1.0f, 1.0f) ||
                pat->getPivotPoint() != osg::Vec3d())
            {
                // Non-identity position attribute transform
                return false;
            }
        }
        else
        {
            // Other transform (not identity or not predefined type)
            return false;
        }
    }

    // Test the presence of a non-empty stateset
    if (node.getStateSet())
    {
        osg::ref_ptr<osg::StateSet> emptyStateSet = new osg::StateSet;
        if (node.getStateSet()->compare(*emptyStateSet, true) != 0)
        {
            return false;
        }
    }

    return true;
}


class CleanUpFbx
{
    KFbxSdkManager* m_pSdkManager;
public:
    explicit CleanUpFbx(KFbxSdkManager* pSdkManager) : m_pSdkManager(pSdkManager)
    {}

    ~CleanUpFbx()
    {
        KFbxIOSettings::IOSettingsRef().FreeIOSettings();
        m_pSdkManager->Destroy();
    }
};

//Some files don't correctly mark their skeleton nodes, so this function infers
//them from the nodes that skin deformers linked to.
void findLinkedFbxSkeletonNodes(KFbxNode* pNode, std::set<const KFbxNode*>& fbxSkeletons)
{
    if (const KFbxGeometry* pMesh = dynamic_cast<const KFbxGeometry*>(pNode->GetNodeAttribute()))
    {
        for (int i = 0; i < pMesh->GetDeformerCount(KFbxDeformer::eSKIN); ++i)
        {
            const KFbxSkin* pSkin = (const KFbxSkin*)pMesh->GetDeformer(i, KFbxDeformer::eSKIN);

            for (int j = 0; j < pSkin->GetClusterCount(); ++j)
            {
                const KFbxNode* pSkeleton = pSkin->GetCluster(j)->GetLink();
                fbxSkeletons.insert(pSkeleton);
            }
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        findLinkedFbxSkeletonNodes(pNode->GetChild(i), fbxSkeletons);
    }
}

void resolveBindMatrices(
    osg::Node& root,
    const BindMatrixMap& boneBindMatrices,
    const std::map<KFbxNode*, osg::Node*>& nodeMap)
{
    std::set<std::string> nodeNames;
    for (std::map<KFbxNode*, osg::Node*>::const_iterator it = nodeMap.begin(); it != nodeMap.end(); ++it)
    {
        nodeNames.insert(it->second->getName());
    }

    for (BindMatrixMap::const_iterator it = boneBindMatrices.begin();
        it != boneBindMatrices.end();)
    {
        KFbxNode* const fbxBone = it->first.first;
        std::map<KFbxNode*, osg::Node*>::const_iterator nodeIt = nodeMap.find(fbxBone);
        if (nodeIt != nodeMap.end())
        {
            const osg::Matrix bindMatrix = it->second;
            osgAnimation::Bone& osgBone = dynamic_cast<osgAnimation::Bone&>(*nodeIt->second);
            osgBone.setInvBindMatrixInSkeletonSpace(bindMatrix);

            ++it;
            for (; it != boneBindMatrices.end() && it->first.first == fbxBone; ++it)
            {
                if (it->second != bindMatrix)
                {
                    std::string name;
                    for (int i = 0;; ++i)
                    {
                        std::stringstream ss;
                        ss << osgBone.getName() << '_' << i;
                        name = ss.str();
                        if (nodeNames.insert(name).second)
                        {
                            break;
                        }
                    }
                    osgAnimation::Bone* newBone = new osgAnimation::Bone(name);
                    newBone->setDefaultUpdateCallback();
                    newBone->setInvBindMatrixInSkeletonSpace(it->second);
                    osgBone.addChild(newBone);

                    osgAnimation::RigGeometry* pRigGeometry = it->first.second;

                    osgAnimation::VertexInfluenceMap* vertexInfluences = pRigGeometry->getInfluenceMap();

                    osgAnimation::VertexInfluenceMap::iterator vimIt = vertexInfluences->find(osgBone.getName());
                    if (vimIt != vertexInfluences->end())
                    {
                        osgAnimation::VertexInfluence vi;
                        vi.swap(vimIt->second);
                        vertexInfluences->erase(vimIt);
                        osgAnimation::VertexInfluence& vi2 = (*vertexInfluences)[name];
                        vi.swap(vi2);
                        vi2.setName(name);
                    }
                    else
                    {
                        osg::notify(osg::WARN) << "No vertex influences found for \"" << osgBone.getName() << "\"" << std::endl;
                    }
                }
            }
        }
        else
        {
            osg::notify(osg::WARN) << "No bone found for \"" << fbxBone->GetName() << "\"" << std::endl;
            ++it;
        }
    }
}

osgDB::ReaderWriter::ReadResult
ReaderWriterFBX::readNode(const std::string& filenameInit,
                          const Options* options) const
{
    try
    {
        std::string ext(osgDB::getLowerCaseFileExtension(filenameInit));
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string filename(osgDB::findDataFile(filenameInit, options));
        if (filename.empty()) return ReadResult::FILE_NOT_FOUND;

        KFbxSdkManager* pSdkManager = KFbxSdkManager::Create();

        if (!pSdkManager)
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }

        CleanUpFbx cleanUpFbx(pSdkManager);

        KFbxScene* pScene = KFbxScene::Create(pSdkManager, "");

        // The FBX SDK interprets the filename as UTF-8
#ifdef OSG_USE_UTF8_FILENAME
        const std::string& utf8filename(filename);
#else
        std::string utf8filename(osgDB::convertStringFromCurrentCodePageToUTF8(filename));
#endif

        int fileFormat;
        if (!pSdkManager->GetIOPluginRegistry()->DetectFileFormat(utf8filename.c_str(), fileFormat))
        {
            return ReadResult::FILE_NOT_HANDLED;
        }
        KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager, "");

        if (!lImporter->Initialize(utf8filename.c_str(), fileFormat))
        {
            return std::string(lImporter->GetLastErrorString());
        }

        if (!lImporter->IsFBX())
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }

        for (int i = 0; i < lImporter->GetTakeCount(); i++)
        {
            KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            lTakeInfo->mSelect = true;
        }

        if (!lImporter->Import(pScene))
        {
            return std::string(lImporter->GetLastErrorString());
        }

        //KFbxAxisSystem::OpenGL.ConvertScene(pScene);        // Doesn't work as expected. Still need to transform vertices.

        if (KFbxNode* pNode = pScene->GetRootNode())
        {
            pScene->SetCurrentTake(pScene->GetCurrentTakeName());

            bool useFbxRoot = false;
            if (options)
            {
                std::istringstream iss(options->getOptionString());
                std::string opt;
                while (iss >> opt)
                {
                    if (opt == "UseFbxRoot")
                    {
                        useFbxRoot = true;
                    }
                }
            }

            osg::ref_ptr<osgAnimation::AnimationManagerBase> pAnimationManager;
            bool bIsBone = false;
            int nLightCount = 0;
            osg::ref_ptr<Options> localOptions = NULL;
            if (options)
                localOptions = new osgDB::ReaderWriter::Options(*options);
            else
                localOptions = new osgDB::ReaderWriter::Options();
            localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_IMAGES);

            std::string filePath = osgDB::getFilePath(filename);
            FbxMaterialToOsgStateSet fbxMaterialToOsgStateSet(filePath, localOptions.get());

            std::set<const KFbxNode*> fbxSkeletons;
            findLinkedFbxSkeletonNodes(pNode, fbxSkeletons);

            std::map<KFbxNode*, osg::Node*> nodeMap;
            BindMatrixMap boneBindMatrices;
            std::map<KFbxNode*, osgAnimation::Skeleton*> skeletonMap;
            ReadResult res = readFbxNode(*pSdkManager, pNode, pAnimationManager,
                bIsBone, nLightCount, fbxMaterialToOsgStateSet, nodeMap,
                boneBindMatrices, fbxSkeletons, skeletonMap, localOptions.get());

            if (res.success())
            {
                fbxMaterialToOsgStateSet.checkInvertTransparency();

                resolveBindMatrices(*res.getNode(), boneBindMatrices, nodeMap);

                osg::Node* osgNode = res.getNode();
                osgNode->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL,osg::StateAttribute::ON);
                osgNode->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

                if (pAnimationManager.valid())
                {
                    if (osgNode->getUpdateCallback())
                    {
                        osg::Group* osgGroup = new osg::Group;
                        osgGroup->addChild(osgNode);
                        osgNode = osgGroup;
                    }

                    //because the animations may be altered after registering
                    pAnimationManager->buildTargetReference();
                    osgNode->setUpdateCallback(pAnimationManager.get());
                }

                KFbxAxisSystem fbxAxis = pScene->GetGlobalSettings().GetAxisSystem();

                if (fbxAxis != KFbxAxisSystem::OpenGL)
                {
                    int upSign;
                    KFbxAxisSystem::eUpVector eUp = fbxAxis.GetUpVector(upSign);
                    bool bLeftHanded = fbxAxis.GetCoorSystem() == KFbxAxisSystem::LeftHanded;
                    float fSign = upSign < 0 ? -1.0f : 1.0f;
                    float zScale = bLeftHanded ? -1.0f : 1.0f;

                    osg::Matrix mat;
                    switch (eUp)
                    {
                    case KFbxAxisSystem::XAxis:
                        mat.set(0,fSign,0,0,-fSign,0,0,0,0,0,zScale,0,0,0,0,1);
                        break;
                    case KFbxAxisSystem::YAxis:
                        mat.set(1,0,0,0,0,fSign,0,0,0,0,fSign*zScale,0,0,0,0,1);
                        break;
                    case KFbxAxisSystem::ZAxis:
                        mat.set(1,0,0,0,0,0,-fSign*zScale,0,0,fSign,0,0,0,0,0,1);
                        break;
                    }

                    osg::Transform* pTransformTemp = osgNode->asTransform();
                    osg::MatrixTransform* pMatrixTransform = pTransformTemp ?
                        pTransformTemp->asMatrixTransform() : NULL;
                    if (pMatrixTransform)
                    {
                        pMatrixTransform->setMatrix(pMatrixTransform->getMatrix() * mat);
                    }
                    else
                    {
                        pMatrixTransform = new osg::MatrixTransform(mat);
                        if (useFbxRoot && isBasicRootNode(*osgNode))
                        {
                            // If root node is a simple group, put all FBX elements under the OSG root
                            osg::Group* osgGroup = osgNode->asGroup();
                            for(unsigned int i = 0; i < osgGroup->getNumChildren(); ++i)
                            {
                                pMatrixTransform->addChild(osgGroup->getChild(i));
                            }
                            pMatrixTransform->setName(osgGroup->getName());
                        }
                        else
                        {
                            pMatrixTransform->addChild(osgNode);
                        }
                    }
                    osgNode = pMatrixTransform;
                }

                osgNode->setName(filenameInit);
                return osgNode;
            }
        }
    }
    catch (...)
    {
        osg::notify(osg::WARN) << "Exception thrown while importing \"" << filenameInit << '\"' << std::endl;
    }

    return ReadResult::ERROR_IN_READING_FILE;
}

osgDB::ReaderWriter::WriteResult ReaderWriterFBX::writeNode(
    const osg::Node& node,
    const std::string& filename,
    const Options* options) const
{
    try
    {
        std::string ext = osgDB::getLowerCaseFileExtension(filename);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        osg::ref_ptr<Options> localOptions = options ?
            static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        localOptions->getDatabasePathList().push_front(osgDB::getFilePath(filename));

        KFbxSdkManager* pSdkManager = KFbxSdkManager::Create();

        if (!pSdkManager)
        {
            return WriteResult::ERROR_IN_WRITING_FILE;
        }

        CleanUpFbx cleanUpFbx(pSdkManager);

        bool useFbxRoot = false;
        if (options)
        {
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                if (opt == "Embedded")
                {
                    IOSREF.SetBoolProp(EXP_FBX_EMBEDDED, true);
                    if (KFbxIOSettings::IOSettingsRef().IsIOSettingsAllocated())
                        KFbxIOSettings::IOSettingsRef().AllocateIOSettings(*pSdkManager);
                }
                else if (opt == "UseFbxRoot")
                {
                    useFbxRoot = true;
                }
            }
        }

        KFbxScene* pScene = KFbxScene::Create(pSdkManager, "");
        pluginfbx::WriterNodeVisitor writerNodeVisitor(pScene, pSdkManager, filename,
            options, osgDB::getFilePath(node.getName().empty() ? filename : node.getName()));
        if (useFbxRoot && isBasicRootNode(node))
        {
            // If root node is a simple group, put all elements under the FBX root
            const osg::Group * osgGroup = node.asGroup();
            for(unsigned int child=0; child<osgGroup->getNumChildren(); ++child) {
                const_cast<osg::Node *>(osgGroup->getChild(child))->accept(writerNodeVisitor);
            }
        }
        else {
            // Normal scene
            const_cast<osg::Node&>(node).accept(writerNodeVisitor);
        }

        KFbxExporter* lExporter = KFbxExporter::Create(pSdkManager, "");
        pScene->GetGlobalSettings().SetAxisSystem(KFbxAxisSystem::eOpenGL);

        // Ensure the directory exists or else the FBX SDK will fail
        if (!osgDB::makeDirectoryForFile(filename)) {
            osg::notify(osg::NOTICE) << "Can't create directory for file '" << filename << "'. FBX SDK may fail creating the file." << std::endl;
        }

        // The FBX SDK interprets the filename as UTF-8
#ifdef OSG_USE_UTF8_FILENAME
        const std::string& utf8filename(filename);
#else
        std::string utf8filename(osgDB::convertStringFromCurrentCodePageToUTF8(filename));
#endif

        if (!lExporter->Initialize(utf8filename.c_str()))
        {
            return std::string(lExporter->GetLastErrorString());
        }
        if (!lExporter->Export(pScene))
        {
            return std::string(lExporter->GetLastErrorString());
        }

        return WriteResult::FILE_SAVED;
    }
    catch (const std::string& s)
    {
        return s;
    }
    catch (const char* s)
    {
        return std::string(s);
    }
    catch (...)
    {
    }

    return WriteResult::ERROR_IN_WRITING_FILE;
}

///////////////////////////////////////////////////////////////////////////
// Add ourself to the Registry to instantiate the reader/writer.

REGISTER_OSGPLUGIN(fbx, ReaderWriterFBX)
