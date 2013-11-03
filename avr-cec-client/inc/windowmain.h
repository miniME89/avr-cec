#ifndef WINDOWMAIN_H
#define WINDOWMAIN_H

#include <QMainWindow>
#include <QItemSelection>
#include "avr_cec_lib.h"

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
    avrcec::Connector connector;

    Ui::WindowMain *ui;

    void listenerCECMessage(void* data);
    void listenerDebugMessage(void* data);
    void listenerConfig(void* data);

    void setupUi();
    void setupUiPageConnection();
    void setupUiPageActions();
    void setupUiPageSniffer();
    void setupUiPageDebug();
    void setupUiPageSettings();

private slots:
    void selectionChangedMainMenu(QItemSelection selection);
    void clickedButtonAdd();
};

#endif // WINDOWMAIN_H
