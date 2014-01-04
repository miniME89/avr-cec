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

#ifndef TABACTIONS_H
#define TABACTIONS_H

#include <QObject>
#include <QItemSelection>
#include "actions.h"

class WindowMain;

class TabActions : public QObject
{
    Q_OBJECT

    private:
        WindowMain* window;

        QList<Trigger*> triggers;
        QString triggersFilename;

        void addTriggerRule(Rule *rule);
        void removeTriggerRulesAll();
        void load(QString filename);
        void save(QString filename);

        void updateTree();
        void updateForm(Trigger* trigger);
        void updateForm(Action* action);

        Trigger* getSelectedTrigger();
        Action* getSelectedAction();
        int getSelectedTriggerIndex();
        int getSelectedActionIndex();

        void setSelection(int trigger, int action = -1);

    public:
        TabActions(WindowMain* window);
        ~TabActions();

        void setupUi();

    private slots:
        void eventTriggerShowContextMenu(const QPoint& pos);

        void eventTriggerMessageChanged(int index);
        void eventTriggerConjunctionChanged(int index);
        void eventTriggerParameterChanged(int index);
        void eventTriggerTypeChanged(int index);
        void eventTriggerOptionChanged(int index);

        void eventActionTypeChanged(int index);

        void eventImport();
        void eventExport();

        void eventAddTrigger();
        void eventAddTriggerRule();
        void eventAddAction();

        void eventRemove();
        void eventRemoveTrigger();
        void eventRemoveTriggerRule();
        void eventRemoveAction();

        void eventTreeActionsSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
};

#endif // TABACTIONS_H
