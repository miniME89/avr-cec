#include "windowmain.h"
#include "ui_windowmain.h"
#include "dialogaction.h"
#include <QStandardItemModel>
#include <QDebug>

using namespace avrcec;

WindowMain::WindowMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowMain)
{
    ui->setupUi(this);
    setupUi();
}

WindowMain::~WindowMain()
{
    delete ui;
}

int counterMessagesRead = 0;
void WindowMain::listenerCECMessage(void* data)
{
    CECMessage* message = (CECMessage*)data;

    printf("message: %i    bytes read: %i    data: ", ++counterMessagesRead , message->size);
    for (int i = 0; i < message->size; i++)
    {
        printf("%02X ", message->data[i]);
    }
    printf("\n");
    fflush(stdout);
}

void WindowMain::listenerDebugMessage(void* data)
{
    DebugMessage* message = (DebugMessage*)data;

    printf("Debug: ");
    for (int i = 0; i < message->size; i++)
    {
        printf("%c", message->data[i]);
    }
    printf("\n");
    fflush(stdout);
}

void WindowMain::listenerConfig(void* data)
{

}

void WindowMain::setupUi()
{
    QStandardItemModel *model = new QStandardItemModel(this);
    model->appendRow(new QStandardItem(QIcon(":/icons/connection.png"), "Connection"));
    model->appendRow(new QStandardItem(QIcon(":/icons/actions.png"), "Actions"));
    model->appendRow(new QStandardItem(QIcon(":/icons/sniffer.png"), "Sniffer"));
    model->appendRow(new QStandardItem(QIcon(":/icons/debug.png"), "Debug"));
    model->appendRow(new QStandardItem(QIcon(":/icons/settings.png"), "Settings"));

    for (int i = 0; i < model->rowCount(); i++) {
        model->item(i)->setSizeHint(QSize(50, 50));
    }

    ui->mainMenu->setModel(model);

    connect(ui->mainMenu->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChangedMainMenu(QItemSelection)));
    //ui->mainMenu->selectionModel()->select(ui->mainMenu->rootIndex(), QItemSelectionModel::Select);

    if (connector.connect())
    {
        printf("connected: vendorName=%s, deviceName=%s, vendorId=%i, deviceId=%i\n", connector.getVendorName(), connector.getDeviceName(), connector.getVendorId(), connector.getDeviceId());

        connector.addListenerCECMessage(&WindowMain::listenerCECMessage, *this);
        connector.addListenerDebugMessage(&WindowMain::listenerDebugMessage, *this);
    }
    else
    {
        printf("couldn't connect!\n");
    }

    setupUiPageConnection();
    setupUiPageActions();
    setupUiPageSniffer();
    setupUiPageSettings();
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

void WindowMain::setupUiPageDebug()
{

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
    //DialogAction* dialogAction = new DialogAction();
    //dialogAction->open();
}