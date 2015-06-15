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

// make sure this header isn't built as par of osgQt, leaving it to applications to build
#if !defined(OSGQT_LIBRARY) && !defined(OSG_LIBRARY_STATIC)

#if QT_VERSION >= 0x050000
# include <QtWebKitWidgets>
#else
# include <QtWebKit>
#endif


#include <osgWidget/Browser>
#include <osgQt/QGraphicsViewAdapter>
#include <osgQt/Version>

namespace osgQt
{

class QWebViewImage : public osgWidget::BrowserImage
{
    public:

        QWebViewImage()
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

        virtual void navigateTo(const std::string& url)
        {
            _webView->load(QUrl(url.c_str()));
        }

        QWebView* getQWebView() { return _webView; }
        QWebPage* getQWebPage() { return _webPage; }
        QGraphicsViewAdapter* getQGraphicsViewAdapter() { return _adapter; }

        void clearWriteBuffer()
        {
            _adapter->clearWriteBuffer();
        }

        void render()
        {
            if (_adapter->requiresRendering()) _adapter->render();
        }

        virtual bool requiresUpdateCall() const { return true; }
        virtual void update( osg::NodeVisitor* nv ) { render(); }
        
        virtual bool sendFocusHint(bool focus)
        {
            QFocusEvent event(focus ? QEvent::FocusIn : QEvent::FocusOut, Qt::OtherFocusReason);
            QCoreApplication::sendEvent(_webPage, &event);
            return true;
        }

        virtual bool sendPointerEvent(int x, int y, int buttonMask)
        {
            return _adapter->sendPointerEvent(x,y,buttonMask);
        }

        virtual bool sendKeyEvent(int key, bool keyDown)
        {
            return QWebViewImage::_adapter->sendKeyEvent(key, keyDown);
        }

        virtual void setFrameLastRendered(const osg::FrameStamp* frameStamp)
        {
            _adapter->setFrameLastRendered(frameStamp);
        }

    protected:

        QPointer<QGraphicsViewAdapter>  _adapter;
        QPointer<QWebView>              _webView;
        QPointer<QWebPage>              _webPage;
};

}

#endif

#endif
