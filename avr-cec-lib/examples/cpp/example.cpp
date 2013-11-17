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
 * @file example.cpp
 * @author Marcel
 * @brief
 *
 */

#include "avrcec.h"
#include <stdio.h>

using namespace avrcec;

class ExampleClass
{
    private:
        Connector connector;
        int counterMessagesRead;

    public:
        ExampleClass()
        {
            counterMessagesRead = 0;

            if (connector.connect())
            {
                printf("connected: vendorName=%s, deviceName=%s, vendorId=%i, deviceId=%i\n", connector.getVendorName(), connector.getDeviceName(), connector.getVendorId(), connector.getDeviceId());

                connector.addListenerCECMessage(&ExampleClass::listenerCECMessage, this);
                connector.addListenerDebugMessage(&ExampleClass::listenerDebugMessage, this);
                connector.addListenerConfig(&ExampleClass::listenerConfig, this);

                connector.spin();
            }
            else
            {
                printf("couldn't connect!\n");
            }
        }

        void listenerCECMessage(void* data)
        {
            CECMessage* message = (CECMessage*)data;

            printf("message: %i    bytes read: %i    data: ", ++counterMessagesRead , message->size);
            for (int i = 0; i < message->size; i++)
            {
                printf("%02X ", message->data[i]);
            }
            printf("\n");
        }

        void listenerDebugMessage(void* data)
        {
            DebugMessage* message = (DebugMessage*)data;

            printf("Debug: ");
            for (int i = 0; i < message->size; i++)
            {
                printf("%c", message->data[i]);
            }
            printf("\n");
        }

        void listenerConfig(void* data)
        {

        }
};

int main(int argc, char** argv)
{
    ExampleClass obj;

    return 0;
}
