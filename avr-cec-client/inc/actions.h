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

#ifndef ACTIONS_H
#define ACTIONS_H

#include <QString>
#include <QList>

class Action
{
    public:
        typedef struct Parameter
        {
            QString name;
            QString value;
        } Parameter;

        typedef enum Type
        {
            KEY = 0
        } Type;

    protected:
        QString name;
        Type type;
        QList<Parameter> parameters;

    public:
        Action(QString name, Type type, QList<Parameter> parameters);
        virtual ~Action();

        virtual void execute() = 0;

        QString getName();
        void setName(QString name);

        Type getType();
        void setType(Type type);

        QList<Parameter> getParameters();
        void setParameters(QList<Parameter> parameters);
};

class ActionKey : public Action
{
    public:
        ActionKey(QString name, Action::Type type, QList<Parameter> parameters);

        void execute();
};

class ActionFactory
{
    public:
        ActionFactory();

        Action* create(QString name, Action::Type type, QList<Action::Parameter> parameters);
};

class Rule
{
    public:
        typedef enum Conjunction
        {
            AND = 0,
            OR = 1
        } Conjunction;

        typedef enum Type
        {
            CONTAINS = 0,
            CONTAINSNOT = 1
        } Type;

    private:
        Conjunction conjunction;
        int parameter;
        Type type;
        QString match;

    public:
        Rule();
        Rule(Conjunction conjunction, int parameter, Type type, QString match);
        ~Rule();

        Conjunction getConjunction();
        void setConjunction(Conjunction conjunction);

        int getParameter();
        void setParameter(int parameter);

        Type getType();
        void setType(Type type);

        QString getMatch();
        void setMatch(QString match);
};

class Trigger
{
    private:
        QString name;
        int message;
        QList<Rule*> rules;
        QList<Action*> actions;

    public:
        Trigger();
        Trigger(QString name, int message, QList<Rule*> rules, QList<Action*> actions);

        QString getName();
        void setName(QString name);

        int getMessage();
        void setMessage(int message);

        QList<Rule*> getRules();
        void setRules(QList<Rule*> rules);
        void addRule(Rule* rule);

        QList<Action*> getActions();
        void setActions(QList<Action*> actions);
        void addAction(Action* action);

        static bool load(QList<Trigger*> &triggers);
        static bool save(QList<Trigger*>& triggers);
};

#endif // ACTIONS_H
