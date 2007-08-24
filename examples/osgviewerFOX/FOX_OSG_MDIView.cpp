#include "FOX_OSG_MDIView.h"

#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>

#include <osgDB/ReadFile>


// Map
FXDEFMAP(FOX_OSG_MDIView) FOX_OSG_MDIView_Map[] = {
    //________Message_Type_________        ___ID___                        ________Message_Handler________
    FXMAPFUNC(SEL_CHORE,                FOX_OSG_MDIView::ID_CHORE,        FOX_OSG_MDIView::OnIdle)
};

FXIMPLEMENT(FOX_OSG_MDIView, FXMDIChild, FOX_OSG_MDIView_Map, ARRAYNUMBER(FOX_OSG_MDIView_Map))

FOX_OSG_MDIView::FOX_OSG_MDIView(FXMDIClient *p, const FXString &name,
        FXIcon *ic, FXPopup *pup, FXuint opt,
        FXint x, FXint y, FXint w, FXint h)
        :   FXMDIChild(p, name, ic, pup, opt, x, y, w, h)
{
    // A visual to drag OpenGL in double-buffered mode; note the glvisual is
    // shared between all windows which need the same depths and numbers of buffers
    // Thus, while the first visual may take some time to initialize, each subsequent
    // window can be created very quickly; we need to determine grpaphics hardware
    // characteristics only once.
    FXGLVisual* glVisual=new FXGLVisual(getApp(),VISUAL_DOUBLEBUFFER|VISUAL_STEREO);

    m_gwFox = new GraphicsWindowFOX(this, glVisual, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y, x, y, w, h );

    osgViewer::Viewer *viewer = new osgViewer::Viewer;
    viewer->getCamera()->setGraphicsContext(m_gwFox);
    viewer->getCamera()->setViewport(0,0,w,h);
    viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // FOX example does not catch the close of the graphics window, so
    // don't allow the default escape sets to done to be active.
    viewer->setKeyEventSetsDone(0);

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile("cow.osg");
    if (!loadedModel)
    {
        return ;
    }

    // add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);

    viewer->setSceneData(loadedModel.get());

    viewer->setCameraManipulator(new osgGA::TrackballManipulator);

    SetViewer(viewer);

    getApp()->addChore(this,ID_CHORE);

}


FOX_OSG_MDIView::~FOX_OSG_MDIView()
{
    getApp()->removeChore(this,ID_CHORE);
}

long FOX_OSG_MDIView::OnIdle(FXObject *sender, FXSelector sel, void* ptr)
{
    m_osgViewer->frame();
    getApp()->addChore(this, ID_CHORE);
    return 1;
}

void FOX_OSG_MDIView::SetViewer(osgViewer::Viewer* viewer)
{
    m_osgViewer = viewer;
}
