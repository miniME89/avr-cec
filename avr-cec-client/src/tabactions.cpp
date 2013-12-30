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

#include "tabactions.h"
#include "windowmain.h"
#include "ui_windowmain.h"
#include <QStandardItemModel>
#include <QDebug>
#include <QScrollBar>

TabActions::TabActions(WindowMain* window)
{
    this->window = window;
    setupUi();
}

TabActions::~TabActions()
{

}

void TabActions::setupUi()
{
    QStandardItemModel *model = new QStandardItemModel(0, 2, NULL);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Type")));

    QList<QStandardItem *> item;
    item << new QStandardItem(QIcon(":/icons/trigger.png"), "Trigger");
    item << new QStandardItem("Active Source");
    model->invisibleRootItem()->appendRow(item);

    QList<QStandardItem *> item2;
    item2 << new QStandardItem(QIcon(":/icons/event_enabled.png"), "Some Action 1");
    item2 << new QStandardItem("Keypress");
    model->item(0)->appendRow(item2);

    item2.clear();
    item2 << new QStandardItem(QIcon(":/icons/event_enabled.png"), "Some Action 2");
    item2 << new QStandardItem("Keypress");
    model->item(0)->appendRow(item2);

    item2.clear();
    item2 << new QStandardItem(QIcon(":/icons/event_enabled.png"), "Some Action 3");
    item2 << new QStandardItem("Keypress");
    model->item(0)->appendRow(item2);

    item2.clear();
    item2 << new QStandardItem(QIcon(":/icons/event_enabled.png"), "Some Action 4");
    item2 << new QStandardItem("Keypress");
    model->item(0)->appendRow(item2);

    window->getUi()->treeActions->setModel(model);

    connect(window->getUi()->buttonAddTriggerRule, SIGNAL(clicked()), this, SLOT(eventAddTriggerRule()));

    connect(window->getUi()->treeActions->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(eventTreeActionsSelectionChanged(const QModelIndex&, const QModelIndex&)));
}

void TabActions::addTriggerRule()
{
    //Widget container
    QWidget* containerTriggerRule = new QWidget(window->getUi()->containerTriggerRulesContents);
    containerTriggerRule->setObjectName(QStringLiteral("templateTriggerRule"));

    //horizontal layout
    QHBoxLayout* containerTriggerRuleLayout = new QHBoxLayout(containerTriggerRule);
    containerTriggerRuleLayout->setSpacing(6);
    containerTriggerRuleLayout->setContentsMargins(11, 11, 11, 11);
    containerTriggerRuleLayout->setObjectName(QStringLiteral("horizontalLayout_7"));
    containerTriggerRuleLayout->setContentsMargins(0, 0, 0, 0);

    //combo box 1 (operation)
    QComboBox* comboOperation = new QComboBox(containerTriggerRule);
    comboOperation->setObjectName(QStringLiteral("comboBox_8"));
    comboOperation->setMinimumSize(QSize(50, 0));
    comboOperation->setMaximumSize(QSize(50, 16777215));
    comboOperation->addItem("And");
    comboOperation->addItem("Or");

    containerTriggerRuleLayout->addWidget(comboOperation);

    //combo box 2 (Parameter)
    QComboBox* comboParameter = new QComboBox(containerTriggerRule);
    comboParameter->setObjectName(QStringLiteral("comboBox_9"));
    QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(comboParameter->sizePolicy().hasHeightForWidth());
    comboParameter->setSizePolicy(sizePolicy4);
    comboParameter->setMinimumSize(QSize(120, 0));

    containerTriggerRuleLayout->addWidget(comboParameter);

    //combo box 3 (type)
    QComboBox* comboType = new QComboBox(containerTriggerRule);
    comboType->setObjectName(QStringLiteral("comboBox_10"));
    QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy2.setHeightForWidth(comboType->sizePolicy().hasHeightForWidth());
    comboType->setSizePolicy(sizePolicy2);
    comboType->setMinimumSize(QSize(80, 0));
    comboType->addItem("contains");
    comboType->addItem("contains not");

    containerTriggerRuleLayout->addWidget(comboType);

    //combo box 4 (Value)
    QComboBox* boxValue = new QComboBox(containerTriggerRule);
    boxValue->setObjectName(QStringLiteral("comboBox_11"));
    sizePolicy2.setHeightForWidth(boxValue->sizePolicy().hasHeightForWidth());
    boxValue->setSizePolicy(sizePolicy2);
    boxValue->setMinimumSize(QSize(120, 0));

    containerTriggerRuleLayout->addWidget(boxValue);

    //remove button
    QPushButton* buttonRemove = new QPushButton(containerTriggerRule);
    buttonRemove->setObjectName(QStringLiteral("pushButton_3"));
    sizePolicy2.setHeightForWidth(buttonRemove->sizePolicy().hasHeightForWidth());
    buttonRemove->setSizePolicy(sizePolicy2);
    QIcon icon;
    icon.addFile(QStringLiteral(":/icons/cross.png"), QSize(), QIcon::Normal, QIcon::Off);
    buttonRemove->setIcon(icon);
    buttonRemove->setIconSize(QSize(14, 14));

    containerTriggerRuleLayout->addWidget(buttonRemove);

    connect(buttonRemove, SIGNAL(clicked()), this, SLOT(eventRemoveTriggerRule()));

    window->getUi()->containerTriggerRulesLayout->insertWidget(window->getUi()->containerTriggerRulesContents->children().size() - 3, containerTriggerRule);
    window->getUi()->containerTriggerRules->verticalScrollBar()->setValue(window->getUi()->containerTriggerRules->verticalScrollBar()->maximum());
}

void TabActions::eventAddTrigger()
{

}

void TabActions::eventAddTriggerRule()
{
    addTriggerRule();
}

void TabActions::eventRemoveTriggerRule()
{
    QWidget* senderWidget = (QWidget*)sender();
    delete senderWidget->parent();
}

#include "actions.h"
void TabActions::eventTreeActionsSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    QStandardItemModel* itemModel = (QStandardItemModel*)window->getUi()->treeActions->model();
    QStandardItem* selection;
    //selection->setData(QVariant(QVariant::UserType, new Trigger()));

    //is Trigger
    if (!current.parent().isValid())
    {
        selection = itemModel->item(current.row());
    }
    //is Action
    else
    {
        selection = itemModel->item(current.parent().row())->child(current.row());
    }

    qDebug() <<selection->text();
}
