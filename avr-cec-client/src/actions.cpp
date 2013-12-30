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

Rule::Rule(Rule::Conjunction conjunction, int parameter, Rule::Type type, QString match)
{

}

Rule::~Rule()
{

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

QString Rule::getMatch()
{
    return match;
}

void Rule::setMatch(QString match)
{
    this->match = match;
}

//==================================================
// Trigger
//==================================================
Trigger::Trigger()
{

}

Trigger::Trigger(QString name, int message, QList<Rule*> rules, QList<Action*> actions)
{
    this->name = name;
    this->message = message;
    this->rules = rules;
    this->actions = actions;
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

bool Trigger::load(QList<Trigger*>& triggers)
{
    QDomDocument doc;
    QFile file("actions.xml");
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
                        QDomElement ruleMatch = elementSub.firstChildElement("match");

                        if (!ruleConjunction.isNull() && !ruleParameter.isNull() && !ruleType.isNull() && !ruleMatch.isNull())
                        {
                            qDebug() <<"found rule";
                            trigger->addRule(new Rule((Rule::Conjunction)ruleConjunction.text().toInt(), ruleParameter.text().toInt(), (Rule::Type)ruleType.text().toInt(), ruleMatch.text()));
                        }
                    }
                    //is action
                    else if(!elementSub.isNull() && elementSub.tagName() == "action")
                    {
                        QDomElement actionName = elementSub.firstChildElement("name");
                        QDomElement actionType = elementSub.firstChildElement("type");

                        if (!actionName.isNull() && !actionType.isNull())
                        {
                            qDebug() <<"found action";

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
                                        qDebug() <<"found parameter";

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

bool Trigger::save(QList<Trigger*> &triggers)
{
    return true;
}
