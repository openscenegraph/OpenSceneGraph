#ifndef TESTPLOTWIN_HPP_
#define TESTPLOTWIN_HPP_

#include "ui_testOutboardWin.h"

class testOutboardWin : public QDialog
{
    Q_OBJECT

public:
    testOutboardWin( QWidget *parent = 0 );
    QWidget *getDrawingAreaWidget();

private:
    Ui::testOutboardWindow ui;
    
};


#endif // TESTOUTBOARDWIN_HPP_
