#include "dialogaction.h"
#include "ui_dialogaction.h"

DialogAction::DialogAction(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAction)
{
    ui->setupUi(this);
}

DialogAction::~DialogAction()
{
    delete ui;
}
