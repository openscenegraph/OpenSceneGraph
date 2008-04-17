/* OpenSceneGraph example, osganimate.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#if USE_QT4

    #include <QtCore/QString>
    #include <QtCore/QTimer>
    #include <QtGui/QKeyEvent>
    #include <QtGui/QApplication>
    #include <QtOpenGL/QGLWidget>
    #include <QtGui/QMainWindow>
    #include <QtGui/QMdiSubWindow>
    #include <QtGui/QMdiArea>
    
    using Qt::WindowFlags;

#else

    class QWidget;
    #include <qtimer.h>
    #include <qgl.h>
    #include <qapplication.h>

    #define WindowFlags WFlags

#endif

#include <iostream>

class AdapterWidget : public QGLWidget
{
    public:

        AdapterWidget( QWidget * parent = 0, const char * name = 0, const QGLWidget * shareWidget = 0, WindowFlags f = 0 );

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

        osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;
};

AdapterWidget::AdapterWidget( QWidget * parent, const char * name, const QGLWidget * shareWidget, WindowFlags f):
#if USE_QT4
    QGLWidget(parent, shareWidget, f)
#else
    QGLWidget(parent, name, shareWidget, f)
#endif
{
    _gw = new osgViewer::GraphicsWindowEmbedded(0,0,width(),height());
#if USE_QT4
    setFocusPolicy(Qt::ClickFocus);
#else
    setFocusPolicy(QWidget::ClickFocus);
#endif
}

void AdapterWidget::resizeGL( int width, int height )
{
    _gw->getEventQueue()->windowResize(0, 0, width, height );
    _gw->resized(0,0,width,height);
}

void AdapterWidget::keyPressEvent( QKeyEvent* event )
{
#if USE_QT4
    _gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data() ) );
#else
    _gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) event->ascii() );
#endif
}

void AdapterWidget::keyReleaseEvent( QKeyEvent* event )
{
#if USE_QT4
    _gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data() ) );
#else
    _gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) event->ascii() );
#endif
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


class ViewerQT : public osgViewer::Viewer, public AdapterWidget
{
    public:

        ViewerQT(QWidget * parent = 0, const char * name = 0, const QGLWidget * shareWidget = 0, WindowFlags f = 0):
            AdapterWidget( parent, name, shareWidget, f )
        {
            getCamera()->setViewport(new osg::Viewport(0,0,width(),height()));
            getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width())/static_cast<double>(height()), 1.0f, 10000.0f);
            getCamera()->setGraphicsContext(getGraphicsWindow());

            setThreadingModel(osgViewer::Viewer::SingleThreaded);

            connect(&_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
            _timer.start(10);
        }

        virtual void paintGL()
        {
            frame();
        }
    
    protected:

        QTimer _timer;
};

class CompositeViewerQT : public osgViewer::CompositeViewer, public AdapterWidget
{
    public:

        CompositeViewerQT(QWidget * parent = 0, const char * name = 0, const QGLWidget * shareWidget = 0, WindowFlags f = 0):
            AdapterWidget( parent, name, shareWidget, f )
        {
            setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);

            connect(&_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
            _timer.start(10);
        }

        virtual void paintGL()
        {
            frame();
        }
    
    protected:

        QTimer _timer;
};

int mainAdapterWidget(QApplication& a, osg::ArgumentParser& arguments)
{
    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments[0] <<": No data loaded." << std::endl;
        return 1;
    }
    
    std::cout<<"Using AdapterWidget - QGLWidget subclassed to integrate with osgViewer using its embedded graphics window support."<<std::endl;
    
    if (arguments.read("--CompositeViewer"))
    {
        CompositeViewerQT* viewerWindow = new CompositeViewerQT;

        unsigned int width = viewerWindow->width();
        unsigned int height = viewerWindow->height();
        
        {
            osgViewer::View* view1 = new osgViewer::View;
            view1->getCamera()->setGraphicsContext(viewerWindow->getGraphicsWindow());
            view1->getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width)/static_cast<double>(height/2), 1.0, 1000.0);
            view1->getCamera()->setViewport(new osg::Viewport(0,0,width,height/2));
            view1->setCameraManipulator(new osgGA::TrackballManipulator);
            view1->setSceneData(loadedModel.get());
            
            viewerWindow->addView(view1);
        }
        
        {
            osgViewer::View* view2 = new osgViewer::View;
            view2->getCamera()->setGraphicsContext(viewerWindow->getGraphicsWindow());
            view2->getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width)/static_cast<double>(height/2), 1.0, 1000.0);
            view2->getCamera()->setViewport(new osg::Viewport(0,height/2,width,height/2));
            view2->setCameraManipulator(new osgGA::TrackballManipulator);
            view2->setSceneData(loadedModel.get());
            
            viewerWindow->addView(view2);
        }

        viewerWindow->show();
#if USE_QT4
    }
    else if (arguments.read("--mdi")) {
          std::cout<<"Using ViewetQT MDI version"<<std::endl;
         /*
         Following problems are found here:
         - miminize causes loaded model to disappear (some problem with Camera matrix? - clampProjectionMatrix is invalid)
         */
         ViewerQT* viewerWindow = new ViewerQT;
         viewerWindow->setCameraManipulator(new osgGA::TrackballManipulator);
         viewerWindow->setSceneData(loadedModel.get());
 
         QMainWindow* mw = new QMainWindow();
         QMdiArea* mdiArea = new QMdiArea(mw);
         mw->setCentralWidget(mdiArea);

         QMdiSubWindow *subWindow = mdiArea->addSubWindow(viewerWindow);
         subWindow->showMaximized();
         subWindow->setWindowTitle("New Window");
         mw->show();
#endif // USE_QT4
    } else {
        ViewerQT* viewerWindow = new ViewerQT;

        viewerWindow->setCameraManipulator(new osgGA::TrackballManipulator);
        viewerWindow->setSceneData(loadedModel.get());

        viewerWindow->show();
    }    
    
    
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
   
    return a.exec();
}

/*EOF*/
