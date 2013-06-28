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

#include <osgQt/QWidgetImage>
#include <QLayout>

namespace osgQt
{

QWidgetImage::QWidgetImage( QWidget* widget )
{
    // make sure we have a valid QApplication before we start creating widgets.
    getOrCreateQApplication();

    _widget = widget;
    _adapter = new QGraphicsViewAdapter(this, _widget.data());
}

bool QWidgetImage::sendFocusHint(bool focus)
{
    QFocusEvent event(focus ? QEvent::FocusIn : QEvent::FocusOut, Qt::OtherFocusReason);
    QCoreApplication::sendEvent(_widget, &event);
    return true;
}

void QWidgetImage::clearWriteBuffer()
{
    _adapter->clearWriteBuffer();
}

void QWidgetImage::render()
{
    _adapter->render();
}

void QWidgetImage::scaleImage(int s,int t,int /*r*/, GLenum /*newDataType*/)
{
    _adapter->resize(s, t);
}

void QWidgetImage::setFrameLastRendered(const osg::FrameStamp* frameStamp)
{
    _adapter->setFrameLastRendered(frameStamp);
}

bool QWidgetImage::sendPointerEvent(int x, int y, int buttonMask)
{
    return _adapter->sendPointerEvent(x,y,buttonMask);
}

bool QWidgetImage::sendKeyEvent(int key, bool keyDown)
{
    return _adapter->sendKeyEvent(key, keyDown);
}

}
