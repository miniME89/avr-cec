#include "windowmain.h"
#include "ui_windowmain.h"

WindowMain::WindowMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WindowMain)
{
    ui->setupUi(this);
}

WindowMain::~WindowMain()
{
    delete ui;
}
