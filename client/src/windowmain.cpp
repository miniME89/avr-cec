#include "windowmain.h"
#include "ui_windowmain.h"
#include "dialogaction.h"
#include <QStandardItemModel>
#include <QDebug>
#include "opendevice.h"

#define  USB_CFG_VENDOR_ID       0xc0, 0x16 /* = 0x16c0 = 5824 = voti.nl */
#define  USB_CFG_DEVICE_ID       0xdc, 0x05 /* = 0x05dc = 1500 */
#define  USB_CFG_VENDOR_NAME     'o', 'b', 'd', 'e', 'v', '.', 'a', 't'
#define  USB_CFG_DEVICE_NAME     'L', 'E', 'D', 'C', 'o', 'n', 't', 'r', 'o', 'l'

WindowMain::WindowMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowMain)
{
    ui->setupUi(this);
    setupUi();
}

WindowMain::~WindowMain()
{
    delete ui;
}

void WindowMain::setupUi()
{
    QStandardItemModel *model = new QStandardItemModel(this);
    model->appendRow(new QStandardItem(QIcon(":/icons/connection.png"), "Connection"));
    model->appendRow(new QStandardItem(QIcon(":/icons/actions.png"), "Actions"));
    model->appendRow(new QStandardItem(QIcon(":/icons/sniffer.png"), "Sniffer"));
    model->appendRow(new QStandardItem(QIcon(":/icons/settings.png"), "Settings"));

    for (int i = 0; i < model->rowCount(); i++) {
        model->item(i)->setSizeHint(QSize(50, 50));
    }

    ui->mainMenu->setModel(model);

    connect(ui->mainMenu->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChangedMainMenu(QItemSelection)));
    //ui->mainMenu->selectionModel()->select(ui->mainMenu->rootIndex(), QItemSelectionModel::Select);

    setupUiPageConnection();
    setupUiPageActions();
    setupUiPageSniffer();
    setupUiPageSettings();

    usb_dev_handle *handle = NULL;
    char vendor[] = {USB_CFG_VENDOR_NAME, 0};
    char product[] = {USB_CFG_DEVICE_NAME, 0};
    const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
    int vid = rawVid[1] * 256 + rawVid[0];
    int pid = rawPid[1] * 256 + rawPid[0];

    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0) {
        qDebug() <<"Could not find USB device \"" <<product <<"\" with vid=" <<vid <<" pid=" <<pid;
    }
    else
    {
        qDebug() <<"Found USB device \"" <<product <<"\" with vid=" <<vid <<" pid=" <<pid;
    }
}

void WindowMain::setupUiPageConnection()
{

}

void WindowMain::setupUiPageActions()
{
    QStandardItemModel *model = new QStandardItemModel(1, 3, this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Column1 Header")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Column2 Header")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("Column3 Header")));

    connect(ui->buttonAdd, SIGNAL(clicked()), this, SLOT(clickedButtonAdd()));

    ui->tableActions->setModel(model);
}

void WindowMain::setupUiPageSniffer()
{
    QStandardItemModel *model = new QStandardItemModel(1, 4, this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Raw")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("From")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("To")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("Message")));

    ui->tableSniffer->setModel(model);
}

void WindowMain::setupUiPageSettings()
{

}

void WindowMain::selectionChangedMainMenu(QItemSelection selection)
{
    ui->stackedContent->setCurrentIndex(ui->mainMenu->currentIndex().row());
}

void WindowMain::clickedButtonAdd()
{
    DialogAction* dialogAction = new DialogAction();
    dialogAction->open();
}
