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
#ifndef _QOSG_WIDGET_HPP_
#define _QOSG_WIDGET_HPP_


#include <QtCore/QString>
//#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QApplication>

#include <QtGui/QWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QFocusEvent>

    
using Qt::WindowFlags;

// Port Note 10/14/08 LM -- I tried putting some of these headers into the
// source file, but ran into compile problems.  Very order dependent?
#include <osgViewer/View>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/GraphicsWindow>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgDB/ReadFile>

#include <iostream>

class QOSGWidget : public QWidget
{
public:

    QOSGWidget( QWidget * parent = 0, WindowFlags f = 0 );

    virtual ~QOSGWidget() {}

    osgViewer::GraphicsWindow* getGraphicsWindow() { 
        return _gw.get(); 
    }
    const osgViewer::GraphicsWindow* getGraphicsWindow() const { 
        return _gw.get(); 
    }

protected:

    void init();
    void createContext( QWidget *qwindow = 0 );
    bool eventFilter(QObject *obj, QEvent *event);

    // Looking at the doc for Qt::WA_PaintOnScreen this may be appropriate.
    // Didn't seem to help or hurt.
    virtual QPaintEngine *paintEngine() { return 0; }

// Grabbed this from Martin Beckett:
//  The GraphincsWindowWin32 implementation already takes care of message handling.
//  We don't want to relay these on Windows, it will just cause duplicate messages
//  with further problems downstream (i.e. not being able to throw the trackball

#ifndef WIN32
    virtual void mouseDoubleClickEvent ( QMouseEvent * event );
    virtual void closeEvent( QCloseEvent * event );
    virtual void destroyEvent( bool destroyWindow = true, 
                               bool destroySubWindows = true);
    virtual void resizeEvent( QResizeEvent * event );
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
#endif // ndef WIN32
    virtual void keyPressEvent( QKeyEvent* event );
    virtual void keyReleaseEvent( QKeyEvent* event );

    osg::ref_ptr<osgViewer::GraphicsWindow> _gw;
    bool _overrideTraits;

}; // QOSGWidget


//------------------------------------------------------------------------------
// I could get Linux to work in so many different ways.
//
// But I could get Windows to work in only one way:
//    1.  If QOSGWidget is constructed with a parent QWidget, the parent's
//        focusPolicy is set to StrongFocus.  The QOSGWidget widget 
//        will have NoFocus, the default for QWidget.
//
//    2.  If ViewQOSG is part of a osgViewer::CompositeViewer,
//        ViewQOSG::keyReleaseEvent(), on a TAB or Shift-TAB key release,
//        sets the Viewer's cameraWithFocus to the ViewQOSG's camera.
//        All other key releases are passed up to QOSGWidget::keyReleaseEvent.
//
//    3.  Since the QOSGWidget's focusPolicy is noFocus, and hence ViewQOSG's
//        is as well, ViewQOSG::focusInEvent() will never be called.
//
//    This was the only way I could get Windows to allow tabbing to change
//    which View of the CompositeViewer should have focus.
//    
//    Using StrongFocus on the QOSGWidget (and hence ViewQOSG) instead of its
//    parent caused all sorts of behavioral problems on Windows.  For example,
//    tabbing didn't work as expected.  Or it did, but it didn't matter
//    because the CompositeViewer considered the "current ViewQOSG" to be the
//    one under the mouse.
//    
//    The code we based this on created a class that inherited from
//    QOSGWidget and osgViewer::Viewer.  We instead inherit from 
//    QOSGWidget and osgViewer::View.  This allows us to post a different
//    scene graph in each view that the viewer manages.

class ViewQOSG : public osgViewer::View, public QOSGWidget
{
public:
    ViewQOSG( QWidget *parent /*, osg::GraphicsContext::Traits* traits*/ );

    virtual ~ViewQOSG() {}

// Reimplement from QWidget
#ifndef WIN32 
    virtual void resizeEvent( QResizeEvent * event );
#endif // ndef WIN32
    virtual void keyReleaseEvent( QKeyEvent* event );

    void setData( osg::ref_ptr<osg::Node> loadedModel );

    QWidget *         getDrawingAreaWidget()  { return _daw; }

    float aspectRatio( int width, int height );

protected:
    void        focusInEvent( QFocusEvent *event );

    QWidget *_daw;  // drawing area widget
    int _x, _y, _width, _height;

};


extern void setupManipulatorAndHandler(osgViewer::View & viewer
                                       /*, osg::ArgumentParser & arguments*/);
#endif // _QOSG_WIDGET_HPP_
