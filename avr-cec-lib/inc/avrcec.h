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
 * @file avrcec.h
 * @author Marcel
 * @brief
 *
 */

#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

#include <pthread.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>

namespace avrcec
{
    typedef unsigned char byte;

    enum Commands
    {
        READ_CEC_MESSAGE = 1,
        SEND_CEC_MESSAGE = 2,
        READ_CONFIG = 3,
        SEND_CONFIG = 4,
        READ_DEBUG_MESSAGE = 5
    };

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
            void (T::*member)(void*);
            T* object;

        public:
            BindMember(void (T::*member)(void*), T* object) : member(member), object(object) { }
            void call(void* data) { ((*object).*(member))(data); }
    };

    class DebugMessage
    {
        public:
            unsigned char data[8];
            int size;
    };

    class Config
    {
        //...
    };

    /**
     *
     */
    class Header
    {
        private:
            byte header;

        public:
            Header();
            Header(byte header);
            Header(byte initiator, byte destination);
            ~Header();

            byte getHeader();
            void setHeader(byte header);

            byte getInitiator();
            void setInitiator(byte initiator);
            byte getDestination();
            void setDestination(byte destination);
    };

    /**
     *
     */
    class Opcode
    {
        private:
            byte opcode;

        public:
            Opcode();
            Opcode(byte opcode);
            ~Opcode();

            byte getOpcode();
            void setOpcode(byte opcode);
    };

    /**
     *
     */
    class Operand
    {
        private:
            byte* operand;
            int length;

        public:
            Operand();
            Operand(byte* operand, int length);
            ~Operand();

            byte* getOperand();
            void setOperand(byte* operand, int length);

            int getLength();
            void setLength(int length);
    };

    /**
     *
     */
    class CECMessage
    {
        private:
            //Header header;
            //Opcode opcode;
            //std::vector<Operand> operands;

        public:
            byte data[16];
            int size;

            void setData(byte* data, int size) {memcpy(this->data, data, size); this->size = size;}
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

            int readData(int commandId, byte* data, int size);
            int sendData(int commandId, byte* data, int size);

            static void* wrapperThreadPoll(void* arg);
            void* workerThreadPoll();

            void startThreadPoll();
            void stopThreadPoll();

        public:
            Connector();
            ~Connector();

            bool connect();
            bool connect(char vendorName[], char deviceName[], int vendorId, int deviceId);
            bool disconnect();
            bool isConnected();

            void spin();

            bool readCECMessage(CECMessage* message);
            bool sendCECMessage(CECMessage message);
            bool readConfig(Config* config);
            bool sendConfig(Config config);
            bool readDebugMessage(DebugMessage* message);

            void addListenerCECMessage(void (*listener)(void*));
            void addListenerDebugMessage(void (*listener)(void*));
            void addListenerConfig(void (*listener)(void*));

            template <typename T>
            void addListenerCECMessage(void (T::*member)(void*), T* object)
            {
                listenersCECMessage.push_back(new BindMember<T>(member, object));
            }
            template <typename T>
            void addListenerDebugMessage(void (T::*member)(void*), T* object)
            {
                listenersDebugMessage.push_back(new BindMember<T>(member, object));
            }
            template <typename T>
            void addListenerConfig(void (T::*member)(void*), T* object)
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

    /**
     *
     */
    class CECDefinitionAddress
    {
        public:
            int id;
            std::string name;
    };

    /**
     *
     */
    class CECDefinitionOperand
    {
        public:
            int id;
            int length;
            std::string name;
            std::string description;
            std::map<int, std::string> options;
            std::map<CECDefinitionOperand*, std::vector<int> > constraints;
    };

    /**
     *
     */
    class CECDefinitionMessage
    {
        public:
            int id;
            std::string name;
            std::string description;
            bool direct;
            bool broadcast;
            std::vector<CECDefinitionOperand*> operands;

            std::string toString();
    };

    /**
     *
     */
    class CECFactory
    {
        private:
            static CECFactory* instance;

            std::vector<CECDefinitionAddress*> addressDefinitions;
            std::vector<CECDefinitionMessage*> messageDefinitions;

            CECFactory();

            bool loadDefinitions();
            bool unloadDefinitions();

        public:
            static CECFactory* getInstance();
            ~CECFactory();

            CECMessage decode(byte* data, int size);
    };
}

#endif
