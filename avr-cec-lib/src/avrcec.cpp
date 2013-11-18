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
 * @file avrcec.cpp
 * @author Marcel
 * @brief
 *
 */

#include "avrcec.h"
#include "usb.h"
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;

namespace avrcec
{
    int Connector::readData(int commandId, byte* data, int size)
    {
        if (!isConnected())
        {
            return -1;
        }

        pthread_mutex_lock(&lockControlMessage);
        int num = usb_control_msg((usb_dev_handle*) deviceHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, commandId, 0, 0, (char*)data, size, 5000);
        pthread_mutex_unlock(&lockControlMessage);

        if (num < 0)
        {
            printf("USB error: %s", usb_strerror());
        }

        return num;
    }

    int Connector::sendData(int commandId, byte* data, int size)
    {
        if (!isConnected())
        {
            return -1;
        }

        pthread_mutex_lock(&lockControlMessage);
        int num = usb_control_msg((usb_dev_handle*) deviceHandle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, commandId, 0, 0, (char*)data, size, 5000);
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
            readCECMessage(&cecMessage);
            readDebugMessage(&debugMessage);

            usleep(40 * 1000);
        }

        return 0;
    }

    void Connector::startThreadPoll()
    {
        if (!runThreadPoll)
        {
            runThreadPoll = true;
            pthread_create(&threadPoll, NULL, Connector::wrapperThreadPoll, this);
        }
    }

    void Connector::stopThreadPoll()
    {
        if (runThreadPoll)
        {
            runThreadPoll = false;
            pthread_join(threadPoll, NULL);
        }
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

    Connector::~Connector()
    {
        disconnect();
        pthread_mutex_destroy(&lockControlMessage);
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

    bool Connector::readCECMessage(CECMessage* message)
    {
        byte data[16];
        int readBytes = readData(READ_CEC_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            message->setData(data, readBytes);
            CECFactory::getInstance()->decode(data, readBytes);

            notifyListenersCECMessage(*message);

            return true;
        }

        return false;
    }

    bool Connector::sendCECMessage(CECMessage message)
    {
        int wroteBytes = sendData(SEND_CEC_MESSAGE, message.data, message.size);

        if (wroteBytes == message.size)
        {
            return true;
        }

        return false;
    }

    bool Connector::readConfig(Config* config)
    {
        byte data[8];
        int readBytes = readData(READ_DEBUG_MESSAGE, data, sizeof(data));
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

    bool Connector::readDebugMessage(DebugMessage* message)
    {
        byte data[8];
        int readBytes = readData(READ_DEBUG_MESSAGE, data, sizeof(data));
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

    string CECDefinitionOperand::toString(CECDefinitionOperand* operand, int level)
    {
        std::stringstream ss;

        if (operand->parent != NULL)
        {
            for (int i = 0; i < level; i++)
            {
                ss <<"  ";
            }

            ss <<"|--> operand: " <<operand->name <<" [id: " <<operand->id <<", length: " <<operand->length <<", description: " <<operand->description.substr(0, 20) <<"...]" <<"\n";


            //print operand constraints
            for(map<CECDefinitionOperand*, vector<int> >::iterator j = operand->constraints.begin(); j != operand->constraints.end(); j++)
            {
                for (int i = 0; i < level; i++)
                {
                    ss <<"  ";
                }

                ss <<"! constraint: " <<j->first->name <<" [values: ";
                for (unsigned int k = 0; k < j->second.size(); k++)
                {
                    if (k != 0)
                    {
                        ss <<" ";
                    }

                    ss <<j->second[k];
                }
                ss <<"]\n";
            }
        }

        for (unsigned int i = 0; i < operand->childs.size(); i++)
        {
            ss <<toString(operand->childs[i], level + 1);
        }

        return ss.str();
    }

    CECDefinitionOperand::CECDefinitionOperand()
    {
        id = 0;
        length = 0;
        parent = NULL;
    }

    string CECDefinitionOperand::toString()
    {
        return toString(this, 1);
    }

    string CECDefinitionMessage::toString()
    {
        std::stringstream ss;

        //print message
        ss <<"message: " <<name <<" [id: " <<id <<", description: " <<description.substr(0, 20) <<"..., direct: " <<(direct ? "true" : "false") <<", broadcast: " <<(broadcast ? "true" : "false") <<"]"  <<"\n";

        //print operands
        ss <<operands->toString();

        return ss.str();
    }

    CECFactory* CECFactory::instance = NULL;

    CECFactory::CECFactory()
    {
        loadDefinitions();
    }

    CECFactory* CECFactory::getInstance()
    {
        if (instance == NULL)
        {
            instance = new CECFactory();
        }

        return instance;
    }

    CECFactory::~CECFactory()
    {
        unloadDefinitions();
    }

    CECDefinitionOperand* CECFactory::getOperandDefinition(void* nodeRoot, int id)
    {
        xml_node<>* operands = ((xml_node<>*)nodeRoot)->first_node("operands");

        //get the operand definition
        for (xml_node<>* node = operands->first_node("operand"); node; node = node->next_sibling())
        {
            int currentOperandId = atoi(node->first_attribute("id")->value());
            if (id == currentOperandId)
            {
                CECDefinitionOperand* operandDefinition = new CECDefinitionOperand;
                operandDefinition->id = currentOperandId;
                operandDefinition->length = atoi(node->first_attribute("length")->value());
                operandDefinition->name = node->first_node("name")->value();
                operandDefinition->description = node->first_node("description")->value();

                for (xml_node<>* subNode = node->first_node("option"); subNode; subNode = subNode->next_sibling())
                {
                    operandDefinition->options.insert(std::pair<int, string>(atoi(subNode->first_attribute("value")->value()), subNode->value()));
                }

                return operandDefinition;
            }
        }

        return NULL;
    }

    void CECFactory::getOperands(void* nodeRoot, void* nodeCurrent, CECDefinitionOperand* parent)
    {
        //loop through operands
        for (xml_node<>* nodeOperand = ((xml_node<>*)nodeCurrent)->first_node("operand"); nodeOperand; nodeOperand = nodeOperand->next_sibling())
        {
            //get the operand definition
            CECDefinitionOperand* operandDefinition = getOperandDefinition(nodeRoot, atoi(nodeOperand->first_attribute("id")->value()));
            if (operandDefinition != NULL)
            {
                operandDefinition->parent = parent;
                parent->childs.push_back(operandDefinition);

                //loop through constraints
                for (xml_node<>* nodeConstraint = nodeOperand->first_node("constraint"); nodeConstraint; nodeConstraint = nodeConstraint->next_sibling("constraint"))
                {
                    //loop through previously added operands and get the dependent operand
                    int operandId = atoi(nodeConstraint->first_attribute("operand")->value());
                    CECDefinitionOperand* operand = operandDefinition->parent;
                    while(operand != NULL)
                    {
                        if (operand->id == operandId)
                        {
                            vector<int> values;

                            //loop through all values of the constraint
                            for (xml_node<>* nodeValue = nodeConstraint->first_node("value"); nodeValue; nodeValue = nodeValue->next_sibling("value"))
                            {
                                values.push_back(atoi(nodeValue->value()));
                            }

                            if (values.size() > 0)
                            {
                                operandDefinition->constraints.insert(pair<CECDefinitionOperand*, vector<int> >(operand, values));
                            }

                            break;
                        }

                        operand = operand->parent;
                    }
                }

                getOperands(nodeRoot, nodeOperand, operandDefinition);
            }
        }
    }

    bool CECFactory::loadDefinitions()
    {
        file<> xmlFile("E:/Programmierung/Microcontroller/avr-cec/avr-cec/avr-cec-client/cec.xml");
        xml_document<> xmlDoc;
        xmlDoc.parse<0>(xmlFile.data());

        xml_node<>* nodeRoot = xmlDoc.first_node("cec");
        xml_node<>* nodeAddresses = nodeRoot->first_node("addresses");
        xml_node<>* nodeMessages = nodeRoot->first_node("messages");

        //loop through addresses
        for (xml_node<>* nodeAdress = nodeAddresses->first_node("address"); nodeAdress; nodeAdress = nodeAdress->next_sibling())
        {
            CECDefinitionAddress* address = new CECDefinitionAddress;
            address->id = atoi(nodeAdress->first_attribute("id")->value());
            address->name = nodeAdress->first_node("name")->value();

            addressDefinitions.push_back(address);
        }

        //loop through nodeMessages
        for (xml_node<>* nodeMessage = nodeMessages->first_node("message"); nodeMessage; nodeMessage = nodeMessage->next_sibling())
        {
            CECDefinitionMessage* messageDefinition = new CECDefinitionMessage;
            messageDefinition->id = atoi(nodeMessage->first_attribute("id")->value());
            messageDefinition->name = nodeMessage->first_node("name")->value();
            messageDefinition->description = nodeMessage->first_node("description")->value();
            messageDefinition->direct = strcmp(nodeMessage->first_node("direct")->value(), "true") == 0;
            messageDefinition->broadcast = strcmp(nodeMessage->first_node("broadcast")->value(), "true") == 0;

            messageDefinition->operands = new CECDefinitionOperand;
            getOperands(nodeRoot, nodeMessage, messageDefinition->operands);

            messageDefinitions.push_back(messageDefinition);
            printf("%s", messageDefinition->toString().c_str());
            fflush(stdout);
        }

        return true;
    }

    bool CECFactory::unloadDefinitions()
    {
        for (unsigned int i = 0; i < addressDefinitions.size(); i++)
        {
            delete addressDefinitions[i];
        }

        addressDefinitions.clear();

        for (unsigned int i = 0; i < messageDefinitions.size(); i++)
        {
            /*TODO unload!
            for (unsigned int j = 0; j < messageDefinitions[i]->operands.size(); j++)
            {
                delete messageDefinitions[i]->operands[j];
            }
            */

            delete messageDefinitions[i];
        }

        messageDefinitions.clear();

        return true;
    }

    CECMessage CECFactory::decode(byte* data, int size)
    {
        CECMessage message;



        return message;
    }
}

