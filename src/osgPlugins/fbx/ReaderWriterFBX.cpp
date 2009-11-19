#include <sstream>
#include <memory>

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/Texture2D>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/Skeleton>

#include <OpenThreads/ScopedLock>

#if defined(_MSC_VER)
    #pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

#include "ReaderWriterFBX.h"
#include "fbxRNode.h"

osgDB::ReaderWriter::ReadResult
ReaderWriterFBX::readNode(const std::string& utf8filename,
                          const osgDB::ReaderWriter::Options* options) const
{
    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex);

    try
    {
        std::string ext(osgDB::getLowerCaseFileExtension(utf8filename));
        if(!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName(osgDB::findDataFile(utf8filename, options));
        if( fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        KFbxSdkManager* pSdkManager = KFbxSdkManager::Create();

        if (!pSdkManager)
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }

        class CleanUpFbx
        {
            KFbxSdkManager* m_pSdkManager;
        public:
            CleanUpFbx(KFbxSdkManager* pSdkManager) : m_pSdkManager(pSdkManager)
            {}

            ~CleanUpFbx()
            {
                KFbxIOSettings::IOSettingsRef().FreeIOSettings();
                m_pSdkManager->Destroy();
            }
        } cleanUpFbx(pSdkManager);

        KFbxScene* pScene = KFbxScene::Create(pSdkManager,"");

        int fileFormat;
        if (!pSdkManager->GetIOPluginRegistry()->DetectFileFormat(utf8filename.c_str(), fileFormat))
        {
            return ReadResult::FILE_NOT_HANDLED;
        }

        KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager,"");
        lImporter->SetFileFormat(fileFormat);

        if (!lImporter->Initialize(utf8filename.c_str()))
        {
            return std::string(lImporter->GetLastErrorString());
        }

        if (!lImporter->IsFBX())
        {
            return ReadResult::ERROR_IN_READING_FILE;
        }

        for(int i = 0; i < lImporter->GetTakeCount(); i++)
        {
            KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            lTakeInfo->mSelect = true;
        }

        if (!lImporter->Import(pScene))
        {
            return std::string(lImporter->GetLastErrorString());
        }

        if (KFbxNode* pNode = pScene->GetRootNode())
        {
            osg::ref_ptr<osgAnimation::AnimationManagerBase> pAnimationManager;
            bool bNeedSkeleton = false;
            int nLightCount = 0;
            ReadResult res = readFbxNode(*pSdkManager, pNode, pAnimationManager,
                osgDB::getFilePath(fileName), bNeedSkeleton, nLightCount);
            if (res.success())
            {
                osg::Node* osgNode = res.getNode();
                if (bNeedSkeleton)
                {
                    osgAnimation::Skeleton* osgSkeleton = new osgAnimation::Skeleton;
                    osgSkeleton->setDefaultUpdateCallback();
                    osgSkeleton->addChild(osgNode);
                    osgNode = osgSkeleton;
                }
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
                int upSign;
                KFbxAxisSystem::eUpVector eUp = fbxAxis.GetUpVector(upSign);
                bool bLeftHanded = fbxAxis.GetCoorSystem() == KFbxAxisSystem::LeftHanded;
                if (eUp != KFbxAxisSystem::YAxis || upSign < 0 || bLeftHanded)
                {
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
                    osg::MatrixTransform* pTransform = new osg::MatrixTransform(mat);
                    pTransform->addChild(osgNode);
                    osgNode = pTransform;
                }

                return osgNode;
            }
        }
    }
    catch (...)
    {
    }

    return ReadResult::ERROR_IN_READING_FILE;
}

///////////////////////////////////////////////////////////////////////////
// Add ourself to the Registry to instantiate the reader/writer.

REGISTER_OSGPLUGIN(fbx, ReaderWriterFBX)
