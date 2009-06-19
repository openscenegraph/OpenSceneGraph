#ifndef TESTMAINWIN_HPP_
#define TESTMAINWIN_HPP_

#include "ui_testMainWin.h"

class testMainWin : public QMainWindow // QWidget
{
    Q_OBJECT

public:
    testMainWin();

public:  // for now
    Ui::testMainWin ui;
};


#endif // TESTMAINWIN_HPP_
