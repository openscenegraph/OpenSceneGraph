#include <osgViewer/Version>
#include <osg/Version>

extern "C" {

const char* osgViewerGetVersion()
{
    return osgGetVersion();
}


const char* osgViewerGetLibraryName()
{
    return "OpenSceneGraph Viewer Library";
}

}
