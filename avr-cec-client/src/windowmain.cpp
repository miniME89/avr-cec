/*
 *  Copyright (C) 2013
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windowmain.h"
#include "ui_windowmain.h"
#include <QStandardItemModel>
#include <QDebug>
#include <QScrollBar>

using namespace avrcec;

WindowMain::WindowMain(QWidget *parent) : QMainWindow(parent)
{
    ui = new Ui::WindowMain();

    ui->setupUi(this);
    setupUi();

    tabConnection = new TabConnection(this);
    tabActions = new TabActions(this);
    tabSniffer = new TabSniffer(this);
    tabDebug = new TabDebug(this);
    tabSettings = new TabSettings(this);
}

WindowMain::~WindowMain()
{
    delete tabConnection;
    delete tabActions;
    delete tabSniffer;
    delete tabDebug;
    delete tabSettings;

    delete ui;
}

void WindowMain::listenerCECMessage(void* data)
{
    CECMessage* message = (CECMessage*)data;

    printf("%s", message->toString().c_str());
    fflush(stdout);
}

void WindowMain::listenerDebugMessage(void* data)
{
    DebugMessage* message = (DebugMessage*)data;

    printf("Debug: %s\n", message->toString().c_str());
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

    for (int i = 0; i < model->rowCount(); i++)
    {
        model->item(i)->setSizeHint(QSize(50, 50));
    }

    ui->mainMenu->setModel(model);

    connect(ui->mainMenu->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(eventSelectionChangedMainMenu(QItemSelection)));
    //ui->mainMenu->selectionModel()->select(ui->mainMenu->rootIndex(), QItemSelectionModel::Select);

    if (connector.connect())
    {
        printf("connected: vendorName=%s, deviceName=%s, vendorId=%i, deviceId=%i\n", connector.getVendorName(), connector.getDeviceName(), connector.getVendorId(), connector.getDeviceId());

        connector.addListenerCECMessage(&WindowMain::listenerCECMessage, this);
        connector.addListenerDebugMessage(&WindowMain::listenerDebugMessage, this);
    }
}

void WindowMain::eventSelectionChangedMainMenu(QItemSelection selection)
{
    ui->stackedMainContent->setCurrentIndex(ui->mainMenu->currentIndex().row());
}

Ui::WindowMain* WindowMain::getUi()
{
    return ui;
}
