#include "testMainWin.h"

testMainWin::testMainWin()
    : QMainWindow()
{
    ui.setupUi( this );
    connect ( ui.actionExit, SIGNAL(activated(void)), this, SLOT(close()) );

}


