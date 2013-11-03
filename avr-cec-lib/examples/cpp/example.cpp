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

#include "avr_cec_lib.h"
#include <stdio.h>

using namespace avrcec;

class Listeners
{
    private:
        int counterMessagesRead;

    public:
        Listeners() : counterMessagesRead(0)
        {

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
	Connector connector;
	if (connector.connect())
	{
	    printf("connected: vendorName=%s, deviceName=%s, vendorId=%i, deviceId=%i\n", connector.getVendorName(), connector.getDeviceName(), connector.getVendorId(), connector.getDeviceId());

	    Listeners listeners;
	    connector.addListenerCECMessage(&Listeners::listenerCECMessage, listeners);
	    connector.addListenerDebugMessage(&Listeners::listenerDebugMessage, listeners);
	    connector.addListenerConfig(&Listeners::listenerConfig, listeners);

	    connector.spin();
	}
	else
	{
	    printf("couldn't connect!\n");
	}

    return 0;
}
