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

#include <osg/Image>
#include <osg/observer_ptr>

#include <QtWebKit/QWebSettings>
#include <QtWebKit/QtWebKit>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QtEvents>

extern QCoreApplication* getOrCreateQApplication();

class QGraphicsViewAdapter : public QObject
{
    Q_OBJECT

    public:

        QGraphicsViewAdapter(osg::Image* image, QWidget* widget);

        void setUpKeyMap();

        bool sendPointerEvent(int x, int y, int buttonMask);


        bool sendKeyEvent(int key, bool keyDown);


        void setFrameLastRendered(const osg::FrameStamp* frameStamp);

        void clearWriteBuffer();

        void render();

        void assignImage(unsigned int i);

    protected:

        bool handlePointerEvent(int x, int y, int buttonMask);
        bool handleKeyEvent(int key, bool keyDown);

        osg::observer_ptr<osg::Image>   _image;

        unsigned int                    _previousButtonMask;
        int                             _previousMouseX;
        int                             _previousMouseY;

        typedef std::map<int, Qt::Key> KeyMap;
        KeyMap                          _keyMap;
        Qt::KeyboardModifiers           _qtKeyModifiers;

        QColor                          _backgroundColor;
        QPointer<QGraphicsView>         _graphicsView;
        QPointer<QGraphicsScene>        _graphicsScene;

        OpenThreads::Mutex              _qimagesMutex;
        unsigned int                    _previousFrameNumber;
        bool                            _newImageAvailable;
        unsigned int                    _currentRead;
        unsigned int                    _currentWrite;
        unsigned int                    _previousWrite;
        QImage                          _qimages[3];

        virtual void customEvent ( QEvent * event ) ;

    private slots:

        void repaintRequestedSlot(const QList<QRectF> &regions);

};

#endif