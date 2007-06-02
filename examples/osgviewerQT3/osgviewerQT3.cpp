// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.

#include <osgViewer/Viewer>
#include <osgViewer/StatsHandler>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#if USE_QT3

    class QWidget;
    #include <qtimer.h>
    #include <qgl.h>
    #include <qapplication.h>

#else

    #include <QtCore/QTimer>
    #include <QtGui/QKeyEvent>
    #include <QtGui/QApplication>
    #include <QtOpenGL/QGLWidget>

    using Qt::WFlags;
#endif

#include <iostream>

class AdapterWidget : public QGLWidget
{
public:

    AdapterWidget( QWidget * parent = 0, const char * name = 0, const QGLWidget * shareWidget = 0, WFlags f = 0 );

    virtual ~AdapterWidget() {}
    
    osgViewer::GraphicsWindow* getGraphicsWindow() { return _gw.get(); }
    const osgViewer::GraphicsWindow* getGraphicsWindow() const { return _gw.get(); }

protected:

    void init();

    virtual void resizeGL( int width, int height );
    virtual void keyPressEvent( QKeyEvent* event );
    virtual void keyReleaseEvent( QKeyEvent* event );
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );

    QTimer _timer;
    
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;
};

AdapterWidget::AdapterWidget( QWidget * parent, const char * name, const QGLWidget * shareWidget, WFlags f):
    QGLWidget(parent, name, shareWidget, f)
{
    connect(&_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    _timer.start(10);
    
    _gw = new osgViewer::GraphicsWindowEmbedded(0,0,width(),height());
}

void AdapterWidget::resizeGL( int width, int height )
{
    _gw->getEventQueue()->windowResize(0, 0, width, height );
    _gw->resized(0,0,width,height);
}

void AdapterWidget::keyPressEvent( QKeyEvent* event )
{
    _gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) event->ascii() );
}

void AdapterWidget::keyReleaseEvent( QKeyEvent* event )
{
    _gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) event->ascii() );
}

void AdapterWidget::mousePressEvent( QMouseEvent* event )
{
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseButtonPress(event->x(), event->y(), button);
}

void AdapterWidget::mouseReleaseEvent( QMouseEvent* event )
{
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseButtonRelease(event->x(), event->y(), button);
}

void AdapterWidget::mouseMoveEvent( QMouseEvent* event )
{
    _gw->getEventQueue()->mouseMotion(event->x(), event->y());
}


class ViewerQT : public AdapterWidget, public osgViewer::Viewer
{
    public:

        ViewerQT(QWidget * parent = 0, const char * name = 0, const QGLWidget * shareWidget = 0, WFlags f = 0):
            AdapterWidget( parent, name, shareWidget, f )
        {
            getCamera()->setViewport(new osg::Viewport(0,0,width(),height()));
            getCamera()->setGraphicsContext(getGraphicsWindow());
            setThreadingModel(osgViewer::Viewer::SingleThreaded);
        }

        virtual void initializeGL()
        {
            QGLWidget::initializeGL();
        }

        virtual void paintGL()
        {
            frame();
        }

};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    
    if (argc<2)
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel)
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }


    ViewerQT* viewerWindow = new ViewerQT;

    viewerWindow->setSceneData(loadedModel.get());
    viewerWindow->setCameraManipulator(new osgGA::TrackballManipulator);
    viewerWindow->addEventHandler(new osgViewer::StatsHandler);

    viewerWindow->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
   
    return a.exec();
}

/*EOF*/
