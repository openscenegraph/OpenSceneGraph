//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// forward declaration
namespace osgUtil {
    class SceneView;
    class CameraManipulator;
}

#include <wx/app.h>

#include <osg/Node>
#include <osgUtil/GUIActionAdapter>

// Define a new application type
class wxosgApp: public wxApp, public osgUtil::GUIActionAdapter
{
public:
    wxosgApp();
    bool OnInit();
    void DoUpdate();
    void SetWindowSize(int x, int y);
    void LoadFile(const char *filename);
    void ZoomTo(osg::Node *node);

    osgUtil::CameraManipulator *Manip() { return m_pCameraManipulator.get(); }
    osg::Node *Root();

    virtual void requestRedraw() {}
    virtual void requestContinuousUpdate(bool needed=true) {}
    virtual void requestWarpPointer(int x,int y) {}

protected:
    osg::ref_ptr<osgUtil::SceneView>            m_pSceneView;
    osg::ref_ptr<osgUtil::CameraManipulator>    m_pCameraManipulator;
    int            m_winx, m_winy;    // Window size
    bool        m_bInitialized;
};

