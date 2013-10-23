#ifndef DIALOGACTION_H
#define DIALOGACTION_H

#include <QDialog>

namespace Ui {
class DialogAction;
}

class DialogAction : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAction(QWidget *parent = 0);
    ~DialogAction();

private:
    Ui::DialogAction *ui;
};

#endif // DIALOGACTION_H
