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

#include "QWebViewImage.h"

QWebViewImage::QWebViewImage()
{
    // make sure we have a valid QApplication before we start creating widgets.
    getOrCreateQApplication();

    _webView = new QWebView;

    _webPage = new QWebPage;
    _webPage->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    _webPage->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    _webView->setPage(_webPage);

    _adapter = new QGraphicsViewAdapter(this, _webView.data());
}

void QWebViewImage::navigateTo(const std::string& url)
{
    _webView->load(QUrl(url.c_str()));
}

void QWebViewImage::focusBrowser(bool focus)
{
    QFocusEvent event(focus ? QEvent::FocusIn : QEvent::FocusOut, Qt::OtherFocusReason);
    QCoreApplication::sendEvent(_webPage, &event);
}

void QWebViewImage::clearWriteBuffer()
{
    _adapter->clearWriteBuffer();
}

void QWebViewImage::render()
{
    _adapter->render();
}

void QWebViewImage::setFrameLastRendered(const osg::FrameStamp* frameStamp)
{
    _adapter->setFrameLastRendered(frameStamp);
}

bool QWebViewImage::sendPointerEvent(int x, int y, int buttonMask)
{
    return _adapter->sendPointerEvent(x,y,buttonMask);
}

bool QWebViewImage::sendKeyEvent(int key, bool keyDown)
{
    return QWebViewImage::_adapter->sendKeyEvent(key, keyDown);
}

