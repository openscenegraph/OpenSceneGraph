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

#ifndef QGRAPHICSVIEWADAPTER
#define QGRAPHICSVIEWADAPTER

#include <QGLWidget>

#include <osg/Image>
#include <osg/observer_ptr>
#include <osgQt/Export>

#include <QPointer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QApplication>
#include <QPainter>
#include <QtEvents>

namespace osgQt
{

extern OSGQT_EXPORT QCoreApplication* getOrCreateQApplication();

class OSGQT_EXPORT QGraphicsViewAdapter : public QObject
{
    Q_OBJECT

    public:

        QGraphicsViewAdapter(osg::Image* image, QWidget* widget);

        void setUpKeyMap();

        bool sendPointerEvent(int x, int y, int buttonMask);


        bool sendKeyEvent(int key, bool keyDown);


        void setFrameLastRendered(const osg::FrameStamp* frameStamp);

        void clearWriteBuffer();

        bool requiresRendering() const { return _requiresRendering; }
        void render();

        void assignImage(unsigned int i);

        void resize(int width, int height);

        void setBackgroundColor(QColor color) { _backgroundColor = color; }
        QColor getBackgroundColor() const     { return _backgroundColor; }

        /** The 'background widget' will ignore mouse/keyboard events and let following handlers handle them
            It is mainly used for integrating scene graph and full-screen UIs
        */
        void setBackgroundWidget(QWidget* w) { _backgroundWidget = w; }
        QWidget* getBackgroundWidget() { return _backgroundWidget; }

        QGraphicsView* getQGraphicsView() { return _graphicsView; }
        QGraphicsScene* getQGraphicsScene() { return _graphicsScene; }

    protected:

        bool handlePointerEvent(int x, int y, int buttonMask);
        bool handleKeyEvent(int key, bool keyDown);
        QWidget* getWidgetAt(const QPoint& pos);

        osg::observer_ptr<osg::Image>   _image;
        QWidget*                        _backgroundWidget;

        int                             _previousButtonMask;
        int                             _previousMouseX;
        int                             _previousMouseY;
        int                             _previousQtMouseX;
        int                             _previousQtMouseY;
        bool                            _previousSentEvent;
        bool                            _requiresRendering;

        int _width;
        int _height;

        typedef std::map<int, Qt::Key> KeyMap;
        KeyMap                          _keyMap;
        Qt::KeyboardModifiers           _qtKeyModifiers;

        QColor                          _backgroundColor;
        QPointer<QGraphicsView>         _graphicsView;
        QPointer<QGraphicsScene>        _graphicsScene;
        QPointer<QWidget>               _widget;

        OpenThreads::Mutex              _qimagesMutex;
        OpenThreads::Mutex              _qresizeMutex;
        unsigned int                    _previousFrameNumber;
        bool                            _newImageAvailable;
        unsigned int                    _currentRead;
        unsigned int                    _currentWrite;
        unsigned int                    _previousWrite;
        QImage                          _qimages[3];

        virtual void customEvent ( QEvent * event ) ;

    private slots:

        void repaintRequestedSlot(const QList<QRectF> &regions);
        void repaintRequestedSlot(const QRectF &region);

};

}

#endif
