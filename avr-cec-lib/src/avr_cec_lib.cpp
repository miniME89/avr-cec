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

/**
 * @file avr_cec_lib.cpp
 * @author Marcel
 * @brief
 *
 */

#include "avr_cec_lib.h"
#include "usb.h"
#include <stdio.h>
#include <unistd.h>

namespace avrcec
{
    int Connector::readData(int commandId, char* data, int size)
    {
        if (!isConnected())
        {
            return -1;
        }

        pthread_mutex_lock(&lockControlMessage);
        int num = usb_control_msg((usb_dev_handle*) deviceHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, commandId, 0, 0, data, size, 5000);
        pthread_mutex_unlock(&lockControlMessage);

        if (num < 0)
        {
            printf("USB error: %s", usb_strerror());
        }

        return num;
    }

    int Connector::sendData(int commandId, char* data, int size)
    {
        if (!isConnected())
        {
            return -1;
        }

        pthread_mutex_lock(&lockControlMessage);
        int num = usb_control_msg((usb_dev_handle*) deviceHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, commandId, 0, 0, data, size, 5000);
        pthread_mutex_unlock(&lockControlMessage);

        if (num < 0)
        {
            printf("USB error: %s", usb_strerror());
        }

        return num;
    }

    void* Connector::wrapperThreadPoll(void* arg)
    {
        return ((Connector*)arg)->workerThreadPoll();
    }

    void* Connector::workerThreadPoll()
    {
        CECMessage cecMessage;
        DebugMessage debugMessage;

        while(runThreadPoll)
        {
            requestCECMessage(&cecMessage);
            requestDebugMessage(&debugMessage);

            usleep(40 * 1000);
        }

        return 0;
    }

    void Connector::startThreadPoll()
    {
        runThreadPoll = true;
        pthread_create(&threadPoll, NULL, Connector::wrapperThreadPoll, this);
    }

    void Connector::stopThreadPoll()
    {
        runThreadPoll = false;
        pthread_join(threadPoll, NULL);
    }

    Connector::Connector()
    {
        deviceHandle = NULL;

        vendorName[0] = '\0';
        deviceName[0] = '\0';
        vendorId = 0;
        deviceId = 0;

        runThreadPoll = false;

        pthread_mutex_init(&lockControlMessage, NULL);
    }

    bool Connector::connect()
    {
        char vendorName[] = "avr-cec";
        char deviceName[] = "avr-cec";
        int vendorId = 0x16C0;
        int deviceId = 0x05DC;

        return connect(vendorName, deviceName, vendorId, deviceId);
    }

    bool Connector::connect(char vendorName[], char deviceName[], int vendorId, int deviceId)
    {
        if (isConnected())
        {
            return false;
        }

        usb_dev_handle *handle = NULL;

        usb_find_busses();
        usb_find_devices();

        for (struct usb_bus* bus = usb_get_busses(); bus; bus = bus->next)       //iterate over all busses
        {
            for (struct usb_device* dev = bus->devices; dev; dev = dev->next)       //iterate over all devices on bus
            {
                if ((vendorId == 0 || dev->descriptor.idVendor == vendorId) && (deviceId == 0 || dev->descriptor.idProduct == deviceId))
                {
                    char vendor[256];
                    char product[256];
                    int length;
                    handle = usb_open(dev);

                    if (!handle)
                    {
                        continue;
                    }

                    length = 0;
                    vendor[0] = 0;
                    if (dev->descriptor.iManufacturer > 0)
                    {
                        length = usb_get_string_simple(handle, dev->descriptor.iManufacturer, vendor, sizeof(vendor));
                    }

                    if (length >= 0)
                    {
                        if (strcmp(vendor, vendorName) == 0)
                        {
                            length = 0;
                            product[0] = 0;
                            if (dev->descriptor.iProduct > 0)
                            {
                                length = usb_get_string_simple(handle, dev->descriptor.iProduct, product, sizeof(product));
                            }

                            if (length >= 0)
                            {
                                if (strcmp(product, deviceName) == 0)
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

            if (handle)
            {
                break;
            }
        }

        if (handle != NULL)
        {
            deviceHandle = handle;

            strcpy(this->vendorName, vendorName);
            strcpy(this->deviceName, deviceName);
            this->vendorId = vendorId;
            this->deviceId = deviceId;

            startThreadPoll();

            return true;
        }

        return false;
    }

    bool Connector::disconnect()
    {
        if (deviceHandle != NULL)
        {
            stopThreadPoll();

            usb_close((usb_dev_handle*) deviceHandle);

            deviceHandle = NULL;
            vendorName[0] = '\0';
            deviceName[0] = '\0';
            vendorId = 0;
            deviceId = 0;

            return true;
        }

        return false;
    }

    bool Connector::isConnected()
    {
        return deviceHandle != NULL;
    }

    void Connector::spin()
    {
        pthread_join(threadPoll, NULL);
    }

    bool Connector::requestCECMessage(CECMessage* message)
    {
        char data[16];
        int readBytes = readData(REQUEST_CEC_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            message->size = readBytes;

            for (int i = 0; i < readBytes; i++)
            {
                message->data[i] = data[i];
            }

            notifyListenersCECMessage(*message);

            return true;
        }

        return false;
    }

    bool Connector::sendCECMessage(CECMessage message)
    {
        int wroteBytes = sendData(SEND_CEC_MESSAGE, (char*)message.data, message.size);

        if (wroteBytes == message.size)
        {
            return true;
        }

        return false;
    }

    bool Connector::requestConfig(Config* config)
    {
        char data[8];
        int readBytes = readData(REQUEST_DEBUG_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            //TODO implement

            notifyListenersConfig(*config);

            return true;
        }

        return false;
    }

    bool Connector::sendConfig(Config config)
    {
        //TODO implement
        return false;
    }

    bool Connector::requestDebugMessage(DebugMessage* message)
    {
        char data[8];
        int readBytes = readData(REQUEST_DEBUG_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            message->size = readBytes;

            for (int i = 0; i < readBytes; i++)
            {
                message->data[i] = data[i];
            }

            notifyListenersDebugMessage(*message);

            return true;
        }

        return false;
    }

    void Connector::addListenerCECMessage(void (*listener)(void*))
    {
        listenersCECMessage.push_back(new BindFunction(listener));
    }

    void Connector::addListenerDebugMessage(void (*listener)(void*))
    {
        listenersDebugMessage.push_back(new BindFunction(listener));
    }

    void Connector::addListenerConfig(void (*listener)(void*))
    {
        listenersConfig.push_back(new BindFunction(listener));
    }

    void Connector::notifyListenersCECMessage(CECMessage message)
    {
        for (unsigned int i = 0; i < listenersCECMessage.size(); i++)
        {
            listenersCECMessage[i]->call(&message);
        }
    }

    void Connector::notifyListenersDebugMessage(DebugMessage message)
    {
        for (unsigned int i = 0; i < listenersDebugMessage.size(); i++)
        {
            listenersDebugMessage[i]->call(&message);
        }
    }

    void Connector::notifyListenersConfig(Config config)
    {
        for (unsigned int i = 0; i < listenersDebugMessage.size(); i++)
        {
            listenersConfig[i]->call(&config);
        }
    }

    char *Connector::getVendorName()
    {
        return vendorName;
    }

    char *Connector::getDeviceName()
    {
        return deviceName;
    }

    int Connector::getVendorId()
    {
        return vendorId;
    }

    int Connector::getDeviceId()
    {
        return deviceId;
    }
}
