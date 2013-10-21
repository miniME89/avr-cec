#include "windowmain.h"
#include "ui_windowmain.h"
#include <QStandardItemModel>
#include <QDebug>

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
