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
    QStandardItemModel *model = new QStandardItemModel(1, 4, window);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Raw")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("From")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("To")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("Message")));

    window->getUi()->tableSniffer->setModel(model);
}
