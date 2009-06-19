// Demo for a 4-way split window with a different scene graph in
// each of the 4 views.  Also, an optional 5th view in it's own window.
// Qt 4.4.3 was used for this example.
//
#include <iostream>
#include <string>
#include <QtGui/QApplication>
#include <QtCore/QDebug>

#include <osgDB/ReadFile>
#include <osg/ArgumentParser>

#include "testMainWin.h"
#include "testOutboardWin.h"

#include <QtCore/QPointer>

#ifdef __APPLE__
#define DO_OUTBOARD_WINDOW 0  // won't go on Mac
#else
#define DO_OUTBOARD_WINDOW 1
#endif

using namespace std;


int main( int argc, char **argv )

{
    QApplication app( argc, argv );

    // load some standard files
    osg::ref_ptr<osg::Node> Cow = osgDB::readNodeFile("cow.osg");
    if (!Cow)
    {
        qDebug() << "No cow loaded.";
        return 1;
    }
    
    osg::ref_ptr<osg::Node> Truck = osgDB::readNodeFile("dumptruck.osg");
    if (!Truck)
    {
        qDebug() << "No truck loaded.";
        return 1;
    }

    osg::ref_ptr<osg::Node> Spaceship = osgDB::readNodeFile("spaceship.osg");
    if (!Spaceship)
    {
        qDebug() << "No spaceship loaded.";
        return 1;
    }

    osg::ref_ptr<osg::Node> Cessna = osgDB::readNodeFile("cessna.osg");
    if (!Cessna)
    {
        qDebug() << "No cessna loaded.";
        return 1;
    }

    osg::ref_ptr<osg::Node> Fountain = osgDB::readNodeFile("fountain.osg");
    if (!Fountain)
    {
        qDebug() << "No fountain loaded.";
        return 1;
    }

    ///////////////////////////////////////////////////////////////////////////
    app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );

    osg::ArgumentParser arguments(&argc, argv);

    QPointer<testMainWin> myMainWindow = new testMainWin;

#if DO_OUTBOARD_WINDOW
    QPointer<testOutboardWin> mySecondaryWindow = new testOutboardWin;
#endif

    // The .ui file uses the "Promoted" widget, CompositeViewerQOSG
    osg::ref_ptr<CompositeViewerQOSG> compositeViewer = myMainWindow->ui.osgGraphicsArea;

    osg::ref_ptr<ViewQOSG> view1 = new ViewQOSG( myMainWindow->ui.graphicsView1 );
    view1->setObjectName("ViewQOSG 1");
    osg::ref_ptr<ViewQOSG> view2 = new ViewQOSG( myMainWindow->ui.graphicsView2 );
    view2->setObjectName("ViewQOSG 2");
    osg::ref_ptr<ViewQOSG> view3 = new ViewQOSG( myMainWindow->ui.graphicsView3 );
    view3->setObjectName("ViewQOSG 3");
    osg::ref_ptr<ViewQOSG> view4 = new ViewQOSG( myMainWindow->ui.graphicsView4 );
    view4->setObjectName("ViewQOSG 4");

    view1->setData( Cow );
    view2->setData( Truck );
    view3->setData( Spaceship );
    view4->setData( Cessna );

    compositeViewer->addView( view1.get() );
    compositeViewer->addView( view2.get() );
    compositeViewer->addView( view3.get() );
    compositeViewer->addView( view4.get() );

    myMainWindow->show();

#if DO_OUTBOARD_WINDOW
    QWidget *outboardGfx = mySecondaryWindow->getDrawingAreaWidget();
    osg::ref_ptr<ViewQOSG> outboardView = static_cast<ViewQOSG*>( outboardGfx );
    outboardView->setObjectName("ViewQOSG Outboard");
    outboardView->setData( Fountain );

    // Note that outboardView, in a completely different window, is going to be
    // managed by the compositeViewer in the QMainWindow.
    compositeViewer->addView( outboardView.get() );

    mySecondaryWindow->show();
#endif

    return app.exec();

}
