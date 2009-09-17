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

#ifndef QWEBVIEWIMAGE
#define QWEBVIEWIMAGE

#include <osgWidget/Browser>
#include "QGraphicsViewAdapter.h"

class QWebViewImage : public osgWidget::BrowserImage
{
    public:

        QWebViewImage();

        virtual void navigateTo(const std::string& url);

        QWebView* getQWebView() { return _webView; }
        QWebPage* getQWebPage() { return _webPage; }
        QGraphicsViewAdapter* getQGraphicsViewAdapter() { return _adapter; }

        void focusBrowser(bool focus);

        void clearWriteBuffer();

        void render();

        virtual void setFrameLastRendered(const osg::FrameStamp* frameStamp);

        virtual bool sendPointerEvent(int x, int y, int buttonMask);

        virtual bool sendKeyEvent(int key, bool keyDown);

    protected:

        QPointer<QGraphicsViewAdapter>  _adapter;
        QPointer<QWebView>              _webView;
        QPointer<QWebPage>              _webPage;
};

#endif