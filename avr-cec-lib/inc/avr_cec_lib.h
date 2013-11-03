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
 * @file avr_cec_lib.h
 * @author Marcel
 * @brief
 *
 */

#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

#include <pthread.h>
#include <vector>

namespace avrcec
{
    enum Commands
    {
        REQUEST_CEC_MESSAGE = 1,
        SEND_CEC_MESSAGE = 2,
        REQUEST_CONFIG = 3,
        SEND_CONFIG = 4,
        REQUEST_DEBUG_MESSAGE = 5
    };

    typedef struct CECMessage
    {
        unsigned char data[16];
        int size;
    } CECMessage;

    typedef struct DebugMessage
    {
        unsigned char data[8];
        int size;
    } DebugMessage;

    typedef struct Config
    {
        //...
    } Config;

    /**
     * Abstract class to register a callback. The derived classes BindFunction and BindMember are used to hold function pointers and member function pointers.
     */
    class Bind
    {
        public:
            virtual ~Bind() { }
            virtual void call(void* data) = 0;
    };

    /**
     * This class holds a function pointer and can call the corresponding function. This class will be used to register a callback.
     */
    class BindFunction : public Bind
    {
        private:
            void (*function)(void*);
        public:
            BindFunction(void (*function)(void*)) : function(function) { }
            void call(void* data) { function(data); }
    };

    /**
     * This class holds a member function pointer and can call the corresponding member function. This class will be used to register a callback.
     */
    template <typename T>
    class BindMember : public Bind
    {
        private:
            T& object;
            void (T::*member)(void*);

        public:
            BindMember(void (T::*member)(void*), T& object) : member(member), object(object) { }
            void call(void* data) { ((object).*(member))(data); }
    };

    /**
     * USB connector class.
     */
    class Connector
    {
        private:
            void* deviceHandle;

            char vendorName[256];
            char deviceName[256];
            int vendorId;
            int deviceId;

            std::vector<Bind*> listenersCECMessage;
            std::vector<Bind*> listenersDebugMessage;
            std::vector<Bind*> listenersConfig;

            bool runThreadPoll;
            pthread_t threadPoll;

            pthread_mutex_t lockControlMessage;

            int readData(int commandId, char* data, int size);
            int sendData(int commandId, char* data, int size);

            static void* wrapperThreadPoll(void* arg);
            void* workerThreadPoll();

            void startThreadPoll();
            void stopThreadPoll();

        public:
            Connector();

            bool connect();
            bool connect(char vendorName[], char deviceName[], int vendorId, int deviceId);
            bool disconnect();
            bool isConnected();

            void spin();

            bool requestCECMessage(CECMessage* message);
            bool sendCECMessage(CECMessage message);
            bool requestConfig(Config* config);
            bool sendConfig(Config config);
            bool requestDebugMessage(DebugMessage* message);

            void addListenerCECMessage(void (*listener)(void*));
            void addListenerDebugMessage(void (*listener)(void*));
            void addListenerConfig(void (*listener)(void*));

            template <typename T>
            void addListenerCECMessage(void (T::*member)(void*), T& object)
            {
                listenersCECMessage.push_back(new BindMember<T>(member, object));
            }
            template <typename T>
            void addListenerDebugMessage(void (T::*member)(void*), T& object)
            {
                listenersDebugMessage.push_back(new BindMember<T>(member, object));
            }
            template <typename T>
            void addListenerConfig(void (T::*member)(void*), T& object)
            {
                listenersConfig.push_back(new BindMember<T>(member, object));
            }

            void notifyListenersCECMessage(CECMessage message);
            void notifyListenersDebugMessage(DebugMessage message);
            void notifyListenersConfig(Config config);

            char* getVendorName();
            char* getDeviceName();
            int getVendorId();
            int getDeviceId();
    };
}

#endif
