#ifndef WINDOWMAIN_H
#define WINDOWMAIN_H

#include <QMainWindow>
#include <QItemSelection>

namespace Ui {
    class WindowMain;
}

class WindowMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit WindowMain(QWidget *parent = 0);
    ~WindowMain();

private:
    Ui::WindowMain *ui;
    void setupUi();
    void setupUiPageConnection();
    void setupUiPageActions();
    void setupUiPageSniffer();
    void setupUiPageSettings();

private slots:
    void selectionChangedMainMenu(QItemSelection selection);
    void clickedButtonAdd();
};

#endif // WINDOWMAIN_H
