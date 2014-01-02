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
#include <QMenu>

using namespace avrcec;

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
    window->getUi()->treeActions->setModel(model);

    //add trigger CEC messages
    std::vector<CECDefinitionMessage*> definitionsMessage = CECMessageFactory::getInstance()->getDefinitionsMessage();
    for (unsigned int i = 0; i < definitionsMessage.size(); i++)
    {
        window->getUi()->comboTriggerMessage->addItem(QString(definitionsMessage[i]->getName().c_str()), QVariant(definitionsMessage[i]->getId()));
    }

    connect(window->getUi()->comboTriggerMessage, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerMessageChanged(int)));
    connect(window->getUi()->comboActionType, SIGNAL(currentIndexChanged(int)), this, SLOT(eventActionTypeChanged(int)));
    connect(window->getUi()->buttonImport, SIGNAL(clicked()), this, SLOT(eventImport()));
    connect(window->getUi()->buttonExport, SIGNAL(clicked()), this, SLOT(eventExport()));
    connect(window->getUi()->buttonTriggerAddRule, SIGNAL(clicked()), this, SLOT(eventAddTriggerRule()));
    connect(window->getUi()->treeActions->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(eventTreeActionsSelectionChanged(const QModelIndex&, const QModelIndex&)));
    connect(window->getUi()->treeActions, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(eventTriggerShowContextMenu(const QPoint)));

    load();
}

void TabActions::eventTriggerShowContextMenu(const QPoint &pos)
{
    qDebug() <<"eventTriggerShowContextMenu";

    QMenu menu;
    QAction* actionAddTrigger = menu.addAction("Add Trigger");
    connect(actionAddTrigger, SIGNAL(triggered()), this, SLOT(eventAddTrigger()));
    QAction* actionAddAction = menu.addAction("Add Action");
    connect(actionAddAction, SIGNAL(triggered()), this, SLOT(eventAddAction()));
    QAction* actionRemove = menu.addAction("Remove");
    connect(actionRemove, SIGNAL(triggered()), this, SLOT(eventRemove()));

    if (getCurrentTrigger() == NULL)
    {
        actionAddAction->setDisabled(true);
    }

    menu.exec(QCursor::pos());
}

void TabActions::addTriggerRule(Rule* rule = NULL)
{
    CECDefinitionMessage* definition = CECMessageFactory::getInstance()->getDefinitionMessage(window->getUi()->comboTriggerMessage->itemData(window->getUi()->comboTriggerMessage->currentIndex()).toInt());

    //Widget container
    QWidget* containerTriggerRule = new QWidget(window->getUi()->containerTriggerRulesContents);
    containerTriggerRule->setObjectName(QStringLiteral("rule"));

    //horizontal layout
    QHBoxLayout* containerTriggerRuleLayout = new QHBoxLayout(containerTriggerRule);
    containerTriggerRuleLayout->setSpacing(6);
    containerTriggerRuleLayout->setContentsMargins(11, 11, 11, 11);
    containerTriggerRuleLayout->setObjectName(QStringLiteral("horizontalLayout_7"));
    containerTriggerRuleLayout->setContentsMargins(0, 0, 0, 0);

    //combo box 1 (operation)
    QComboBox* comboConjunction = new QComboBox(containerTriggerRule);
    comboConjunction->setObjectName(QStringLiteral("comboBox_8"));
    comboConjunction->setMinimumSize(QSize(50, 0));
    comboConjunction->setMaximumSize(QSize(50, 16777215));
    comboConjunction->addItem("And");
    comboConjunction->addItem("Or");

    containerTriggerRuleLayout->addWidget(comboConjunction);

    //combo box 2 (Parameter)
    QComboBox* comboParameter = new QComboBox(containerTriggerRule);
    comboParameter->setObjectName(QStringLiteral("comboBox_9"));
    QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(comboParameter->sizePolicy().hasHeightForWidth());
    comboParameter->setSizePolicy(sizePolicy4);
    comboParameter->setMinimumSize(QSize(120, 0));

    //add trigger CEC message parameter
    std::vector<CECDefinitionOperand*> operands = definition->getOperandList();
    for (unsigned int i = 0; i < operands.size(); i++)
    {
        comboParameter->addItem(QString(operands[i]->getName().c_str()), QVariant(operands[i]->getId()));
    }

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
    QComboBox* comboValue = new QComboBox(containerTriggerRule);
    comboValue->setObjectName(QStringLiteral("comboBox_11"));
    sizePolicy2.setHeightForWidth(comboValue->sizePolicy().hasHeightForWidth());
    comboValue->setSizePolicy(sizePolicy2);
    comboValue->setMinimumSize(QSize(120, 0));

    containerTriggerRuleLayout->addWidget(comboValue);

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

    if (rule != NULL)
    {
        comboConjunction->setCurrentIndex(rule->getConjunction());
        comboType->setCurrentIndex(rule->getType());
    }

    connect(comboConjunction, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerConjunctionChanged(int)));
    connect(comboParameter, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerParameterChanged(int)));
    connect(comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerTypeChanged(int)));
    connect(comboValue, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerValueChanged(int)));
    connect(buttonRemove, SIGNAL(clicked()), this, SLOT(eventRemoveTriggerRule()));

    window->getUi()->containerTriggerRulesLayout->insertWidget(window->getUi()->containerTriggerRulesContents->children().size() - 3, containerTriggerRule);
    window->getUi()->containerTriggerRules->verticalScrollBar()->setValue(window->getUi()->containerTriggerRules->verticalScrollBar()->maximum());

    if (rule != NULL)
    {
        //select parameter in combo box
        for (int i = 0; i < comboParameter->count(); i++)
        {
            if (rule->getParameter() == comboParameter->itemData(i))
            {
                comboParameter->setCurrentIndex(i);

                break;
            }
        }

        //select value in combo box
        for (int i = 0; i < comboValue->count(); i++)
        {
            if (rule->getValue() == comboValue->itemData(i))
            {
                comboValue->setCurrentIndex(i);

                break;
            }
        }
    }
}

void TabActions::removeTriggerRules()
{
    QObjectList childs = window->getUi()->containerTriggerRulesContents->children();
    for (int i = 0; i < childs.size(); i++)
    {
        if (childs[i]->objectName() == "rule")
        {
            delete childs[i];
        }
    }
}

void TabActions::load()
{
    //TODO delete allocated memmory?
    triggers.clear();

    if (Trigger::load(triggers))
    {
        updateTree();
    }
}

void TabActions::save()
{
    Trigger::save(triggers);
}

void TabActions::updateTree()
{
    QStandardItemModel* model = (QStandardItemModel*)window->getUi()->treeActions->model();

    model->removeRows(0, model->rowCount());

    //add triggers
    for (int i = 0; i < triggers.size(); i++)
    {
        QList<QStandardItem*> item;
        item << new QStandardItem(QIcon(":/icons/trigger.png"), triggers[i]->getName());
        item << new QStandardItem("...");

        model->invisibleRootItem()->appendRow(item);

        //add actions
        QList<Action*> actions = triggers[i]->getActions();
        for (int j = 0; j < actions.size(); j++)
        {
            QList<QStandardItem*> item;
            item << new QStandardItem(QIcon(":/icons/event_enabled.png"), actions[j]->getName());
            item << new QStandardItem("...");

            model->item(model->rowCount() - 1)->appendRow(item);
        }
    }

    window->getUi()->treeActions->expandAll();
}

void TabActions::updateForm(Trigger* trigger)
{
    window->getUi()->stackedActionsContent->setCurrentIndex(0);

    //select message in combo box
    for (int i = 0; i < window->getUi()->comboTriggerMessage->count(); i++)
    {
        if (trigger->getMessage() == window->getUi()->comboTriggerMessage->itemData(i))
        {
            window->getUi()->comboTriggerMessage->setCurrentIndex(i);

            break;
        }
    }

    removeTriggerRules();

    //add rules
    QList<Rule*> rules = trigger->getRules();
    for (int i = 0; i < rules.size(); i++)
    {
        addTriggerRule(rules[i]);
    }
}

void TabActions::updateForm(Action* action)
{
    window->getUi()->stackedActionsContent->setCurrentIndex(1);
}

Trigger* TabActions::getCurrentTrigger()
{
    QItemSelectionModel* selectionModel = window->getUi()->treeActions->selectionModel();
    QModelIndex current = selectionModel->currentIndex();

    if (current.isValid())
    {
        //selection is Trigger
        if (!current.parent().isValid())
        {
            qDebug() <<"current trigger index: " <<current.row();
            return triggers[current.row()];
        }
        //selection is Action
        else
        {
            qDebug() <<"current trigger index: " <<current.parent().row();
            return triggers[current.parent().row()];
        }
    }

    return NULL;
}

Action* TabActions::getCurrentAction()
{
    QItemSelectionModel* selectionModel = window->getUi()->treeActions->selectionModel();
    QModelIndex current = selectionModel->currentIndex();

    if (current.isValid())
    {
        //selection is action
        if (current.parent().isValid())
        {
            qDebug() <<"current trigger index: " <<current.parent().row();
            qDebug() <<"current action index: " <<current.row();
            return triggers[current.parent().row()]->getActions()[current.row()];
        }
    }

    return NULL;
}

int TabActions::getCurrentTriggerIndex()
{
    QItemSelectionModel* selectionModel = window->getUi()->treeActions->selectionModel();
    QModelIndex current = selectionModel->currentIndex();

    if (current.isValid())
    {
        //selection is Trigger
        if (!current.parent().isValid())
        {
            qDebug() <<"current trigger index: " <<current.row();
            return current.row();
        }
        //selection is Action
        else
        {
            qDebug() <<"current trigger index: " <<current.parent().row();
            return current.parent().row();
        }
    }

    return -1;
}

int TabActions::getCurrentActionIndex()
{
    QItemSelectionModel* selectionModel = window->getUi()->treeActions->selectionModel();
    QModelIndex current = selectionModel->currentIndex();

    if (current.isValid())
    {
        //selection is action
        if (current.parent().isValid())
        {
            qDebug() <<"current action index: " <<current.row();
            return current.row();
        }
    }

    return -1;
}

void TabActions::selectTrigger(int trigger)
{
    QStandardItemModel* model = (QStandardItemModel*)window->getUi()->treeActions->model();
    QItemSelectionModel* selectionModel = window->getUi()->treeActions->selectionModel();
    selectionModel->setCurrentIndex(model->item(trigger)->index(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void TabActions::selectAction(int trigger, int action)
{
    QStandardItemModel* model = (QStandardItemModel*)window->getUi()->treeActions->model();
    QItemSelectionModel* selectionModel = window->getUi()->treeActions->selectionModel();
    selectionModel->setCurrentIndex(model->item(trigger)->child(action)->index(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void TabActions::eventTriggerMessageChanged(int index)
{
    qDebug() <<"eventTriggerMessageChanged";

    CECDefinitionMessage* definition = CECMessageFactory::getInstance()->getDefinitionMessage(window->getUi()->comboTriggerMessage->itemData(index).toInt());

    window->getUi()->labelTriggerDescription->setText(QString(definition->getDescription().c_str()));
    window->getUi()->labelTriggerDirect->setText((definition->isDirect()) ? "yes" : "no");
    window->getUi()->labelTriggerBroadcast->setText((definition->isBroadcast()) ? "yes" : "no");

    removeTriggerRules();

    if (definition->getOperandCount() > 0)
    {
        window->getUi()->buttonTriggerAddRule->show();
    }
    else
    {
        window->getUi()->buttonTriggerAddRule->hide();
    }

    //update trigger
    getCurrentTrigger()->setMessage(definition->getId());
}

void TabActions::eventTriggerConjunctionChanged(int index)
{
    qDebug() <<"eventTriggerConjunctionChanged";

    //get current trigger rule index
    QObject* container = sender()->parent();
    QObjectList children = window->getUi()->containerTriggerRulesContents->children();
    int currentTriggerRuleIndex;
    for (currentTriggerRuleIndex = 0; currentTriggerRuleIndex < children.size(); currentTriggerRuleIndex++)
    {
        if (children[currentTriggerRuleIndex] == container)
        {
            break;
        }
    }
    currentTriggerRuleIndex = currentTriggerRuleIndex - 2;

    //update trigger rule
    getCurrentTrigger()->getRules()[currentTriggerRuleIndex]->setConjunction((Rule::Conjunction)index);
}

void TabActions::eventTriggerParameterChanged(int index)
{
    qDebug() <<"eventTriggerParameterChanged";

    //get current trigger rule index
    QObject* container = sender()->parent();
    QObjectList children = window->getUi()->containerTriggerRulesContents->children();
    int currentTriggerRuleIndex;
    for (currentTriggerRuleIndex = 0; currentTriggerRuleIndex < children.size(); currentTriggerRuleIndex++)
    {
        if (children[currentTriggerRuleIndex] == container)
        {
            break;
        }
    }
    currentTriggerRuleIndex = currentTriggerRuleIndex - 2;

    //get widgets
    QWidget* senderWidget = (QWidget*)sender();
    QComboBox* comboParameter = (QComboBox*)senderWidget->parent()->children()[2];
    QComboBox* comboOptions = (QComboBox*)senderWidget->parent()->children()[4];

    //update options for selected parameter
    CECDefinitionOperand* definitionOperand = CECMessageFactory::getInstance()->getDefinitionOperand(comboParameter->itemData(index).toInt());
    comboOptions->clear();
    for (std::map<int, std::string>::const_iterator i = definitionOperand->getOptions().begin(); i != definitionOperand->getOptions().end(); i++)
    {
        comboOptions->addItem(QString(i->second.c_str()), QVariant(i->first));
    }

    //update trigger rule
    getCurrentTrigger()->getRules()[currentTriggerRuleIndex]->setParameter(comboParameter->itemData(index).toInt());
}

void TabActions::eventTriggerTypeChanged(int index)
{
    qDebug() <<"eventTriggerTypeChanged";

    //get current trigger rule index
    QObject* container = sender()->parent();
    QObjectList children = window->getUi()->containerTriggerRulesContents->children();
    int currentTriggerRuleIndex;
    for (currentTriggerRuleIndex = 0; currentTriggerRuleIndex < children.size(); currentTriggerRuleIndex++)
    {
        if (children[currentTriggerRuleIndex] == container)
        {
            break;
        }
    }
    currentTriggerRuleIndex = currentTriggerRuleIndex - 2;

    //update trigger rule
    getCurrentTrigger()->getRules()[currentTriggerRuleIndex]->setType((Rule::Type)index);
}

void TabActions::eventTriggerValueChanged(int index)
{
    qDebug() <<"eventTriggerValueChanged";

    //get current trigger rule index
    QObject* container = sender()->parent();
    QObjectList children = window->getUi()->containerTriggerRulesContents->children();
    int currentTriggerRuleIndex;
    for (currentTriggerRuleIndex = 0; currentTriggerRuleIndex < children.size(); currentTriggerRuleIndex++)
    {
        if (children[currentTriggerRuleIndex] == container)
        {
            break;
        }
    }
    currentTriggerRuleIndex = currentTriggerRuleIndex - 2;

    //get sender
    QComboBox* senderWidget = (QComboBox*)sender();

    //update trigger rule
    getCurrentTrigger()->getRules()[currentTriggerRuleIndex]->setValue(QString::number(senderWidget->itemData(index).toInt()));
}

void TabActions::eventActionTypeChanged(int index)
{
    qDebug() <<"eventActionTypeChanged";

}

void TabActions::eventImport()
{
    qDebug() <<"eventImport";

    load();
}

void TabActions::eventExport()
{
    qDebug() <<"eventExport";

    save();
}

void TabActions::eventAddTrigger()
{
    qDebug() <<"eventAddTrigger";

    triggers.push_back(new Trigger("Trigger", 0, QList<Rule*>(), QList<Action*>()));

    updateTree();
    selectTrigger(triggers.size() - 1);
}

void TabActions::eventAddTriggerRule()
{
    qDebug() <<"eventAddTriggerRule";

    addTriggerRule();
}

void TabActions::eventAddAction()
{
    qDebug() <<"eventAddAction";

    ActionFactory actionFactory;
    Trigger* currentTrigger = getCurrentTrigger();
    currentTrigger->addAction(actionFactory.create("Action", Action::KEY, QList<Action::Parameter>()));

    updateTree();
    selectAction(triggers.indexOf(currentTrigger), currentTrigger->getActions().size() - 1);
}

void TabActions::eventRemove()
{
    qDebug() <<"eventRemove";

    if (getCurrentAction() != NULL)
    {
        eventRemoveAction();
    }
    else if (getCurrentTrigger() != NULL)
    {
        eventRemoveTrigger();
    }
}

void TabActions::eventRemoveTrigger()
{
    qDebug() <<"eventRemoveTrigger";

    int triggerIndex = getCurrentTriggerIndex();
    if (triggerIndex > -1)
    {
        triggers.removeAt(triggerIndex);
    }

    updateTree();
}

void TabActions::eventRemoveTriggerRule()
{
    qDebug() <<"eventRemoveTriggerRule";

    QWidget* senderWidget = (QWidget*)sender();
    delete senderWidget->parent();
}

void TabActions::eventRemoveAction()
{
    qDebug() <<"eventRemoveAction";

    int triggerIndex = getCurrentTriggerIndex();
    int actionIndex = getCurrentActionIndex();
    if (triggerIndex > -1 && actionIndex > -1)
    {
        triggers[triggerIndex]->removeAction(actionIndex);
    }

    updateTree();
}

void TabActions::eventTreeActionsSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    qDebug() <<"eventTreeActionsSelectionChanged";

    if (current.isValid())
    {
        //is Trigger
        if (!current.parent().isValid())
        {
            updateForm(getCurrentTrigger());
        }
        //is Action
        else
        {

            updateForm(getCurrentAction());
        }
    }
}
