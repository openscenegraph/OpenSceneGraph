#ifndef _COMPOSITE_VIEWER_QOSG_HPP_
#define _COMPOSITE_VIEWER_QOSG_HPP_

#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <osgViewer/CompositeViewer>

class QPaintEvent;

//------------------------------------------------------------------------------
class CompositeViewerQOSG : public QWidget, public osgViewer::CompositeViewer
{
    Q_OBJECT


public:

    CompositeViewerQOSG( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    virtual ~CompositeViewerQOSG() {}

    void paintEvent( QPaintEvent * /* event */ );

protected:
    QTimer _timer;

}; // CompositeViewerQOSG

#endif // _COMPOSITE_VIEWER_QOSG_HPP_
