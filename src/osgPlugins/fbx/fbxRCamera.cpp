#include <osg/CameraView>
#include <osg/Notify>

#include <osgDB/ReadFile>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#pragma warning( default : 4996 )
#endif
#include <fbxsdk.h>

#include "fbxReader.h"

osgDB::ReaderWriter::ReadResult OsgFbxReader::readFbxCamera(KFbxNode* pNode)
{
    const KFbxCamera* fbxCamera = KFbxCast<KFbxCamera>(pNode->GetNodeAttribute());

    if (!fbxCamera)
    {
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }

    osg::CameraView* osgCameraView = new osg::CameraView;

    if (fbxCamera->FieldOfView.IsValid())
    {
        osgCameraView->setFieldOfView(fbxCamera->FieldOfView.Get());
    }

    if (fbxCamera->FocalLength.IsValid())
    {
        osgCameraView->setFocalLength(fbxCamera->FocalLength.Get());
    }

    if (fbxCamera->ApertureMode.IsValid())
    {
        switch (fbxCamera->ApertureMode.Get())
        {
        case KFbxCamera::eHORIZONTAL:
            osgCameraView->setFieldOfViewMode(osg::CameraView::HORIZONTAL);
            break;
        case KFbxCamera::eVERTICAL:
            osgCameraView->setFieldOfViewMode(osg::CameraView::VERTICAL);
            break;
        case KFbxCamera::eHORIZONTAL_AND_VERTICAL:
        case KFbxCamera::eFOCAL_LENGTH:
        default:
            OSG_WARN << "readFbxCamera: Unsupported Camera aperture mode." << std::endl;
            break;
        }
    }

    return osgDB::ReaderWriter::ReadResult(osgCameraView);
}
