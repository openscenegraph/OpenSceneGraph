/* -*-c++-*- OpenSceneGraph - Copyright (C) 2009 Wang Rui
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

#include <osgQt/GraphicsWindowQt>

namespace osgQt
{

GraphWidget::GraphWidget( const QGLFormat& format, QWidget* parent, const QGLWidget* shareWidget, Qt::WindowFlags f )
: QGLWidget(format, parent, shareWidget, f)
{
    setAutoBufferSwap( false );
    setMouseTracking( true );
}

void GraphWidget::setKeyboardModifiers( QInputEvent* event )
{
    int modkey = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);
    unsigned int mask = 0;
    if ( modkey & Qt::ShiftModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
    if ( modkey & Qt::ControlModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
    if ( modkey & Qt::AltModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_ALT;
    _gw->getEventQueue()->getCurrentEventState()->setModKeyMask( mask );
}

void GraphWidget::resizeEvent( QResizeEvent* event )
{
    const QSize& size = event->size();
    _gw->getEventQueue()->windowResize( 0, 0, size.width(), size.height() );
    _gw->resized( 0, 0, size.width(), size.height() );
}

void GraphWidget::keyPressEvent( QKeyEvent* event )
{
    setKeyboardModifiers( event );
    _gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data()) );
}

void GraphWidget::keyReleaseEvent( QKeyEvent* event )
{
    setKeyboardModifiers( event );
    _gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data()) );
}

void GraphWidget::mousePressEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseButtonPress( event->x(), event->y(), button );
}

void GraphWidget::mouseReleaseEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseButtonRelease( event->x(), event->y(), button );
}

void GraphWidget::mouseDoubleClickEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseDoubleButtonPress( event->x(), event->y(), button );
}

void GraphWidget::mouseMoveEvent( QMouseEvent* event )
{
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseMotion( event->x(), event->y() );
}

void GraphWidget::wheelEvent( QWheelEvent* event )
{
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseScroll(
        event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN );
}

GraphicsWindowQt::GraphicsWindowQt( osg::GraphicsContext::Traits* traits )
:   _widget(0),
    _initialized(false),
    _realized(false)
{
    _traits = traits;
    _initialized = init();

    if ( valid() )
    {
        setState( new osg::State );
        getState()->setGraphicsContext(this);

        if ( _traits.valid() && _traits->sharedContext )
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
    }
}

GraphicsWindowQt::~GraphicsWindowQt()
{
    close();
}

bool GraphicsWindowQt::init()
{
    QGLFormat format( QGLFormat::defaultFormat() );
    format.setAlphaBufferSize( _traits->alpha );
    format.setRedBufferSize( _traits->red );
    format.setGreenBufferSize( _traits->green );
    format.setBlueBufferSize( _traits->blue );
    format.setDepthBufferSize( _traits->depth );
    format.setStencilBufferSize( _traits->stencil );
    format.setSampleBuffers( _traits->sampleBuffers );
    format.setSamples( _traits->samples );

    format.setAlpha( _traits->alpha>0 );
    format.setDepth( _traits->depth>0 );
    format.setStencil( _traits->stencil>0 );
    format.setDoubleBuffer( _traits->doubleBuffer );
    format.setSwapInterval( _traits->vsync ? 1 : 0 );
    format.setStereo( _traits->quadBufferStereo ? 1 : 0 );
    
    WindowData* windowData = _traits.get() ? dynamic_cast<WindowData*>(_traits->inheritedWindowData.get()) : 0;
    _widget = windowData ? windowData->_widget : 0;
    if ( !_widget )
    {
        GraphicsWindowQt* sharedContextQt = dynamic_cast<GraphicsWindowQt*>(_traits->sharedContext);
        QGLWidget* shareWidget = sharedContextQt ? sharedContextQt->getGraphWidget() : 0;
        
        Qt::WindowFlags flags = Qt::Window|Qt::CustomizeWindowHint;//|Qt::WindowStaysOnTopHint;
        if ( _traits->windowDecoration )
            flags |= Qt::WindowTitleHint|Qt::WindowMinMaxButtonsHint|Qt::WindowSystemMenuHint;
        
        _widget = new GraphWidget( format, 0, shareWidget, flags );
    }
    
    _widget->setWindowTitle( _traits->windowName.c_str() );
    _widget->move( _traits->x, _traits->y );
    if ( !_traits->supportsResize ) _widget->setFixedSize( _traits->width, _traits->height );
    else _widget->resize( _traits->width, _traits->height );
    
    _widget->setFocusPolicy( Qt::WheelFocus );
    _widget->setGraphicsWindow( this );
    useCursor( _traits->useCursor );
    return true;
}

bool GraphicsWindowQt::setWindowRectangleImplementation( int x, int y, int width, int height )
{
    if ( _widget ) _widget->setGeometry( x, y, width, height );
    return _widget!=NULL;
}

void GraphicsWindowQt::getWindowRectangle( int& x, int& y, int& width, int& height )
{
    if ( _widget )
    {
        const QRect& geom = _widget->geometry();
        x = geom.x();
        y = geom.y();
        width = geom.width();
        height = geom.height();
    }
}

bool GraphicsWindowQt::setWindowDecorationImplementation( bool windowDecoration )
{
    Qt::WindowFlags flags = Qt::Window|Qt::CustomizeWindowHint;//|Qt::WindowStaysOnTopHint;
    if ( windowDecoration )
        flags |= Qt::WindowTitleHint|Qt::WindowMinMaxButtonsHint|Qt::WindowSystemMenuHint;
    _traits->windowDecoration = windowDecoration;
    
    // FIXME: Calling setWindowFlags or reparent widget will recreate the window handle,
    // which makes QGLContext no longer work...How to deal with that?
    //if ( _widget ) _widget->setWindowFlags( flags );
    return false;
}

bool GraphicsWindowQt::getWindowDecoration() const
{
    return _traits->windowDecoration;
}

void GraphicsWindowQt::grabFocus()
{
    if ( _widget )
        _widget->setFocus( Qt::ActiveWindowFocusReason );
}

void GraphicsWindowQt::grabFocusIfPointerInWindow()
{
    if ( _widget->underMouse() )
        _widget->setFocus( Qt::ActiveWindowFocusReason );
}

void GraphicsWindowQt::raiseWindow()
{
    if ( _widget )
        _widget->raise();
}

void GraphicsWindowQt::setWindowName( const std::string& name )
{
    if ( _widget )
        _widget->setWindowTitle( name.c_str() );
}

std::string GraphicsWindowQt::getWindowName()
{
    return _widget ? _widget->windowTitle().toStdString() : "";
}

void GraphicsWindowQt::useCursor( bool cursorOn )
{
    if ( _widget )
    {
        _traits->useCursor = cursorOn;
        if ( !cursorOn ) _widget->setCursor( Qt::BlankCursor );
        else _widget->setCursor( _currentCursor );
    }
}

void GraphicsWindowQt::setCursor( MouseCursor cursor )
{
    if ( cursor==InheritCursor && _widget )
    {
        _widget->unsetCursor();
    }

    switch ( cursor )
    {
    case NoCursor: _currentCursor = Qt::BlankCursor; break;
    case RightArrowCursor: case LeftArrowCursor: _currentCursor = Qt::ArrowCursor; break;
    case InfoCursor: _currentCursor = Qt::SizeAllCursor; break;
    case DestroyCursor: _currentCursor = Qt::ForbiddenCursor; break;
    case HelpCursor: _currentCursor = Qt::WhatsThisCursor; break;
    case CycleCursor: _currentCursor = Qt::ForbiddenCursor; break;
    case SprayCursor: _currentCursor = Qt::SizeAllCursor; break;
    case WaitCursor: _currentCursor = Qt::WaitCursor; break;
    case TextCursor: _currentCursor = Qt::IBeamCursor; break;
    case CrosshairCursor: _currentCursor = Qt::CrossCursor; break;
    case HandCursor: _currentCursor = Qt::OpenHandCursor; break;
    case UpDownCursor: _currentCursor = Qt::SizeVerCursor; break;
    case LeftRightCursor: _currentCursor = Qt::SizeHorCursor; break;
    case TopSideCursor: case BottomSideCursor: _currentCursor = Qt::UpArrowCursor; break;
    case LeftSideCursor: case RightSideCursor: _currentCursor = Qt::SizeHorCursor; break;
    case TopLeftCorner: _currentCursor = Qt::SizeBDiagCursor; break;
    case TopRightCorner: _currentCursor = Qt::SizeFDiagCursor; break;
    case BottomRightCorner: _currentCursor = Qt::SizeBDiagCursor; break;
    case BottomLeftCorner: _currentCursor = Qt::SizeFDiagCursor; break;
    default: break;
    };
    if ( _widget ) _widget->setCursor( _currentCursor );
}

bool GraphicsWindowQt::valid() const
{
    return _widget && _widget->isValid();
}

bool GraphicsWindowQt::realizeImplementation()
{
    if ( !_initialized )
        _initialized = init();

    // A makeCurrent()/doneCurrent() seems to be required for
    // realizing the context(?) before starting drawing
    _widget->makeCurrent();
    _widget->doneCurrent();

    _realized = true;
    return true;
}

bool GraphicsWindowQt::isRealizedImplementation() const
{
    return _realized;
}

void GraphicsWindowQt::closeImplementation()
{
    if ( _widget )
        _widget->close();
}

bool GraphicsWindowQt::makeCurrentImplementation()
{
    _widget->makeCurrent();
    return true;
}

bool GraphicsWindowQt::releaseContextImplementation()
{
    _widget->doneCurrent();
    return true;
}

void GraphicsWindowQt::swapBuffersImplementation()
{
    _widget->swapBuffers();
}

void GraphicsWindowQt::requestWarpPointer( float x, float y )
{
    if ( _widget )
        QCursor::setPos( _widget->mapToGlobal(QPoint((int)x,(int)y)) );
}

}
