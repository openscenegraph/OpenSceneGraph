//
// Name:    app.cpp
// Purpose:    The application class for a wxWindows application.
// Author:    Ben Discoe, ben@washedashore.com
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <osgUtil/SceneView>
#include <osgUtil/TrackballManipulator>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgWX/WXEventAdapter>

using namespace osg;
using namespace osgWX;

#include "app.h"
#include "frame.h"

IMPLEMENT_APP(wxosgApp)

wxosgApp::wxosgApp()
{
    m_bInitialized = false;
}

//
// Initialize the app object
//
bool wxosgApp::OnInit()
{
    // Create the main frame window
    wxString title = "wxsgv Demo Viewer";
    wxosgFrame *frame = new wxosgFrame(NULL, title,
        wxPoint(50, 50), wxSize(800, 600));

    //
    // Create the 3d scene
    //
    m_pSceneView = new osgUtil::SceneView();
    m_pSceneView->setDefaults();
    Camera *pCam = new Camera();
    m_pSceneView->setCamera(pCam);

    m_pCameraManipulator = new osgUtil::TrackballManipulator();
    m_pCameraManipulator->setCamera(pCam);

    ref_ptr<WXEventAdapter> ea = new WXEventAdapter;
    m_pCameraManipulator->init(*ea, *this);

    m_bInitialized = true;

    return true;
}

void wxosgApp::DoUpdate()
{
    if (!m_bInitialized)
        return;

    m_pSceneView->setViewport(0, 0, m_winx, m_winy);
    m_pSceneView->cull();
    m_pSceneView->draw();
}

void wxosgApp::SetWindowSize(int x, int y)
{
    m_winx = x;
    m_winy = y;
}

void wxosgApp::LoadFile(const char *filename)
{
    Node *node = osgDB::readNodeFile(filename);

    if (!node)
        return;

    m_pSceneView->setSceneData(node);
    m_pCameraManipulator->setNode(node);

    ref_ptr<WXEventAdapter> ea = new WXEventAdapter;
    m_pCameraManipulator->home(*ea, *this);
}

osg::Node *wxosgApp::Root()
{
    return m_pSceneView->getSceneData();
}

void wxosgApp::ZoomTo(Node *node)
{
    m_pCameraManipulator->setNode(node);
    ref_ptr<WXEventAdapter> ea = new WXEventAdapter;
    m_pCameraManipulator->home(*ea, *this);
}



