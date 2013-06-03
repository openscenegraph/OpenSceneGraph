#include <osg/CameraView>
#include <osg/Notify>

#include <osgDB/ReadFile>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#pragma warning( default : 4996 )
#endif
#include <fbxsdk.h>

#include "fbxReader.h"

osgDB::ReaderWriter::ReadResult OsgFbxReader::readFbxCamera(FbxNode* pNode)
{
    const FbxCamera* fbxCamera = FbxCast<FbxCamera>(pNode->GetNodeAttribute());

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
        case FbxCamera::eHorizontal:
            osgCameraView->setFieldOfViewMode(osg::CameraView::HORIZONTAL);
            break;
        case FbxCamera::eVertical:
            osgCameraView->setFieldOfViewMode(osg::CameraView::VERTICAL);
            break;
        case FbxCamera::eHorizAndVert:
        case FbxCamera::eFocalLength:
        default:
            OSG_WARN << "readFbxCamera: Unsupported Camera aperture mode." << std::endl;
            break;
        }
    }

    return osgDB::ReaderWriter::ReadResult(osgCameraView);
}
