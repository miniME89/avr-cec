#ifndef WINDOWMAIN_H
#define WINDOWMAIN_H

#include <QMainWindow>
#include <QItemSelection>
#include "avrcec.h"

namespace Ui {
    class WindowMain;
}

class WindowMain : public QMainWindow
{
    Q_OBJECT

    private:
        Ui::WindowMain *ui;

        avrcec::Connector connector;

        void listenerCECMessage(void* data);
        void listenerDebugMessage(void* data);
        void listenerConfig(void* data);

        void setupUi();
        void setupUiPageConnection();
        void setupUiPageActions();
        void setupUiPageSniffer();
        void setupUiPageDebug();
        void setupUiPageSettings();

    public:
        explicit WindowMain(QWidget *parent = 0);
        ~WindowMain();

    private slots:
        void selectionChangedMainMenu(QItemSelection selection);
        void clickedButtonAdd();
};


#endif // WINDOWMAIN_H
