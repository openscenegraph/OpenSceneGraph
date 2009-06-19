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

#include <QtCore/QDebug>
#include <CompositeViewerQOSG.h>
#include <QOSGWidget.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#include <osgViewer/api/Win32/GraphicsWindowWin32>
typedef HWND WindowHandle;
typedef osgViewer::GraphicsWindowWin32::WindowData WindowData;
#elif defined(__APPLE__)  // Assume using Carbon on Mac.
// #include <Carbon/Carbon.h>
#include <osgViewer/api/Carbon/GraphicsWindowCarbon>
typedef WindowRef WindowHandle;
typedef osgViewer::GraphicsWindowCarbon::WindowData WindowData;
#else // all other unix
#include <osgViewer/api/X11/GraphicsWindowX11>
typedef Window WindowHandle;
typedef osgViewer::GraphicsWindowX11::WindowData WindowData;
// 10/17/08 LM -- osgViewer/api/X11/GraphicsWindowX11 includes X.h.
//                X.h defines KeyPress and KeyRelease.
//                By doing so, using QEvent::KeyPress resolves into QEvent::2,
//                causing a compile error.
#undef KeyPress
#undef KeyRelease
#endif

QOSGWidget::QOSGWidget( QWidget * parent, WindowFlags f)
    : QWidget(parent, f),
      _gw(0)
{
#if 0
    // This was a win on Linux.  Same as WA_PaintOnScreen?
    extern void qt_x11_set_global_double_buffer(bool);
    qt_x11_set_global_double_buffer(false);
#endif
    createContext(parent);

// Date: Tue, 16 Jun 2009 10:07:16 +0000
// From: "Eric Pouliquen" <epouliquen@silicon-worlds.fr>
// Subject: Re: [osg-submissions] New QOSGWidget demo with a 4-way split
//         window        and bonus outboard window.
// Suggested replacing the two setAttribute calls with this...
//    setAttribute(Qt::WA_OpaquePaintEvent);
//  This solverd a flickering problem on Windows 
//  8600 GT (185.85) and a Quadro FX1400 (182.65)
// but causes a visible black border to be visible in the rendering 
// windows on Linux.  This problem gone when WA_PaintOnScreen
// is used in combination.

    // Hmmm...
    // According to Qt doc, WA_PaintOnScreen is X11 only and disables
    // double-buffering.  I think this just means it disables a
    // buffer swap under Qt control.  We want OSG to have full control.
    //
    // Equivalent to qt_x11_set_global_double_buffer(false)?
    //
    // Tried turning it off and got severe flashing on Linux. 
    // Looks like without this we get an extraneous clear and
    // buffer swap form Qt.
    setAttribute(Qt::WA_PaintOnScreen);
    // This flags that something other than Qt is responsible for
    // all rendering in the window under the widget's control.
    setAttribute(Qt::WA_OpaquePaintEvent);
    // This seems superfluous now.
    // setAttribute(Qt::WA_NoSystemBackground);
 
// Here or in ViewQOSG?
// Either way is OK; but since this class is also the one implementing
// eventFilter(), I thought it might be wiser here.
// Qt
    // If you want to see how TAB and SHIFT-TAB work wrt focus, 
    // uncomment the following line.
    // NOTE:  If focusPolicy was set in the .ui file, that setting will
    //        override this.  So don't set focusPolicy in the .ui file!
    if (parent)
    {

        // The desire is to allow tabbing among the views of the 
        // composite viewer.  On Linux, I could get that to work in many
        // different ways, including setting StrongFocus on the QOSGWidget and
        // re-implementing focusInEvent() in ViewQOSG.  But the only thing
        // that worked on Windows was to set StrongFocus on the *parent*, NOT
        // on this QOSGWidget, AND to have ViewQOSG::keyReleaseEvent() do
        // something special on the release of a Tab or Shift-Tab key.  (The
        // release is seen by the ViewQOSG that is getting the focus.)

        parent->setFocusPolicy( Qt::StrongFocus );

        // This instance of the QOSGWidget becomes the filter object on its
        // 'parent'.  Ie, the child is now filtering events for the parent
        // QWidget.
        parent->installEventFilter( this );

        qDebug() << "parent->width() is " << parent->width();
        qDebug() << "parent->height() is " << parent->height();
        qDebug() << "parent->x() is " << parent->x();
        qDebug() << "parent->y() is " << parent->y();

        qDebug() << "width() is " << width();
        qDebug() << "height() is " << height();
        qDebug() << "x() is " << x();
        qDebug() << "y() is " << y();
    }
    else
        setFocusPolicy( Qt::StrongFocus );
}

void QOSGWidget::createContext(QWidget * parent)
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance();

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = 
        new osg::GraphicsContext::Traits;

    traits->readDISPLAY();
    if (traits->displayNum<0)
        traits->displayNum = 0;

    traits->windowName = "qosgwidget";
    traits->screenNum = 0;
// original location:
//    traits->x = x();
//    traits->y = y();
//    traits->width = width();
//    traits->height = height();
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

#if defined(__APPLE__)
    // Extract a WindowPtr from the HIViewRef that QWidget::winId() returns.
    // Without this change, the peer tries to call GetWindowPort on the HIViewRef
    // which returns 0 and we only render white.
    traits->inheritedWindowData = new WindowData(HIViewGetWindow((HIViewRef)winId()));

#else // all others
    traits->inheritedWindowData = new WindowData(winId());
#endif
    if (ds->getStereo())
    {
        switch(ds->getStereoMode())
        {
            case(osg::DisplaySettings::QUAD_BUFFER): 
                traits->quadBufferStereo = true; 
                break;
            case(osg::DisplaySettings::VERTICAL_INTERLACE):
            case(osg::DisplaySettings::CHECKERBOARD):
            case(osg::DisplaySettings::HORIZONTAL_INTERLACE): 
                traits->stencil = 8; 
                break;
            default: break;
        }
    }

    osg::ref_ptr<osg::GraphicsContext> gc = 
        osg::GraphicsContext::createGraphicsContext(traits.get());

    _gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());

    // A person named Lukas on the OSG group posted that to support embedding
    // in another window, the following traits had to be set after the
    // graphics context was created.  And he's right, if embedding, you
    // definitely have to do this AFTER creating the context....  Since it
    // also works when creating a top-level window, do it here for all cases

    // We may have just gotten some Bad Window errors from X11 calling
    // XGetWindowAttributes before the Window has been realized.
    // We'll also have garbage in the traits which get fixed up next.

    // _overrideTraits is superfluous now.

    if (parent)
    {
        traits->x = parent->x();
        traits->y = parent->y();
        traits->width = parent->width();
        traits->height = parent->height();
    }
    else
    {
        traits->x = x();
        traits->y = y();
        traits->width = width();
        traits->height = height();
    }
}

//------------------------------------------------------------------------------
// From the Qt Doc:
// "eventFilter() can accept or reject the event, and allow or deny further
//  processing of the event.  
// 
//  If all event filters allow further processing of an event (by each
//  returning false), the event is sent to the target object, in this case, the
//  parent of this QOSGWidget instance. 
// 
//  If one of them stops processing (by returning true), the target (ie the
//  parent widget) and any later event filters do not get to see the event at
//  all. "
// 
// Effectively, this filter takes all the events it's interested in, most
// notably keyboard events, and passes them along to OSG and *prevents* them
// from reaching their intended target, this widget's parent.
// 
// Since QOSGWidget sets Qt focusPolicy to StrongFocus, eventFilter() isn't
// needed for the keyboard events...at least this what I've observed on Linux;
// however it is essential to support resizing in embedded Qt windows that
// aren't a top-level window.
// 
// Addendum:  I think the above statement is true only if the parent has the
//            NoFocus policy?  If the parent also has StrongFocus, then
//            eventFilter() has to return false for key events or tabbing
//            breaks. 
//------------------------------------------------------------------------------
// Return false to allow event to go to the intended target, the parent of
// this QOSGWidget.  The only events we really want to go to the parent are
// TAB or shift-TAB presses and releases.
bool QOSGWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != parent())
    {
        return false;
    }
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab)
        {
            qDebug() << "QOSGWidget::eventFilter:  TAB Press on " 
                     << qPrintable(objectName());

            // Empirically have found that it's not necessary to call
            // keyPressEvent on tab press ... my guess is that OSG ignores it.
            keyPressEvent( ke );

            // Return false so that the parent QWidget will process the tab.
            return false;
        }
        else
        {
            qDebug() << "QOSGWidget::eventFilter:  KeyPress on " 
                     << qPrintable(objectName());
            keyPressEvent( ke );
            // event handled, return true because parent does not have to see
            // this event
            return true;
        }
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab)
        {
            qDebug() << "QOSGWidget::eventFilter:  TAB Release on " 
                     << qPrintable(objectName());

            keyReleaseEvent( ke );
            // Return false so that the parent QWidget will process the tab..
            return false;
        }
        else
        {
            qDebug() << "QOSGWidget::eventFilter:  KeyRelease on " 
                     << qPrintable(objectName());
            keyReleaseEvent( ke );
            // event handled, return true because parent does not have to see
            // this event
            return true;
        }
    }
    else if (event->type() == QEvent::Resize)
    {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);

        qDebug() << "QOSGWidget::eventFilter:  width is " 
                 << re->size().width() 
                 << "; height is " << re->size().height() 
                 << " on " << qPrintable(objectName())
            ;

        // Call setGeometry on 'this', which will trigger
        // QOSGWidget::resizeEvent  
        setGeometry(0, 0, re->size().width(), re->size().height());

        // event handled, return true because parent does not have to see
        // this event
        return true;
    }
    else if (event->type() == QEvent::Close)
    {
        QCloseEvent *ce = static_cast<QCloseEvent *>(event);
        closeEvent( ce );
    }
    
    return false;
}

// Skip all of the event queue reimplementations on WIN32.
// On second thought, all but the key events.
#ifndef WIN32

void QOSGWidget::destroyEvent(bool /* destroyWindow */, bool /* destroySubWindows */ )
{   
    qDebug() << "QOSGWidget::destroyEvent";
    _gw->getEventQueue()->closeWindow();
}


void QOSGWidget::closeEvent( QCloseEvent * /* event */ )
{
    qDebug() << "QOSGWidget::closeEvent";
    _gw->getEventQueue()->closeWindow();
}


void QOSGWidget::resizeEvent( QResizeEvent * event )
{
    const QSize & size = event->size();

    qDebug() << "QOSGWidget::resizeEvent on " << qPrintable(objectName())
             << " - width is " << size.width() 
             << "; height is " << size.height();

    _gw->getEventQueue()->windowResize(0, 0, size.width(), size.height() );
    _gw->resized(0, 0, size.width(), size.height());

}

void QOSGWidget::mousePressEvent( QMouseEvent* event )
{
    qDebug() << "QOSGWidget::mousePressEvent on " 
             << qPrintable(objectName()) << " at X, Y = " 
             << event->x() << ", " << event->y();
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

void QOSGWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
    qDebug() << "QOSGWidget::mouseDoubleClickEvent on " 
             << qPrintable(objectName());
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseDoubleButtonPress(event->x(), event->y(), button);
}
void QOSGWidget::mouseReleaseEvent( QMouseEvent* event )
{
    qDebug() << "QOSGWidget::mouseReleaseEvent on " 
             << qPrintable(objectName()) << " at X, Y = " 
             << event->x() << ", " << event->y();

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

void QOSGWidget::mouseMoveEvent( QMouseEvent* event )
{
    qDebug() << "QOSGWidget::mouseMoveEvent";
    _gw->getEventQueue()->mouseMotion(event->x(), event->y());
}

#endif // ndef WIN32

void QOSGWidget::keyPressEvent( QKeyEvent* event )
{
    qDebug() << "QOSGWidget::keyPressEvent on " << qPrintable(objectName());
    _gw->getEventQueue()->keyPress( 
        (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data() ) );
}

void QOSGWidget::keyReleaseEvent( QKeyEvent* event )
{
    qDebug() << "QOSGWidget::keyReleaseEvent on " << qPrintable(objectName());
    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab)
    {
        qDebug() << "---- It's a TAB Release" ;
    }
    else
    {
        int c = *event->text().toAscii().data();
        _gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) (c) );
    }
}

//---------------------------------------------------------------------------
// ViewQOSG creates the traits...
ViewQOSG::ViewQOSG( QWidget *parent )
    : osgViewer::View(),
      QOSGWidget( parent ),
      _daw(parent)
{

// OSG
    setGeometry( 0, 0, parent->width(), parent->height() );

    getCamera()->setGraphicsContext( getGraphicsWindow() );

    getCamera()->setProjectionMatrixAsPerspective(
        30.0f, 
        static_cast<double>(parent->width()) / 
        static_cast<double>(parent->height()), 1.0, 1000.0);

    getCamera()->setViewport(
        new osg::Viewport( 0, 0, parent->width(), parent->height()));

    setupManipulatorAndHandler( *this );

} // ViewQOSG::ViewQOSG

//---------------------------------------------------------------------------
// note:  since ViewQOSG has focusPolicy of Qt::NoFocus, this won't be called,
//        but leaving it here anyway.
//        See keyReleaseEvent()
void        ViewQOSG::focusInEvent( QFocusEvent * /* event */)
{
    qDebug() << "ViewQOSG::focusInEvent on " << qPrintable(objectName());

    CompositeViewerQOSG *cv = 
        dynamic_cast<CompositeViewerQOSG *>(getViewerBase());
    if (cv)
        // Tell the viewer that this view's camera should now have focus
        cv->setCameraWithFocus( getCamera() );
}

#ifndef WIN32 
//---------------------------------------------------------------------------
void ViewQOSG::resizeEvent( QResizeEvent * event )
{
     const QSize & size = event->size();

    qDebug() << "ViewQOSG::resizeEvent:  width is " 
             << size.width() << "; height is " << size.height();

    QOSGWidget::resizeEvent( event );

    // This call seems to be essential to getting a picture after view's
    // parent QWidget has been collapsed due to QSplitter activity.
    getCamera()->setProjectionMatrixAsPerspective(
        30.0f, 
        static_cast<double>(size.width()) / 
        static_cast<double>(size.height()), 1.0, 1000.0);

////
// This is an attempt to prevent Qt from giving tab focus to a ViewQOSG that
// has been collapsed; in our example, this can happen by using the QSplitters
// in the QMainWindow.
//
// However, while testing this, discovered the bigger problem: 
// On *any* resize, osgViewer::CompositeViewer::eventTraversal() calls
// setCameraWithFocus(0) (ie, to the first view).  
// Meanwhile, Qt's focus widget may or may not be the correpsonding ViewQOSG.
// Possibly worse, the ViewQOSG corresponding to the first view in the
// composite viewer may not even be visible.
// Doesn't seem to matter in this sense:  regardless of which VIEWQOSG gets a
// keyboard event, the compositeViewer applies the key's corresponding action
// to the current camera/view. 
//
// All of which suggests that trying to support TAB focus on the ViewQOSG
// widgets just isn't worth it. 

//    if (size.width() == 0 || size.height() == 0)
//        setFocusPolicy( Qt::NoFocus );
//    else
//        setFocusPolicy( Qt::StrongFocus );

////

} // ViewQOSG::resizeEvent
#endif // WIN32

//---------------------------------------------------------------------------
// Reimplementing keyReleaseEvent from QOSGWidget.
// When tabbing in Qt, the current widget sees the TAB Press and the widget
// that gets the focus sees the TAB Release.
// On that TAB Release, we need to tell the CompositeViewer the camera that
// should now have the focus.
//
// SEE COMMENTS IN ViewQOSG::resizeEvent re how resizing screws this all up...

void ViewQOSG::keyReleaseEvent( QKeyEvent* event )
{
    qDebug() << "ViewQOSG::keyReleaseEvent on " << qPrintable(objectName());

    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab)
    {
        if (event->key() == Qt::Key_Tab)
            qDebug() << "... and it's a TAB";
        else
            qDebug() << "... and it's a SHIFT-TAB";

        CompositeViewerQOSG *cv = 
            dynamic_cast<CompositeViewerQOSG *>(getViewerBase());
        if (cv)
        {
            // Tell the viewer that this view's camera should now have focus
            cv->setCameraWithFocus( getCamera() );

            // Note that the Stats come up wherever; having little success in
            // getting them to come up in a specific osgView.
//            cv->setEventQueue( _gw->getEventQueue() );
        }
        // and otherwise ignore the event
    }
    else
    {
        QOSGWidget::keyReleaseEvent( event );
    }

} // ViewQOSG::keyReleaseEvent

//---------------------------------------------------------------------------
void ViewQOSG::setData( osg::ref_ptr<osg::Node> loadedModel )
{
    setSceneData(loadedModel.get());
}

//---------------------------------------------------------------------------
float ViewQOSG::aspectRatio( int width, int height )
{
    return static_cast<float>(width) / static_cast<float>(height);
}

//---------------------------------------------------------------------------
void setupManipulatorAndHandler(osgViewer::View & view
                                /*, osg::ArgumentParser & arguments*/)
{
    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = 
            new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( 
            '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( 
            '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( 
            '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( 
            '4', "Terrain", new osgGA::TerrainManipulator() );

        view.setCameraManipulator( keyswitchManipulator.get() );
    }

    // add the state manipulator
    view.addEventHandler( new osgGA::StateSetManipulator(
                                view.getCamera()->getOrCreateStateSet()) );
    
    // add the thread model handler
    view.addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    view.addEventHandler(new osgViewer::WindowSizeHandler);
        
    // add the stats handler
    view.addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    view.addEventHandler(new osgViewer::HelpHandler(/*arguments.getApplicationUsage()*/));
}

