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

#include <actions.h>
#include <QDomDocument>
#include <QFile>
#include <QDebug>

using namespace avrcec;

//==================================================
// Action
//==================================================
Action::Action(QString name, Type type, QList<Parameter> parameters)
{
    this->name = name;
    this->type = type;
    this->parameters = parameters;
}

Action::~Action()
{
    qDebug() <<"destroy Action";
}

QString Action::getName()
{
    return name;
}

void Action::setName(QString name)
{
    this->name = name;
}

Action::Type Action::getType()
{
    return type;
}

void Action::setType(Action::Type type)
{
    this->type = type;
}

QList<Action::Parameter> Action::getParameters()
{
    return parameters;
}

void Action::setParameters(QList<Action::Parameter> parameters)
{
    this->parameters = parameters;
}

ActionKey::ActionKey(QString name, Action::Type type, QList<Parameter> parameters) : Action(name, type, parameters)
{

}

//TODO operating specific code... IFDEF...
#include <windows.h>
void ActionKey::execute()
{
    // This structure will be used to create the keyboard input event.
    INPUT ip;

    // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    // Press the "A" key
    ip.ki.wVk = 0x41; // virtual-key code for the "a" key
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));

    // Release the "A" key
    ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
    SendInput(1, &ip, sizeof(INPUT));
}

ActionFactory::ActionFactory()
{

}

Action* ActionFactory::create(QString name, Action::Type type, QList<Action::Parameter> parameters)
{
    if (type == Action::KEY)
    {
        return new ActionKey(name, type, parameters);
    }

    return NULL;
}

//==================================================
// Rule
//==================================================
Rule::Rule()
{

}

Rule::Rule(Rule::Conjunction conjunction, int parameter, Rule::Type type, QString value)
{
    this->conjunction = conjunction;
    this->parameter = parameter;
    this->type = type;
    this->value = value;
}

Rule::~Rule()
{
    qDebug() <<"destroy Rule";
}

Rule::Conjunction Rule::getConjunction()
{
    return conjunction;
}

void Rule::setConjunction(Rule::Conjunction conjunction)
{
    this->conjunction = conjunction;
}

int Rule::getParameter()
{
    return parameter;
}

void Rule::setParameter(int parameter)
{
    this->parameter = parameter;
}

Rule::Type Rule::getType()
{
    return type;
}

void Rule::setType(Rule::Type type)
{
    this->type = type;
}

QString Rule::getValue()
{
    return value;
}

void Rule::setValue(QString value)
{
    this->value = value;
}

//==================================================
// Trigger
//==================================================
Trigger::Trigger()
{
    message = -1;

    definitionMessage = NULL;
}

Trigger::Trigger(QString name, int message, QList<Rule*> rules, QList<Action*> actions)
{
    this->name = name;
    this->message = message;
    this->rules = rules;
    this->actions = actions;

    definitionMessage = CECMessageFactory::getInstance()->getDefinitionMessage(message);
}

Trigger::~Trigger()
{
    qDebug() <<"destroy Trigger";

    qDeleteAll(rules);
    rules.clear();
    qDeleteAll(actions);
    actions.clear();
}

QString Trigger::getName()
{
    return name;
}

void Trigger::setName(QString name)
{
    this->name = name;
}

int Trigger::getMessage()
{
    return message;
}

void Trigger::setMessage(int message)
{
    this->message = message;
    definitionMessage = CECMessageFactory::getInstance()->getDefinitionMessage(message);
}

QList<Rule*> Trigger::getRules()
{
    return rules;
}

void Trigger::setRules(QList<Rule *> rules)
{
    this->rules = rules;
}

void Trigger::addRule(Rule *rule)
{
    rules.push_back(rule);
}

void Trigger::removeRule(int index)
{
    rules.removeAt(index);
}

void Trigger::removeRule(Rule *rule)
{
    rules.removeOne(rule);
}

QList<Action*> Trigger::getActions()
{
    return actions;
}

void Trigger::setActions(QList<Action *> actions)
{
    this->actions = actions;
}

void Trigger::addAction(Action *action)
{
    actions.push_back(action);
}

void Trigger::removeAction(int index)
{
    actions.removeAt(index);
}

void Trigger::removeAction(Action *action)
{
    actions.removeOne(action);
}

CECDefinitionMessage* Trigger::getDefinition()
{
    return definitionMessage;
}

bool Trigger::load(QString filename, QList<Trigger*>& triggers)
{
    QDomDocument doc;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    if(!doc.setContent(&file))
    {
        file.close();

        return false;
    }

    QDomElement root = doc.documentElement();
    if(root.tagName() != "actions")
    {
        return false;
    }

    ActionFactory actionFactory;

    QDomNode nodeTrigger = root.firstChild();
    while(!nodeTrigger.isNull())
    {
        QDomElement elementTrigger = nodeTrigger.toElement();
        if(!elementTrigger.isNull() && elementTrigger.tagName() == "trigger")
        {
            //get trigger name and message
            QDomElement triggerName = elementTrigger.firstChildElement("name");
            QDomElement triggerMessage = elementTrigger.firstChildElement("message");

            if (!triggerName.isNull() && !triggerMessage.isNull())
            {
                Trigger* trigger = new Trigger();

                trigger->setName(triggerName.text());
                trigger->setMessage(triggerMessage.text().toInt());

                //loop through all nodes and find all rules and actions
                QDomNode nodeSub = elementTrigger.firstChild();
                while(!nodeSub.isNull())
                {
                    QDomElement elementSub = nodeSub.toElement();

                    //is rule
                    if(!elementSub.isNull() && elementSub.tagName() == "rule")
                    {
                        QDomElement ruleConjunction = elementSub.firstChildElement("conjunction");
                        QDomElement ruleParameter = elementSub.firstChildElement("parameter");
                        QDomElement ruleType = elementSub.firstChildElement("type");
                        QDomElement ruleValue = elementSub.firstChildElement("value");

                        if (!ruleConjunction.isNull() && !ruleParameter.isNull() && !ruleType.isNull() && !ruleValue.isNull())
                        {
                            trigger->addRule(new Rule((Rule::Conjunction)ruleConjunction.text().toInt(), ruleParameter.text().toInt(), (Rule::Type)ruleType.text().toInt(), ruleValue.text()));
                        }
                    }
                    //is action
                    else if(!elementSub.isNull() && elementSub.tagName() == "action")
                    {
                        QDomElement actionName = elementSub.firstChildElement("name");
                        QDomElement actionType = elementSub.firstChildElement("type");

                        if (!actionName.isNull() && !actionType.isNull())
                        {
                            QList<Action::Parameter> actionParameters;

                            //loop through all nodes and find all parameters
                            QDomNode nodeParameter = elementSub.firstChild();
                            while(!nodeParameter.isNull())
                            {
                                QDomElement elementParameter = nodeParameter.toElement();

                                //is parameter
                                if(!elementParameter.isNull() && elementParameter.tagName() == "parameter")
                                {
                                    QString name = elementParameter.attribute("name");
                                    QString value = elementParameter.text();

                                    if (!name.isEmpty() && !value.isEmpty())
                                    {
                                        Action::Parameter parameter;
                                        parameter.name = name;
                                        parameter.value = value;

                                        actionParameters.push_back(parameter);
                                    }
                                }

                                nodeParameter = nodeParameter.nextSibling();
                            }

                            Action* action = actionFactory.create(actionName.text(), (Action::Type)actionType.text().toInt(), actionParameters);
                            if (action != NULL)
                            {
                                trigger->addAction(action);
                            }
                        }
                    }

                    nodeSub = nodeSub.nextSibling();
                }

                triggers.push_back(trigger);
            }
        }

        nodeTrigger = nodeTrigger.nextSibling();
    }

    file.close();

    return true;
}

bool Trigger::save(QString filename, QList<Trigger*>& triggers)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("actions");
    doc.appendChild(root);

    for (int i = 0; i < triggers.size(); i++)
    {
        QDomElement trigger = doc.createElement("trigger");

        QDomElement element = doc.createElement("name");
        QDomText text = doc.createTextNode(triggers[i]->getName());
        element.appendChild(text);
        trigger.appendChild(element);

        element = doc.createElement("message");
        text = doc.createTextNode(QString::number(triggers[i]->getMessage()));
        element.appendChild(text);
        trigger.appendChild(element);

        //rules
        QList<Rule*> rules = triggers[i]->getRules();
        for (int j = 0; j < rules.size(); j++)
        {
            QDomElement rule = doc.createElement("rule");

            element = doc.createElement("conjunction");
            text = doc.createTextNode(QString::number(rules[j]->getConjunction()));
            element.appendChild(text);
            rule.appendChild(element);

            element = doc.createElement("parameter");
            text = doc.createTextNode(QString::number(rules[j]->getParameter()));
            element.appendChild(text);
            rule.appendChild(element);

            element = doc.createElement("type");
            text = doc.createTextNode(QString::number(rules[j]->getType()));
            element.appendChild(text);
            rule.appendChild(element);

            element = doc.createElement("value");
            text = doc.createTextNode(rules[j]->getValue());
            element.appendChild(text);
            rule.appendChild(element);

            trigger.appendChild(rule);
        }

        //actions
        QList<Action*> actions = triggers[i]->getActions();
        for (int j = 0; j < actions.size(); j++)
        {
            QDomElement action = doc.createElement("action");

            element = doc.createElement("name");
            text = doc.createTextNode(actions[j]->getName());
            element.appendChild(text);
            action.appendChild(element);

            element = doc.createElement("type");
            text = doc.createTextNode(QString::number(actions[j]->getType()));
            element.appendChild(text);
            action.appendChild(element);

            //parameters
            QList<Action::Parameter> parameters = actions[j]->getParameters();
            for (int k = 0; k < parameters.size(); k++)
            {
                element = doc.createElement("parameter");
                element.setAttribute("name", parameters[k].name);
                text = doc.createTextNode(parameters[k].value);
                element.appendChild(text);
                action.appendChild(element);
            }

            trigger.appendChild(action);
        }

        root.appendChild(trigger);
    }

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QTextStream ts(&file);
    ts << doc.toString();

    file.close();

    return true;
}
