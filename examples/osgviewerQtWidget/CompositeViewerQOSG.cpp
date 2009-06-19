// CompositeViewerQOSG.cpp

// #include <QCore/QDebug>
#include "CompositeViewerQOSG.h"

////////////////////////////////////////////////////////////////////////////////
CompositeViewerQOSG::CompositeViewerQOSG( QWidget * parent, Qt::WindowFlags f)
    : QWidget( parent, f ), osgViewer::CompositeViewer()
{
    setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);

    connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
    _timer.start(10);  // Don't know why 10, but 1 was no faster.
}


void CompositeViewerQOSG::paintEvent( QPaintEvent * /* event */ ) 
{ 
    frame(); 
}


