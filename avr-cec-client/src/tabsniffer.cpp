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

#include "tabsniffer.h"
#include "windowmain.h"
#include "ui_windowmain.h"
#include <QStandardItemModel>
#include <QDebug>

using namespace avrcec;

TabSniffer::TabSniffer(WindowMain* window)
{
    this->window = window;
    setupUi();
}

TabSniffer::~TabSniffer()
{

}

void TabSniffer::setupUi()
{
    QStandardItemModel *model = new QStandardItemModel(0, 4, window);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Raw")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("From")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("To")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("Message")));

    window->getUi()->tableSniffer->setModel(model);

    window->getConnector().addListenerCECMessage(&TabSniffer::listenerCECMessage, this);
}

void TabSniffer::listenerCECMessage(void* data)
{
    QStandardItemModel* model = (QStandardItemModel*)window->getUi()->tableSniffer->model();
    CECMessage* message = (CECMessage*)data;

    QString rawString;
    std::vector<byte> rawData = message->getRaw();
    qDebug() <<rawData.size();
    for (unsigned int i = 0; i < rawData.size(); i++)
    {
        rawString.append(QString("%1").arg(rawData[i], 2, 16, QChar('0')).toUpper().append(" "));
    }

    QList<QStandardItem*> item;
    item << new QStandardItem(rawString);
    item << new QStandardItem("...");
    item << new QStandardItem("...");
    item << new QStandardItem("...");

    model->appendRow(item);
}
