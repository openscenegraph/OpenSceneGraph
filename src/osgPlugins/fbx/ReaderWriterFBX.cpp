#include <sstream>
#include <memory>
#ifndef WIN32
#include <strings.h>//for strncasecmp
#endif

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/Texture2D>
#include <osg/Version>
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
    #pragma warning( default : 4996 )
#endif
#include <fbxsdk.h>

#include "ReaderWriterFBX.h"
#include "fbxReader.h"
#include "WriterNodeVisitor.h"

/// Returns true if the given node is a basic root group with no special information.
/// Used in conjunction with UseFbxRoot option.
/// Identity transforms are considered as basic root nodes.
bool isBasicRootNode(const osg::Node& node)
{
    const osg::Group* osgGroup = node.asGroup();
    if (!osgGroup || node.asGeode())        // WriterNodeVisitor handles Geodes the "old way" (= Derived from Node, not Group as for now). Geodes may be considered "basic root nodes" when WriterNodeVisitor will be adapted.
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
    FbxManager* m_pSdkManager;
public:
    explicit CleanUpFbx(FbxManager* pSdkManager) : m_pSdkManager(pSdkManager)
    {}

    ~CleanUpFbx()
    {
        m_pSdkManager->Destroy();
    }
};

//Some files don't correctly mark their skeleton nodes, so this function infers
//them from the nodes that skin deformers linked to.
void findLinkedFbxSkeletonNodes(FbxNode* pNode, std::set<const FbxNode*>& fbxSkeletons)
{
    if (const FbxGeometry* pMesh = FbxCast<FbxGeometry>(pNode->GetNodeAttribute()))
    {
        for (int i = 0; i < pMesh->GetDeformerCount(FbxDeformer::eSkin); ++i)
        {
            const FbxSkin* pSkin = (const FbxSkin*)pMesh->GetDeformer(i, FbxDeformer::eSkin);

            for (int j = 0; j < pSkin->GetClusterCount(); ++j)
            {
                const FbxNode* pSkeleton = pSkin->GetCluster(j)->GetLink();
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
    const std::map<FbxNode*, osg::Node*>& nodeMap)
{
    std::set<std::string> nodeNames;
    for (std::map<FbxNode*, osg::Node*>::const_iterator it = nodeMap.begin(); it != nodeMap.end(); ++it)
    {
        nodeNames.insert(it->second->getName());
    }

    for (BindMatrixMap::const_iterator it = boneBindMatrices.begin(); it != boneBindMatrices.end(); ++it)
    {
        FbxNode* const fbxBone = it->first;
        std::map<FbxNode*, osg::Node*>::const_iterator nodeIt = nodeMap.find(fbxBone);
        if (nodeIt != nodeMap.end())
        {
            osgAnimation::Bone* originalBone = dynamic_cast<osgAnimation::Bone*>(nodeIt->second);

            // Iterate bind matrices and create new bones if needed
            const BindMatrixGeometryMap& bindMatrixGeom = it->second;
            for ( BindMatrixGeometryMap::const_iterator bindIt = bindMatrixGeom.begin(); bindIt != bindMatrixGeom.end(); ++bindIt)
            {
                // First matrix will use original bone
                if (bindIt == bindMatrixGeom.begin())
                {
                    originalBone->setInvBindMatrixInSkeletonSpace(bindIt->first);
                }
                else
                {
                    // Additional matrices need new bone
                    std::string name;
                    for (int i = 0;; ++i)
                    {
                        std::stringstream ss;
                        ss << originalBone->getName() << '_' << i;
                        name = ss.str();
                        if (nodeNames.insert(name).second)
                        {
                            break;
                        }
                    }
                    osgAnimation::Bone* newBone = new osgAnimation::Bone(name);
                    newBone->setDefaultUpdateCallback();
                    newBone->setInvBindMatrixInSkeletonSpace(bindIt->first);
                    originalBone->addChild(newBone);

                    // Update rig geometry with new bone names
                    for (std::set<osgAnimation::RigGeometry*>::const_iterator rigIt = bindIt->second.begin();
                         rigIt != bindIt->second.end();
                         ++rigIt)
                    {
                        osgAnimation::RigGeometry* pRigGeometry = (*rigIt);

                        osgAnimation::VertexInfluenceMap* vertexInfluences = pRigGeometry->getInfluenceMap();

                        osgAnimation::VertexInfluenceMap::iterator vimIt = vertexInfluences->find(originalBone->getName());
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
                            OSG_WARN << "No vertex influences found for \"" << originalBone->getName() << "\"" << std::endl;
                        }
                    }
                }
            }
        }
        else
        {
            OSG_WARN << "No bone found for \"" << fbxBone->GetName() << "\"" << std::endl;
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

        FbxManager* pSdkManager = FbxManager::Create();

        if (!pSdkManager)
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }

        CleanUpFbx cleanUpFbx(pSdkManager);

        pSdkManager->SetIOSettings(FbxIOSettings::Create(pSdkManager, IOSROOT));

        FbxScene* pScene = FbxScene::Create(pSdkManager, "");

        // The FBX SDK interprets the filename as UTF-8
#ifdef OSG_USE_UTF8_FILENAME
        const std::string& utf8filename(filename);
#else
        std::string utf8filename(osgDB::convertStringFromCurrentCodePageToUTF8(filename));
#endif

        FbxImporter* lImporter = FbxImporter::Create(pSdkManager, "");

        if (!lImporter->Initialize(utf8filename.c_str(), -1, pSdkManager->GetIOSettings()))
        {
#if FBXSDK_VERSION_MAJOR < 2014
            return std::string(lImporter->GetLastErrorString());
#else
            return std::string(lImporter->GetStatus().GetErrorString());
#endif
        }

        if (!lImporter->IsFBX())
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }

        for (int i = 0; FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i); i++)
        {
            lTakeInfo->mSelect = true;
        }

        if (!lImporter->Import(pScene))
        {
#if FBXSDK_VERSION_MAJOR < 2014
            return std::string(lImporter->GetLastErrorString());
#else 
            return std::string(lImporter->GetStatus().GetErrorString());
#endif
        }

        //FbxAxisSystem::OpenGL.ConvertScene(pScene);        // Doesn't work as expected. Still need to transform vertices.

        if (FbxNode* pNode = pScene->GetRootNode())
        {
            bool useFbxRoot = false;
            bool lightmapTextures = false;
            bool tessellatePolygons = false;
            bool zUp = false;
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
                    if (opt == "LightmapTextures")
                    {
                        lightmapTextures = true;
                    }
                    if (opt == "TessellatePolygons")
                    {
                        tessellatePolygons = true;
                    }
                    if (opt == "ZUp")
                    {
                        zUp = true;
                    }
                }
            }

            bool bIsBone = false;
            int nLightCount = 0;
            osg::ref_ptr<Options> localOptions = NULL;
            if (options)
            {
                localOptions = options->cloneOptions();
            }
            else
            {
                localOptions = new osgDB::Options();
                localOptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_IMAGES);
            }

            std::string filePath = osgDB::getFilePath(filename);
            FbxMaterialToOsgStateSet fbxMaterialToOsgStateSet(filePath, localOptions.get(), lightmapTextures);

            std::set<const FbxNode*> fbxSkeletons;
            findLinkedFbxSkeletonNodes(pNode, fbxSkeletons);

            OsgFbxReader::AuthoringTool authoringTool = OsgFbxReader::UNKNOWN;
            if (FbxDocumentInfo* pDocInfo = pScene->GetDocumentInfo())
            {
                struct ToolName
                {
                    const char* name;
                    OsgFbxReader::AuthoringTool tool;
                };

                ToolName authoringTools[] = {
                    {"OpenSceneGraph", OsgFbxReader::OPENSCENEGRAPH},
                    {"3ds Max", OsgFbxReader::AUTODESK_3DSTUDIO_MAX}
                };

                FbxString appName = pDocInfo->LastSaved_ApplicationName.Get();

                for (unsigned int i = 0; i < sizeof(authoringTools) / sizeof(authoringTools[0]); ++i)
                {
                    if (0 ==
#ifdef WIN32
                        _strnicmp
#else
                        strncasecmp
#endif
                        (appName, authoringTools[i].name, strlen(authoringTools[i].name)))
                    {
                        authoringTool = authoringTools[i].tool;
                        break;
                    }
                }
            }


            OsgFbxReader reader(*pSdkManager,
                *pScene,
                fbxMaterialToOsgStateSet,
                fbxSkeletons,
                *localOptions,
                authoringTool,
                lightmapTextures,
                tessellatePolygons);

            ReadResult res = reader.readFbxNode(pNode, bIsBone, nLightCount);

            if (res.success())
            {
                fbxMaterialToOsgStateSet.checkInvertTransparency();

                resolveBindMatrices(*res.getNode(), reader.boneBindMatrices, reader.nodeMap);

                osg::Node* osgNode = res.getNode();
                osgNode->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL,osg::StateAttribute::ON);
                osgNode->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

                if (reader.pAnimationManager.valid())
                {
                    if (osgNode->getUpdateCallback())
                    {
                        osg::Group* osgGroup = new osg::Group;
                        osgGroup->addChild(osgNode);
                        osgNode = osgGroup;
                    }

                    //because the animations may be altered after registering
                    reader.pAnimationManager->buildTargetReference();
                    osgNode->setUpdateCallback(reader.pAnimationManager.get());
                }

                FbxAxisSystem fbxAxis = pScene->GetGlobalSettings().GetAxisSystem();
                // some reminder: http://www.realtimerendering.com/blog/left-handed-vs-right-handed-world-coordinates/                
                int upSign;
                FbxAxisSystem::EUpVector eUp = fbxAxis.GetUpVector(upSign);
                bool bLeftHanded = fbxAxis.GetCoorSystem() == FbxAxisSystem::eLeftHanded;
                float fSign = upSign < 0 ? 1.0f : -1.0f;
                float HorizSign = bLeftHanded ? -1.0f : 1.0f;

                bool refCoordSysChange = false;
                osg::Matrix mat;
               
                if (zUp)
                {
                    if (eUp != FbxAxisSystem::eZAxis || fSign != 1.0  || upSign != 1.0)
                    {                    
                        switch (eUp)
                        {
                        case FbxAxisSystem::eXAxis:
                            mat.set(0,fSign,0,0,-fSign,0,0,0,0,0,HorizSign,0,0,0,0,1);
                            break;
                        case FbxAxisSystem::eYAxis:
                            mat.set(1,0,0,0,0,0,-fSign*HorizSign,0,0,fSign,0,0,0,0,0,1);
                            break;
                        case FbxAxisSystem::eZAxis:
                            mat.set(1,0,0,0,0,fSign,0,0,0,0,fSign*HorizSign,0,0,0,0,1);
                            break;
                        }
                        refCoordSysChange = true;
                    }
                } 
                else if (fbxAxis != FbxAxisSystem::OpenGL)
                {
                    switch (eUp)
                    {
                    case FbxAxisSystem::eXAxis:
                        mat.set(0,-fSign,0,0,fSign,0,0,0,0,0,HorizSign,0,0,0,0,1);
                        break;
                    case FbxAxisSystem::eYAxis:
                        mat.set(1,0,0,0,0,-fSign,0,0,0,0,-fSign*HorizSign,0,0,0,0,1);
                        break;
                    case FbxAxisSystem::eZAxis:
                        mat.set(1,0,0,0,0,0,fSign*HorizSign,0,0,-fSign,0,0,0,0,0,1);
                        break;
                    } 
                    refCoordSysChange = true;
                }
                if (refCoordSysChange)
                {
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
        OSG_WARN << "Exception thrown while importing \"" << filenameInit << '\"' << std::endl;
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

        FbxManager* pSdkManager = FbxManager::Create();

        if (!pSdkManager)
        {
            return WriteResult::ERROR_IN_WRITING_FILE;
        }

        CleanUpFbx cleanUpFbx(pSdkManager);

        pSdkManager->SetIOSettings(FbxIOSettings::Create(pSdkManager, IOSROOT));

        bool useFbxRoot = false;
        bool ascii(false);
        std::string exportVersion;
        if (options)
        {
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                if (opt == "Embedded")
                {
                    pSdkManager->GetIOSettings()->SetBoolProp(EXP_FBX_EMBEDDED, true);
                }
                else if (opt == "UseFbxRoot")
                {
                    useFbxRoot = true;
                }
                else if (opt == "FBX-ASCII")
                {
                    ascii = true;
                }
                else if (opt == "FBX-ExportVersion")
                {
                    iss >> exportVersion;
                }
            }
        }

        FbxScene* pScene = FbxScene::Create(pSdkManager, "");

        if (options)
        {
            if (options->getPluginData("FBX-AssetUnitMeter"))
            {
                float unit = *static_cast<const float*>(options->getPluginData("FBX-AssetUnitMeter"));
                FbxSystemUnit kFbxSystemUnit(unit*100);
                pScene->GetGlobalSettings().SetSystemUnit(kFbxSystemUnit);
            }
        }

        pluginfbx::WriterNodeVisitor writerNodeVisitor(pScene, pSdkManager, filename,
            options, osgDB::getFilePath(node.getName().empty() ? filename : node.getName()));
        if (useFbxRoot && isBasicRootNode(node))
        {
            // If root node is a simple group, put all elements under the FBX root
            const osg::Group * osgGroup = node.asGroup();
            for (unsigned int child = 0; child < osgGroup->getNumChildren(); ++child)
            {
                const_cast<osg::Node *>(osgGroup->getChild(child))->accept(writerNodeVisitor);
            }
        }
        else {
            // Normal scene
            const_cast<osg::Node&>(node).accept(writerNodeVisitor);
        }

        FbxDocumentInfo* pDocInfo = pScene->GetDocumentInfo();
        bool needNewDocInfo = pDocInfo != NULL;
        if (needNewDocInfo)
        {
            pDocInfo = FbxDocumentInfo::Create(pSdkManager, "");
        }
        pDocInfo->LastSaved_ApplicationName.Set(FbxString("OpenSceneGraph"));
        pDocInfo->LastSaved_ApplicationVersion.Set(FbxString(osgGetVersion()));
        if (needNewDocInfo)
        {
            pScene->SetDocumentInfo(pDocInfo);
        }

        FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");

        // default axis system is openGL
        FbxAxisSystem::EPreDefinedAxisSystem axisSystem = FbxAxisSystem::eOpenGL;

        // check options
        if (options)
        {
           std::string axisOption = options->getPluginStringData("FBX-AxisSystem");
           if (!axisOption.empty())
           {
              if (axisOption == "MayaZUp")
                 axisSystem = FbxAxisSystem::eMayaZUp;
              else if (axisOption == "MayaYUp")
                 axisSystem = FbxAxisSystem::eMayaYUp;
              else if (axisOption == "Max")
                 axisSystem = FbxAxisSystem::eMax;
              else if (axisOption == "MotionBuilder")
                 axisSystem = FbxAxisSystem::eMotionBuilder;
              else if (axisOption == "OpenGL")
                 axisSystem = FbxAxisSystem::eOpenGL;
              else if (axisOption == "DirectX")
                 axisSystem = FbxAxisSystem::eDirectX;
              else if (axisOption == "Lightwave")
                 axisSystem = FbxAxisSystem::eLightwave;
           }
        }

        pScene->GetGlobalSettings().SetAxisSystem(axisSystem);

        // Ensure the directory exists or else the FBX SDK will fail
        if (!osgDB::makeDirectoryForFile(filename)) {
            OSG_NOTICE << "Can't create directory for file '" << filename << "'. FBX SDK may fail creating the file." << std::endl;
        }

        // The FBX SDK interprets the filename as UTF-8
#ifdef OSG_USE_UTF8_FILENAME
        const std::string& utf8filename(filename);
#else
        std::string utf8filename(osgDB::convertStringFromCurrentCodePageToUTF8(filename));
#endif

        // Output format selection. Here we only handle "recent" FBX, binary or ASCII.
        // pSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription() / GetReaderFormatCount() gives the following list:
        //FBX binary (*.fbx)
        //FBX ascii (*.fbx)
        //FBX encrypted (*.fbx)
        //FBX 6.0 binary (*.fbx)
        //FBX 6.0 ascii (*.fbx)
        //FBX 6.0 encrypted (*.fbx)
        //AutoCAD DXF (*.dxf)
        //Alias OBJ (*.obj)
        //Collada DAE (*.dae)
        //Biovision BVH (*.bvh)
        //Motion Analysis HTR (*.htr)
        //Motion Analysis TRC (*.trc)
        //Acclaim ASF (*.asf)
        int format = ascii ? pSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)") : -1;        // -1 = Default
        if (!lExporter->Initialize(utf8filename.c_str(), format))
        {
#if FBXSDK_VERSION_MAJOR < 2014
            return std::string(lExporter->GetLastErrorString());
#else
            return std::string(lExporter->GetStatus().GetErrorString());
#endif
        }

        if (!exportVersion.empty() && !lExporter->SetFileExportVersion(FbxString(exportVersion.c_str()), FbxSceneRenamer::eNone)) {
            std::stringstream versionsStr;
            char const * const * versions = lExporter->GetCurrentWritableVersions();
            if (versions) for(; *versions; ++versions) versionsStr << " " << *versions;
            OSG_WARN << "Can't set FBX export version to '" << exportVersion << "'. Using default. Available export versions are:" << versionsStr.str() << std::endl;
        }

        if (!lExporter->Export(pScene))
        {
#if FBXSDK_VERSION_MAJOR < 2014
            return std::string(lExporter->GetLastErrorString());
#else
            return std::string(lExporter->GetStatus().GetErrorString());
#endif
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
