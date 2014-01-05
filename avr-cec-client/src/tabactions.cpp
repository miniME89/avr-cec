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
#include <QFileDialog>

using namespace avrcec;

TabActions::TabActions(WindowMain* window)
{
    this->window = window;
    triggersFilename = "triggers.xml";

    setupUi();

    load(triggersFilename);
}

TabActions::~TabActions()
{
    save(triggersFilename);
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

    if (getSelectedTrigger() == NULL)
    {
        actionAddAction->setDisabled(true);
    }

    menu.exec(QCursor::pos());
}

void TabActions::addTriggerRule(Rule* rule = NULL)
{
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
    QComboBox* comboOptions = new QComboBox(containerTriggerRule);
    comboOptions->setObjectName(QStringLiteral("comboBox_11"));
    sizePolicy2.setHeightForWidth(comboOptions->sizePolicy().hasHeightForWidth());
    comboOptions->setSizePolicy(sizePolicy2);
    comboOptions->setMinimumSize(QSize(120, 0));

    containerTriggerRuleLayout->addWidget(comboOptions);

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

    //add parameters for selected message
    CECDefinitionMessage* definitionMessage = CECMessageFactory::getInstance()->getDefinitionMessage(window->getUi()->comboTriggerMessage->itemData(window->getUi()->comboTriggerMessage->currentIndex()).toInt());
    std::vector<CECDefinitionOperand*> operands = definitionMessage->getOperandList();
    for (unsigned int i = 0; i < operands.size(); i++)
    {
        comboParameter->addItem(QString(operands[i]->getName().c_str()), QVariant(operands[i]->getId()));
    }

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
    }

    //add options for selected parameter
    CECDefinitionOperand* definitionOperand = CECMessageFactory::getInstance()->getDefinitionOperand(comboParameter->itemData(comboParameter->currentIndex()).toInt());
    std::map<int, std::string> options = definitionOperand->getOptions();
    for (std::map<int, std::string>::const_iterator i = options.begin(); i != options.end(); i++)
    {
        comboOptions->addItem(QString(i->second.c_str()), QVariant(i->first));
    }

    if (rule != NULL)
    {
        //select option in combo box
        for (int i = 0; i < comboOptions->count(); i++)
        {
            if (rule->getValue() == comboOptions->itemData(i))
            {
                comboOptions->setCurrentIndex(i);

                break;
            }
        }
    }

    connect(comboConjunction, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerConjunctionChanged(int)));
    connect(comboParameter, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerParameterChanged(int)));
    connect(comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerTypeChanged(int)));
    connect(comboOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerOptionChanged(int)));
    connect(buttonRemove, SIGNAL(clicked()), this, SLOT(eventRemoveTriggerRule()));

    window->getUi()->containerTriggerRulesLayout->insertWidget(window->getUi()->containerTriggerRulesContents->children().size() - 3, containerTriggerRule);
    window->getUi()->containerTriggerRules->verticalScrollBar()->setValue(window->getUi()->containerTriggerRules->verticalScrollBar()->maximum());
}

void TabActions::load(QString filename)
{
    qDeleteAll(triggers);
    triggers.clear();

    Trigger::load(filename, triggers);
    updateTree();
}

void TabActions::save(QString filename)
{
    Trigger::save(filename, triggers);
}

void TabActions::updateTree()
{
    QStandardItemModel* model = (QStandardItemModel*)window->getUi()->treeActions->model();

    int prevTriggerIndex = getSelectedTriggerIndex();
    int prevActionIndex = getSelectedActionIndex();

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

    setSelection(prevTriggerIndex, prevActionIndex);
}

void TabActions::updateForm(Trigger* trigger)
{
    window->getUi()->stackedActionsContent->setCurrentIndex(0);

    //select message in combo box
    disconnect(window->getUi()->comboTriggerMessage, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerMessageChanged(int)));
    for (int i = 0; i < window->getUi()->comboTriggerMessage->count(); i++)
    {
        if (trigger->getMessage() == window->getUi()->comboTriggerMessage->itemData(i))
        {
            window->getUi()->comboTriggerMessage->setCurrentIndex(i);

            break;
        }
    }
    connect(window->getUi()->comboTriggerMessage, SIGNAL(currentIndexChanged(int)), this, SLOT(eventTriggerMessageChanged(int)));

    //check "Add Rule" button visibility
    CECDefinitionMessage* definition = CECMessageFactory::getInstance()->getDefinitionMessage(window->getUi()->comboTriggerMessage->itemData(window->getUi()->comboTriggerMessage->currentIndex()).toInt());
    if (definition->getOperandCount() > 0)
    {
        window->getUi()->buttonTriggerAddRule->show();
    }
    else
    {
        window->getUi()->buttonTriggerAddRule->hide();
    }

    //remove trigger rules
    QObjectList childs = window->getUi()->containerTriggerRulesContents->children();
    for (int i = 0; i < childs.size(); i++)
    {
        if (childs[i]->objectName() == "rule")
        {
            delete childs[i];
        }
    }

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

Trigger* TabActions::getSelectedTrigger()
{
    int triggerIndex = getSelectedTriggerIndex();
    if (triggerIndex >= 0)
    {
        return triggers[triggerIndex];
    }

    return NULL;
}

Action* TabActions::getSelectedAction()
{
    int triggerIndex = getSelectedTriggerIndex();
    int actionIndex = getSelectedActionIndex();
    if (triggerIndex >= 0 && actionIndex >= 0)
    {
        return triggers[triggerIndex]->getActions()[actionIndex];
    }

    return NULL;
}

int TabActions::getSelectedTriggerIndex()
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

int TabActions::getSelectedActionIndex()
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

void TabActions::setSelection(int trigger, int action)
{
    qDebug() <<"select: trigger =" <<trigger <<", action =" <<action;

    QStandardItemModel* model = (QStandardItemModel*)window->getUi()->treeActions->model();
    QItemSelectionModel* selectionModel = window->getUi()->treeActions->selectionModel();

    if (trigger >= 0)
    {
        QStandardItem* itemTrigger = model->item(trigger);
        if (itemTrigger != NULL)
        {
            if (action < 0)
            {
                selectionModel->setCurrentIndex(model->item(trigger)->index(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
            else
            {
                QStandardItem* itemAction = itemTrigger->child(action);
                if (itemAction != NULL)
                {
                    selectionModel->setCurrentIndex(itemAction->index(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                }
            }
        }
    }
}

void TabActions::eventTriggerMessageChanged(int index)
{
    qDebug() <<"eventTriggerMessageChanged";

    CECDefinitionMessage* definition = CECMessageFactory::getInstance()->getDefinitionMessage(window->getUi()->comboTriggerMessage->itemData(index).toInt());

    window->getUi()->labelTriggerDescription->setText(QString(definition->getDescription().c_str()));
    window->getUi()->labelTriggerDirect->setText((definition->isDirect()) ? "yes" : "no");
    window->getUi()->labelTriggerBroadcast->setText((definition->isBroadcast()) ? "yes" : "no");

    //delete trigger rules
    getSelectedTrigger()->removeRulesAll();

    QObjectList childs = window->getUi()->containerTriggerRulesContents->children();
    for (int i = 0; i < childs.size(); i++)
    {
        if (childs[i]->objectName() == "rule")
        {
            delete childs[i];
        }
    }

    //check "Add Rule" button visibility
    if (definition->getOperandCount() > 0)
    {
        window->getUi()->buttonTriggerAddRule->show();
    }
    else
    {
        window->getUi()->buttonTriggerAddRule->hide();
    }

    //update trigger
    getSelectedTrigger()->setMessage(definition->getId());
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
    getSelectedTrigger()->getRules()[currentTriggerRuleIndex]->setConjunction((Rule::Conjunction)index);
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

    qDebug() <<"current trigger rule index: " <<currentTriggerRuleIndex;

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
    getSelectedTrigger()->getRules()[currentTriggerRuleIndex]->setParameter(comboParameter->itemData(index).toInt());
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
    getSelectedTrigger()->getRules()[currentTriggerRuleIndex]->setType((Rule::Type)index);
}

void TabActions::eventTriggerOptionChanged(int index)
{
    qDebug() <<"eventTriggerOptionChanged";

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

    qDebug() <<"current trigger rule index: " <<currentTriggerRuleIndex;

    //get sender
    QComboBox* senderWidget = (QComboBox*)sender();

    //update trigger rule
    getSelectedTrigger()->getRules()[currentTriggerRuleIndex]->setValue(QString::number(senderWidget->itemData(index).toInt()));
}

void TabActions::eventActionTypeChanged(int index)
{
    qDebug() <<"eventActionTypeChanged";

}

void TabActions::eventImport()
{
    qDebug() <<"eventImport";

    QString filename = QFileDialog::getOpenFileName(window, tr("Import Triggers"), "", tr("XML (*.xml)"));

    QList<Trigger*> loadedTriggers;
    if (Trigger::load(filename, loadedTriggers))
    {
        triggers <<loadedTriggers;

        updateTree();
    }
}

void TabActions::eventExport()
{
    qDebug() <<"eventExport";

    QString filename = QFileDialog::getSaveFileName(window, tr("Export Triggers"), "", tr("XML (*.xml)"));

    Trigger::save(filename, triggers);
}

void TabActions::eventAddTrigger()
{
    qDebug() <<"eventAddTrigger";

    triggers.push_back(new Trigger("Trigger", 0, QList<Rule*>(), QList<Action*>()));

    updateTree();
    setSelection(triggers.size() - 1);
}

void TabActions::eventAddTriggerRule()
{
    qDebug() <<"eventAddTriggerRule";

    getSelectedTrigger()->addRule(new Rule(Rule::AND, 0, Rule::CONTAINS, ""));

    updateTree();
}

void TabActions::eventAddAction()
{
    qDebug() <<"eventAddAction";

    ActionFactory actionFactory;
    Trigger* currentTrigger = getSelectedTrigger();
    currentTrigger->addAction(actionFactory.create("Action", Action::KEY, QList<Action::Parameter>()));

    updateTree();
    setSelection(triggers.indexOf(currentTrigger), currentTrigger->getActions().size() - 1);
}

void TabActions::eventRemove()
{
    qDebug() <<"eventRemove";

    if (getSelectedAction() != NULL)
    {
        eventRemoveAction();
    }
    else if (getSelectedTrigger() != NULL)
    {
        eventRemoveTrigger();
    }
}

void TabActions::eventRemoveTrigger()
{
    qDebug() <<"eventRemoveTrigger";

    int triggerIndex = getSelectedTriggerIndex();
    if (triggerIndex > -1)
    {
        triggers.removeAt(triggerIndex);
    }

    updateTree();
    setSelection(0);
}

void TabActions::eventRemoveTriggerRule()
{
    qDebug() <<"eventRemoveTriggerRule";

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

    qDebug() <<"current trigger rule index: " <<currentTriggerRuleIndex;


    QWidget* senderWidget = (QWidget*)sender();
    delete senderWidget->parent();

    getSelectedTrigger()->removeRule(currentTriggerRuleIndex);
}

void TabActions::eventRemoveAction()
{
    qDebug() <<"eventRemoveAction";

    int triggerIndex = getSelectedTriggerIndex();
    int actionIndex = getSelectedActionIndex();
    if (triggerIndex > -1 && actionIndex > -1)
    {
        triggers[triggerIndex]->removeAction(actionIndex);
    }

    updateTree();
    setSelection(triggerIndex);
}

void TabActions::eventTreeActionsSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    qDebug() <<"eventTreeActionsSelectionChanged";

    if (current.isValid())
    {
        //is Trigger
        if (!current.parent().isValid())
        {
            updateForm(getSelectedTrigger());
        }
        //is Action
        else
        {

            updateForm(getSelectedAction());
        }
    }
}
