#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

#include "usb.h"
#include <QThread>

class UsbController
{
private:
    class WorkerGetMessage : public QThread
    {
        public:
            void run();
    };

    class WorkerGetDebug : public QThread
    {
        public:
            void run();
    };

    static UsbController* instance;

    WorkerGetMessage workerGetMessage;
    WorkerGetDebug workerGetDebug;

    usb_dev_handle* deviceHandle;
    char vendorName[256];
    int vendorId;
    char deviceName[256];
    int deviceId;

    QMutex mutexReadWrite;

    UsbController();

public:
    static UsbController* getInstance();

    bool connect();
    bool disconnect();
    bool isConnected();

    int readData(int requestId, char* data, int size);
    int sendData(int requestId, char* data, int size);

    usb_dev_handle* getDeviceHandle();

    char* getVendorName();
    void setVendorName(char* name);

    int getVendorId();
    void setVendorId(int id);

    char* getProductName();
    void setDeviceName(char* name);

    int getProductId();
    void setDeviceId(int id);

    void startGetMessages();
    void stopGetMessages();

    void startGetDebug();
    void stopGetDebug();
};

#endif
