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
    typedef bool bit;
    typedef unsigned char byte;

    //forward declaration
    class CECDefinitionAddress;
    class CECDefinitionOperand;
    class CECDefinitionMessage;

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
            byte value;
            CECDefinitionAddress* definitionInitiator;
            CECDefinitionAddress* definitionDestination;

        public:
            Header();
            Header(Header& obj);
            Header(byte header);
            Header(byte initiator, byte destination);
            ~Header();

            byte getValue();
            byte getInitiator();
            byte getDestination();
            CECDefinitionAddress* getDefinitionInitiator();
            CECDefinitionAddress* getDefinitionDestination();
    };

    /**
     *
     */
    class Opcode
    {
        private:
            byte value;
            CECDefinitionMessage* definition;

        public:
            Opcode();
            Opcode(Opcode& obj);
            Opcode(byte opcode);
            ~Opcode();

            byte getValue();
            CECDefinitionMessage* getDefinition();
    };

    /**
     *
     */
    class Operand
    {
        private:
            friend class CECMessageFactory;

            std::vector<bit> value;
            CECDefinitionOperand* definition;

        public:
            Operand();
            Operand(std::vector<bit> operand);
            ~Operand();

            std::vector<bit> getValue();
            int getLength();
            CECDefinitionOperand* getDefinition();
    };

    class CECMessage
    {
        private:
            friend class CECMessageFactory;

            Header header;
            Opcode opcode;
            std::vector<Operand> operands;

            CECMessage(Header header, Opcode opcode);
            CECMessage(Header header, Opcode opcode, std::vector<Operand> operands);

        public:
            CECMessage(CECMessage& obj);
            ~CECMessage();

            Header getHeader();
            Opcode getOpcode();
            std::vector<Operand> getOperands();
            CECDefinitionMessage* getDefinition();

            std::string toString();

            //TODO remove
            CECMessage();
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
        private:
            std::string toString(CECDefinitionOperand* operand, int level);

        public:
            int id;
            int length;
            std::string name;
            std::string description;
            std::map<int, std::string> options;
            std::map<CECDefinitionOperand*, std::vector<int> > constraints;
            CECDefinitionOperand* parent;
            std::vector<CECDefinitionOperand*> childs;

            CECDefinitionOperand();
            std::string toString();
            int getPosStart();
            int getPosEnd();
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
            CECDefinitionOperand* operands;

            std::string toString();
    };

    /**
     *
     */
    class CECMessageFactory
    {
        private:
            static CECMessageFactory* instance;

            std::vector<CECDefinitionAddress*> definitionsAddress;
            std::vector<CECDefinitionOperand*> definitionsOperand;
            std::vector<CECDefinitionMessage*> definitionsMessage;

            CECMessageFactory();

            void loadOperands(void* nodeCurrent, CECDefinitionOperand* parent);

            bool loadDefinitions();
            bool unloadDefinitions();

        public:
            static CECMessageFactory* getInstance();
            ~CECMessageFactory();

            CECMessage create(byte* data, int size);
            CECMessage create(Header header, Opcode opcode);
            CECMessage create(Header header, Opcode opcode, std::vector<Operand> operands);

            std::vector<CECDefinitionAddress*> getDefinitionsAddress();
            std::vector<CECDefinitionOperand*> getDefinitionsOperand();
            std::vector<CECDefinitionMessage*> getDefinitionsMessage();

            CECDefinitionAddress* getDefinitionAddress(int id);
            CECDefinitionOperand* getDefinitionOperand(int id);
            CECDefinitionMessage* getDefinitionMessage(int id);

            CECDefinitionAddress* getDefinitionAddress(std::string name);
            CECDefinitionOperand* getDefinitionOperand(std::string name);
            CECDefinitionMessage* getDefinitionMessage(std::string name);
    };
}

#endif
