#include "testOutboardWin.h"

testOutboardWin::testOutboardWin(QWidget *parent)
    : QDialog( parent )
{
    ui.setupUi( this );
}

QWidget *  testOutboardWin::getDrawingAreaWidget(void)
{
    return ui.graphicsView;
}
