#include "usbcontroller.h"
#include <QDebug>

#define COMMAND_GET_MESSAGE     1
#define COMMAND_PUT_MESSAGE     2
#define COMMAND_GET_CONFIG      3
#define COMMAND_PUT_CONFIG      4
#define COMMAND_GET_DEBUG       5

int counterMessagesRead = 0;
int counterBytesRead = 0;

void UsbController::WorkerGetMessage::run()
{
    char data[16];
    int readBytes;

    while(true)
    {
        readBytes = UsbController::getInstance()->readData(COMMAND_GET_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            QString dataStr = QString(QByteArray(data, readBytes).toHex()).toUpper();
            for (int i = 2; i < dataStr.size(); i = i + 2)
            {
                dataStr.insert(i, ' ');
                i++;
            }

            counterBytesRead += readBytes;
            qDebug() <<"total bytes read: " <<counterBytesRead <<" message: " <<++counterMessagesRead <<" bytes read: "<<readBytes <<" data: " <<dataStr;
            msleep(40);
        }
        else
        {
            msleep(100);
        }
    }
}

void UsbController::WorkerGetDebug::run()
{
    char data[256];
    int readBytes;

    while(true)
    {
        readBytes = UsbController::getInstance()->readData(COMMAND_GET_DEBUG, data, sizeof(data));
        if (readBytes > 0)
        {
            data[readBytes] = '\0';
            qDebug() <<"Debug: " <<QString(data);
            msleep(40);
        }
        else
        {
            msleep(100);
        }
    }
}

UsbController* UsbController::instance = NULL;

UsbController::UsbController()
{
    deviceHandle = NULL;

    setVendorName("avr-cec");
    setVendorId(0x16C0);
    setDeviceName("avr-cec");
    setDeviceId(0x05DC);
}

UsbController *UsbController::getInstance()
{
    if (instance == NULL)
    {
        instance = new UsbController();
    }

    return instance;
}

bool UsbController::connect()
{
    if (isConnected())
    {
        return false;
    }

    usb_dev_handle *handle = NULL;

    usb_find_busses();
    usb_find_devices();

    for(struct usb_bus* bus = usb_get_busses(); bus; bus = bus->next)       //iterate over all busses
    {
        for(struct usb_device* dev = bus->devices; dev; dev = dev->next)       //iterate over all devices on bus
        {
            if((vendorId == 0 || dev->descriptor.idVendor == vendorId) && (deviceId == 0 || dev->descriptor.idProduct == deviceId))
            {
                char vendor[256];
                char product[256];
                int length;
                handle = usb_open(dev);

                if(!handle)
                {
                    continue;
                }

                length = 0;
                vendor[0] = 0;
                if(dev->descriptor.iManufacturer > 0)
                {
                    length = usb_get_string_simple(handle, dev->descriptor.iManufacturer, vendor, sizeof(vendor));
                }

                if(length >= 0)
                {
                    if(strcmp(vendor, vendorName) == 0)
                    {
                        length = 0;
                        product[0] = 0;
                        if(dev->descriptor.iProduct > 0)
                        {
                            length = usb_get_string_simple(handle, dev->descriptor.iProduct, product, sizeof(product));
                        }

                        if(length >= 0)
                        {
                            if(strcmp(product, deviceName) == 0)
                            {
                                break;
                            }
                        }
                    }
                }

                usb_close(handle);
                handle = NULL;
            }
        }

        if(handle)
        {
            break;
        }
    }

    if(handle != NULL)
    {
        deviceHandle = handle;

        startGetMessages();
        startGetDebug();

        return true;
    }

    return false;
}

bool UsbController::disconnect()
{
    if (deviceHandle != NULL)
    {
        stopGetMessages();

        usb_close(deviceHandle);
        deviceHandle = NULL;

        return true;
    }

    return false;
}

bool UsbController::isConnected()
{
    return deviceHandle != NULL;
}

int UsbController::readData(int requestId, char* data, int size)
{
    if (!isConnected())
    {
        return 0;
    }

    mutexReadWrite.lock();
    int num = usb_control_msg(deviceHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, requestId, 0, 0, data, size, 5000);
    mutexReadWrite.unlock();

    if(num < 0)
    {
        qDebug() <<"USB error: " <<usb_strerror();
    }

    return num;
}

int UsbController::sendData(int requestId, char* data, int size)
{
    if (!isConnected())
    {
        return 0;
    }

    mutexReadWrite.lock();
    int num = usb_control_msg(deviceHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, requestId, 0, 0, data, size, 5000);
    mutexReadWrite.unlock();

    if(num < 0)
    {
        qDebug() <<"USB error: " <<usb_strerror();
    }

    return num;
}

usb_dev_handle* UsbController::getDeviceHandle()
{
    return deviceHandle;
}

char *UsbController::getVendorName()
{
    return vendorName;
}

void UsbController::setVendorName(char *name)
{
    strcpy_s(vendorName, name);
}

int UsbController::getVendorId()
{
    return vendorId;
}

void UsbController::setVendorId(int id)
{
    vendorId = id;
}

char *UsbController::getProductName()
{
    return deviceName;
}

void UsbController::setDeviceName(char *name)
{
    strcpy_s(deviceName, name);
}

int UsbController::getProductId()
{
    return deviceId;
}

void UsbController::setDeviceId(int id)
{
    deviceId = id;
}

void UsbController::startGetMessages()
{
    workerGetMessage.start();
}

void UsbController::stopGetMessages()
{

}

void UsbController::startGetDebug()
{
    workerGetDebug.start();
}

void UsbController::stopGetDebug()
{

}
