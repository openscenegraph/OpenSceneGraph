#include <osg/CameraView>
#include <osg/Notify>

#include <osgDB/ReadFile>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

#include "fbxRCamera.h"

osgDB::ReaderWriter::ReadResult readFbxCamera(KFbxNode* pNode)
{
    const KFbxCamera* fbxCamera = dynamic_cast<const KFbxCamera*>(pNode->GetNodeAttribute());

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
            osg::notify(osg::WARN) << "readFbxCamera: Unsupported Camera aperture mode." << std::endl;
            break;
        }
    }

    return osgDB::ReaderWriter::ReadResult(osgCameraView);
}
