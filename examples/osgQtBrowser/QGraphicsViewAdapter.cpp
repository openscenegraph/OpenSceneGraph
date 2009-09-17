/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include "QGraphicsViewAdapter.h"
#include "QWebViewImage.h"

#include <QtOpenGL/QGLWidget>

#include <osgGA/GUIEventAdapter>

#include <osg/io_utils>

#define MYQKEYEVENT 2000
#define MYQPOINTEREVENT 2001

QCoreApplication* getOrCreateQApplication()
{
    if (QApplication::instance()==0)
    {
        static char** argv = 0;
        static int argc = 0;
        static QApplication app(argc,argv);
    }
    return QApplication::instance();
}

class MyQKeyEvent : public QEvent
{
public:
    MyQKeyEvent( int key, bool down ):
        QEvent( QEvent::Type(MYQKEYEVENT) ),
        _key(key), _down(down) {}

    int         _key;
    bool        _down;
};

struct MyQPointerEvent : public QEvent
{
    MyQPointerEvent(int x, int y, unsigned int buttonMask):
        QEvent( QEvent::Type(MYQPOINTEREVENT) ),
        _x(x), _y(y),_buttonMask(buttonMask) {}

    int _x, _y;
    unsigned int _buttonMask;
};

QGraphicsViewAdapter::QGraphicsViewAdapter(osg::Image* image, QWidget* widget):
    _image(image),
    _backgroundColor(255,255,255),
    _qtKeyModifiers(Qt::NoModifier)
{
    // make sure we have a valid QApplication before we start creating widgets.
    getOrCreateQApplication();


    setUpKeyMap();

    _graphicsScene = new QGraphicsScene();
    _graphicsView = new QGraphicsView;
    _graphicsScene->addWidget(widget);
    _graphicsView->setScene(_graphicsScene);
    _graphicsScene->setStickyFocus(true);
    _graphicsView->viewport()->setParent(0);

    int width = _graphicsScene->width();
    int height = _graphicsScene->height();

    _qimages[0] = QImage(QSize(width, height), QImage::Format_ARGB32);
    _qimages[0].fill(_backgroundColor.rgba());
    _qimages[0] = QGLWidget::convertToGLFormat(_qimages[0]);

    _qimages[1] = QImage(QSize(width, height), QImage::Format_ARGB32);
    _qimages[1].fill(_backgroundColor.rgba());

    _qimages[2] = QImage(QSize(width, height), QImage::Format_ARGB32);
    _qimages[2].fill(_backgroundColor.rgba());

    _currentRead = 0;
    _currentWrite = 1;
    _previousWrite = 2;
    _previousFrameNumber = 0;
    _newImageAvailable = false;

    connect(_graphicsScene, SIGNAL(changed(const QList<QRectF> &)),
            this, SLOT(repaintRequestedSlot(const QList<QRectF> &)));

    assignImage(0);
}

void QGraphicsViewAdapter::repaintRequestedSlot(const QList<QRectF>&)
{
    // osg::notify(osg::NOTICE)<<"QGraphicsViewAdapter::repaintRequestedSlot"<<std::endl;
    render();
}

void QGraphicsViewAdapter::customEvent ( QEvent * event )
{
    if (event->type()==MYQKEYEVENT)
    {
        MyQKeyEvent* keyEvent = (MyQKeyEvent*)event;
        handleKeyEvent(keyEvent->_key, keyEvent->_down);
    }
    else if (event->type()==MYQPOINTEREVENT)
    {
        MyQPointerEvent* pointerEvent = (MyQPointerEvent*)event;
        handlePointerEvent(pointerEvent->_x, pointerEvent->_y, pointerEvent->_buttonMask);
    }
}


void QGraphicsViewAdapter::setUpKeyMap()
{
    _keyMap[osgGA::GUIEventAdapter::KEY_BackSpace] = Qt::Key_Backspace;
    _keyMap[osgGA::GUIEventAdapter::KEY_Tab] = Qt::Key_Tab;
    _keyMap[osgGA::GUIEventAdapter::KEY_Linefeed] = Qt::Key_Return; // No LineFeed in Qt!
    _keyMap[osgGA::GUIEventAdapter::KEY_Clear] = Qt::Key_Clear;
    _keyMap[osgGA::GUIEventAdapter::KEY_Return] = Qt::Key_Return;
    _keyMap[osgGA::GUIEventAdapter::KEY_Pause] = Qt::Key_Pause;
    _keyMap[osgGA::GUIEventAdapter::KEY_Scroll_Lock] = Qt::Key_ScrollLock;
    _keyMap[osgGA::GUIEventAdapter::KEY_Sys_Req] = Qt::Key_SysReq;
    _keyMap[osgGA::GUIEventAdapter::KEY_Escape] = Qt::Key_Escape;
    _keyMap[osgGA::GUIEventAdapter::KEY_Delete] = Qt::Key_Delete;

    _keyMap[osgGA::GUIEventAdapter::KEY_Home] = Qt::Key_Home;
    _keyMap[osgGA::GUIEventAdapter::KEY_Left] = Qt::Key_Left;
    _keyMap[osgGA::GUIEventAdapter::KEY_Up] = Qt::Key_Up;
    _keyMap[osgGA::GUIEventAdapter::KEY_Right] = Qt::Key_Right;
    _keyMap[osgGA::GUIEventAdapter::KEY_Down] = Qt::Key_Down;
    _keyMap[osgGA::GUIEventAdapter::KEY_Prior] = Qt::Key_Left; // no Prior in Qt
    _keyMap[osgGA::GUIEventAdapter::KEY_Page_Up] = Qt::Key_PageUp;
    _keyMap[osgGA::GUIEventAdapter::KEY_Next] = Qt::Key_Right; // No Next in Qt
    _keyMap[osgGA::GUIEventAdapter::KEY_Page_Down] = Qt::Key_PageDown;
    _keyMap[osgGA::GUIEventAdapter::KEY_End] = Qt::Key_End;
    _keyMap[osgGA::GUIEventAdapter::KEY_Begin] = Qt::Key_Home; // No Begin in Qt

    _keyMap[osgGA::GUIEventAdapter::KEY_Select] = Qt::Key_Select;
    _keyMap[osgGA::GUIEventAdapter::KEY_Print] = Qt::Key_Print;
    _keyMap[osgGA::GUIEventAdapter::KEY_Execute] = Qt::Key_Execute;
    _keyMap[osgGA::GUIEventAdapter::KEY_Insert] = Qt::Key_Insert;
    //_keyMap[osgGA::GUIEventAdapter::KEY_Undo] = Qt::Key_; // no Undo
    //_keyMap[osgGA::GUIEventAdapter::KEY_Redo] = Qt::Key_; // no Redo
    _keyMap[osgGA::GUIEventAdapter::KEY_Menu] = Qt::Key_Menu;
    _keyMap[osgGA::GUIEventAdapter::KEY_Find] = Qt::Key_Search; // no Qt Find
    _keyMap[osgGA::GUIEventAdapter::KEY_Cancel] = Qt::Key_Cancel;
    _keyMap[osgGA::GUIEventAdapter::KEY_Help] = Qt::Key_Help;
    _keyMap[osgGA::GUIEventAdapter::KEY_Break] = Qt::Key_Escape; // no break
    _keyMap[osgGA::GUIEventAdapter::KEY_Mode_switch] = Qt::Key_Mode_switch;
    _keyMap[osgGA::GUIEventAdapter::KEY_Script_switch] = Qt::Key_Mode_switch; // no Script switch
    _keyMap[osgGA::GUIEventAdapter::KEY_Num_Lock] = Qt::Key_NumLock;

    _keyMap[osgGA::GUIEventAdapter::KEY_Shift_L] = Qt::Key_Shift;
    _keyMap[osgGA::GUIEventAdapter::KEY_Shift_R] = Qt::Key_Shift;
    _keyMap[osgGA::GUIEventAdapter::KEY_Control_L] = Qt::Key_Control;
    _keyMap[osgGA::GUIEventAdapter::KEY_Control_R] = Qt::Key_Control;
    _keyMap[osgGA::GUIEventAdapter::KEY_Caps_Lock] = Qt::Key_CapsLock;
    _keyMap[osgGA::GUIEventAdapter::KEY_Shift_Lock] = Qt::Key_CapsLock;

    _keyMap[osgGA::GUIEventAdapter::KEY_Meta_L] = Qt::Key_Meta; // Qt doesn't have a Meta L
    _keyMap[osgGA::GUIEventAdapter::KEY_Meta_R] = Qt::Key_Meta; // Qt doesn't have a Meta R
    _keyMap[osgGA::GUIEventAdapter::KEY_Alt_L] = Qt::Key_Alt; // Qt doesn't have a Alt L
    _keyMap[osgGA::GUIEventAdapter::KEY_Alt_R] = Qt::Key_Alt; // Qt doesn't have a Alt R
    _keyMap[osgGA::GUIEventAdapter::KEY_Super_L] = Qt::Key_Super_L;
    _keyMap[osgGA::GUIEventAdapter::KEY_Super_R] = Qt::Key_Super_R;
    _keyMap[osgGA::GUIEventAdapter::KEY_Hyper_L] = Qt::Key_Hyper_L;
    _keyMap[osgGA::GUIEventAdapter::KEY_Hyper_R] = Qt::Key_Hyper_R;

    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Space] = Qt::Key_Space;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Tab] = Qt::Key_Tab;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Enter] = Qt::Key_Enter;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_F1] = Qt::Key_F1;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_F2] = Qt::Key_F2;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_F3] = Qt::Key_F3;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_F4] = Qt::Key_F4;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Home] = Qt::Key_Home;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Left] = Qt::Key_Left;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Up] = Qt::Key_Up;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Right] = Qt::Key_Right;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Down] = Qt::Key_Down;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Prior] = Qt::Key_Left;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Page_Up] = Qt::Key_PageUp;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Next] = Qt::Key_Right;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Page_Down] = Qt::Key_PageDown;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_End] = Qt::Key_End;

    // _keyMap[osgGA::GUIEventAdapter::KEY_KP_Begin] = Qt::Key_Begin;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Insert] = Qt::Key_Insert;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Delete] = Qt::Key_Delete;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Equal] = Qt::Key_Equal;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Multiply] = Qt::Key_Asterisk;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Add] = Qt::Key_Plus;
    //_keyMap[osgGA::GUIEventAdapter::KEY_KP_Separator] = Qt::Key_;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Subtract] = Qt::Key_Minus;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Decimal] = Qt::Key_Period;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Divide] = Qt::Key_division;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_0] = Qt::Key_0;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_1] = Qt::Key_1;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_2] = Qt::Key_2;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_3] = Qt::Key_3;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_4] = Qt::Key_4;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_5] = Qt::Key_5;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_6] = Qt::Key_6;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_7] = Qt::Key_7;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_8] = Qt::Key_8;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_9] = Qt::Key_9;

    _keyMap[osgGA::GUIEventAdapter::KEY_F1] = Qt::Key_F1;
    _keyMap[osgGA::GUIEventAdapter::KEY_F2] = Qt::Key_F2;
    _keyMap[osgGA::GUIEventAdapter::KEY_F3] = Qt::Key_F3;
    _keyMap[osgGA::GUIEventAdapter::KEY_F4] = Qt::Key_F4;
    _keyMap[osgGA::GUIEventAdapter::KEY_F5] = Qt::Key_F5;
    _keyMap[osgGA::GUIEventAdapter::KEY_F6] = Qt::Key_F6;
    _keyMap[osgGA::GUIEventAdapter::KEY_F7] = Qt::Key_F7;
    _keyMap[osgGA::GUIEventAdapter::KEY_F8] = Qt::Key_F8;
    _keyMap[osgGA::GUIEventAdapter::KEY_F9] = Qt::Key_F9;
    _keyMap[osgGA::GUIEventAdapter::KEY_F10] = Qt::Key_F10;
    _keyMap[osgGA::GUIEventAdapter::KEY_F11] = Qt::Key_F11;
    _keyMap[osgGA::GUIEventAdapter::KEY_F12] = Qt::Key_F12;
    _keyMap[osgGA::GUIEventAdapter::KEY_F13] = Qt::Key_F13;
    _keyMap[osgGA::GUIEventAdapter::KEY_F14] = Qt::Key_F14;
    _keyMap[osgGA::GUIEventAdapter::KEY_F15] = Qt::Key_F15;
    _keyMap[osgGA::GUIEventAdapter::KEY_F16] = Qt::Key_F16;
    _keyMap[osgGA::GUIEventAdapter::KEY_F17] = Qt::Key_F17;
    _keyMap[osgGA::GUIEventAdapter::KEY_F18] = Qt::Key_F18;
    _keyMap[osgGA::GUIEventAdapter::KEY_F19] = Qt::Key_F19;
    _keyMap[osgGA::GUIEventAdapter::KEY_F20] = Qt::Key_F20;
    _keyMap[osgGA::GUIEventAdapter::KEY_F21] = Qt::Key_F21;
    _keyMap[osgGA::GUIEventAdapter::KEY_F22] = Qt::Key_F22;
    _keyMap[osgGA::GUIEventAdapter::KEY_F23] = Qt::Key_F23;
    _keyMap[osgGA::GUIEventAdapter::KEY_F24] = Qt::Key_F24;
    _keyMap[osgGA::GUIEventAdapter::KEY_F25] = Qt::Key_F25;
    _keyMap[osgGA::GUIEventAdapter::KEY_F26] = Qt::Key_F26;
    _keyMap[osgGA::GUIEventAdapter::KEY_F27] = Qt::Key_F27;
    _keyMap[osgGA::GUIEventAdapter::KEY_F28] = Qt::Key_F28;
    _keyMap[osgGA::GUIEventAdapter::KEY_F29] = Qt::Key_F29;
    _keyMap[osgGA::GUIEventAdapter::KEY_F30] = Qt::Key_F30;
    _keyMap[osgGA::GUIEventAdapter::KEY_F31] = Qt::Key_F31;
    _keyMap[osgGA::GUIEventAdapter::KEY_F32] = Qt::Key_F32;
    _keyMap[osgGA::GUIEventAdapter::KEY_F33] = Qt::Key_F33;
    _keyMap[osgGA::GUIEventAdapter::KEY_F34] = Qt::Key_F34;
    _keyMap[osgGA::GUIEventAdapter::KEY_F35] = Qt::Key_F35;

}

bool QGraphicsViewAdapter::sendPointerEvent(int x, int y, int buttonMask)
{
    QCoreApplication::postEvent(this, new MyQPointerEvent(x,y,buttonMask));
    return true;
}

bool QGraphicsViewAdapter::handlePointerEvent(int x, int y, int buttonMask)
{
    osg::notify(osg::INFO)<<"dispatchPointerEvent("<<x<<", "<<y<<", "<<buttonMask<<")"<<std::endl;

    y = _graphicsScene->height()-y;

    bool leftButtonPressed = (buttonMask & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)!=0;
    bool middleButtonPressed = (buttonMask & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)!=0;
    bool rightButtonPressed = (buttonMask & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)!=0;

    bool prev_leftButtonPressed = (_previousButtonMask & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)!=0;
    bool prev_middleButtonPressed = (_previousButtonMask & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)!=0;
    bool prev_rightButtonPressed = (_previousButtonMask & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)!=0;

    osg::notify(osg::INFO)<<"leftButtonPressed "<<leftButtonPressed<<std::endl;
    osg::notify(osg::INFO)<<"middleButtonPressed "<<middleButtonPressed<<std::endl;
    osg::notify(osg::INFO)<<"rightButtonPressed "<<rightButtonPressed<<std::endl;

    Qt::MouseButtons qtMouseButtons =
        (leftButtonPressed ? Qt::LeftButton : Qt::NoButton) |
        (middleButtonPressed ? Qt::MidButton : Qt::NoButton) |
        (rightButtonPressed ? Qt::RightButton : Qt::NoButton);

    if (buttonMask != _previousButtonMask)
    {

        Qt::MouseButton qtButton = Qt::NoButton;
        QEvent::Type eventType = QEvent::None;
        if (leftButtonPressed != prev_leftButtonPressed)
        {
            qtButton = Qt::LeftButton;
            eventType = leftButtonPressed ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease ;
        }
        else if (middleButtonPressed != prev_middleButtonPressed)
        {
            qtButton = Qt::MidButton;
            eventType = middleButtonPressed ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease ;
        }
        else if (rightButtonPressed != prev_rightButtonPressed)
        {
            qtButton = Qt::RightButton;
            eventType = rightButtonPressed ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease ;
        }

        if (eventType==QEvent::MouseButtonPress)
        {
            QWebViewImage* qwebViewImage = dynamic_cast<QWebViewImage*>(_image.get());
            if (qwebViewImage) qwebViewImage->focusBrowser(true);
        }

        QMouseEvent event(eventType, QPoint(x, y), qtButton, qtMouseButtons, 0);
        QCoreApplication::sendEvent(_graphicsView->viewport(), &event );

        _previousButtonMask = buttonMask;
    }
    else if (x != _previousMouseX || y != _previousMouseY)
    {

        QMouseEvent event(QEvent::MouseMove, QPoint(x, y), Qt::NoButton, qtMouseButtons, 0);
        QCoreApplication::sendEvent(_graphicsView->viewport(), &event);

        _previousMouseX = x;
        _previousMouseY = y;
    }

    return true;
}

bool QGraphicsViewAdapter::sendKeyEvent(int key, bool keyDown)
{
    QCoreApplication::postEvent(this, new MyQKeyEvent(key,keyDown));
    return true;
}

bool QGraphicsViewAdapter::handleKeyEvent(int key, bool keyDown)
{
    QEvent::Type eventType = keyDown ? QEvent::KeyPress : QEvent::KeyRelease;

    osg::notify(osg::INFO)<<"sendKeyEvent("<<key<<", "<<keyDown<<")"<<std::endl;

    if (key==osgGA::GUIEventAdapter::KEY_Shift_L || key==osgGA::GUIEventAdapter::KEY_Shift_R)
    {
        _qtKeyModifiers = (_qtKeyModifiers & ~Qt::ShiftModifier) | (keyDown ? Qt::ShiftModifier : Qt::NoModifier);
    }

    if (key==osgGA::GUIEventAdapter::KEY_Control_L || key==osgGA::GUIEventAdapter::KEY_Control_R)
    {
        _qtKeyModifiers = (_qtKeyModifiers & ~Qt::ControlModifier) | (keyDown ? Qt::ControlModifier : Qt::NoModifier);
    }

    if (key==osgGA::GUIEventAdapter::KEY_Alt_L || key==osgGA::GUIEventAdapter::KEY_Alt_R)
    {
        _qtKeyModifiers = (_qtKeyModifiers & ~Qt::ControlModifier) | (keyDown ? Qt::ControlModifier : Qt::NoModifier);
    }

    if (key==osgGA::GUIEventAdapter::KEY_Meta_L || key==osgGA::GUIEventAdapter::KEY_Meta_R)
    {
        _qtKeyModifiers = (_qtKeyModifiers & ~Qt::MetaModifier) | (keyDown ? Qt::MetaModifier : Qt::NoModifier);
    }

    Qt::Key qtkey;
    QChar input;

    KeyMap::iterator itr = _keyMap.find(key);
    if (itr != _keyMap.end())
    {
        qtkey = itr->second;
    }
    else
    {
        qtkey = (Qt::Key)key;
        input = QChar(key);
    }

    QKeyEvent event(eventType, qtkey, _qtKeyModifiers, input);
    QCoreApplication::sendEvent(_graphicsScene.data(), &event);
    return true;
}

void QGraphicsViewAdapter::setFrameLastRendered(const osg::FrameStamp* frameStamp)
{
    osg::notify(osg::INFO)<<"setFrameLastRendered("<<frameStamp->getFrameNumber()<<")"<<std::endl;

    if (_newImageAvailable && _previousFrameNumber!=frameStamp->getFrameNumber())
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_qimagesMutex);

            // make sure that _previousFrameNumber hasn't been updated by another thread since we entered this branch.
            if (_previousFrameNumber==frameStamp->getFrameNumber()) return;
            _previousFrameNumber = frameStamp->getFrameNumber();

            std::swap(_currentRead, _previousWrite);
            _newImageAvailable = false;
        }

        assignImage(_currentRead);
    }
}

void QGraphicsViewAdapter::clearWriteBuffer()
{
    QImage& image = _qimages[_currentWrite];
    image.fill(_backgroundColor.rgba ());
    image = QGLWidget::convertToGLFormat(image);

    // swap the write buffers in a thread safe way
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_qimagesMutex);
    std::swap(_currentWrite, _previousWrite);
    _newImageAvailable = true;
}

void QGraphicsViewAdapter::render()
{
    osg::notify(osg::INFO)<<"Current write = "<<_currentWrite<<std::endl;
    QImage& image = _qimages[_currentWrite];

#if 1
    // paint the image with the graphics view
    QPainter painter(&image);
    QRectF destinationRect(0, 0, image.width(), image.height());
    QRect sourceRect(0, 0, _graphicsScene->width(), _graphicsScene->height());
    _graphicsView->render(&painter, destinationRect, sourceRect);
    painter.end();
#else
    // paint the image with the graphics view
    QPixmap pixmap(image.width(), image.height());
    QPainter painter(&pixmap);
    QRectF destinationRect(0, 0, image.width(), image.height());
    QRect sourceRect(0, 0, _graphicsScene->width(), _graphicsScene->height());
    _graphicsView->render(&painter, destinationRect, sourceRect);
    painter.end();

    image = pixmap.toImage();
#endif

    // convert into OpenGL format - flipping around the Y axis and swizzling the pixels
    image = QGLWidget::convertToGLFormat(image);

    // swap the write buffers in a thread safe way
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_qimagesMutex);
    std::swap(_currentWrite, _previousWrite);
    _newImageAvailable = true;
}

void QGraphicsViewAdapter::assignImage(unsigned int i)
{
    QImage& image = _qimages[i];
    unsigned char* data = image.bits();

    osg::notify(osg::INFO)<<"assigImage("<<i<<") image = "<<&image<<" data = "<<(void*)data<<std::endl;

    _image->setImage(image.width(),image.height(),1,
                4,
                GL_RGBA,GL_UNSIGNED_BYTE,
                data,
                osg::Image::NO_DELETE, 1);
}