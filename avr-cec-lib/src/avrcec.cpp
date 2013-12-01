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
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;

namespace avrcec
{

    enum Commands
    {
        READ_CEC_MESSAGE = 1,
        SEND_CEC_MESSAGE = 2,
        READ_CONFIG = 3,
        SEND_CONFIG = 4,
        READ_DEBUG_MESSAGE = 5
    };

    //==============================================================
    // DebugMessage
    //==============================================================
    std::string DebugMessage::toString()
    {
        stringstream ss;

        for (int i = 0; i < size; i++)
        {
            ss <<data[i];
        }

        return ss.str();
    }

    //==============================================================
    // Config
    //==============================================================
    std::string Config::toString()
    {
        stringstream ss;

        //TODO implement

        return ss.str();
    }

    //==============================================================
    // Header
    //==============================================================
    Header::Header()
    {
        value = 0x00;
        definitionInitiator = NULL;
        definitionDestination = NULL;
    }

    Header::Header(const Header& obj)
    {
        value = obj.value;
        definitionInitiator = obj.definitionInitiator;
        definitionDestination = obj.definitionDestination;
    }

    Header::Header(byte header)
    {
        value = header;
        definitionInitiator = CECMessageFactory::getInstance()->getDefinitionAddress((header & 0xF0) >> 4);
        definitionDestination = CECMessageFactory::getInstance()->getDefinitionAddress(header & 0x0F);
    }

    Header::Header(byte initiator, byte destination)
    {
        initiator = initiator & 0x0F;
        destination = destination & 0x0F;
        value = (initiator << 4) | destination;
        definitionInitiator = CECMessageFactory::getInstance()->getDefinitionAddress(initiator);
        definitionDestination = CECMessageFactory::getInstance()->getDefinitionAddress(destination);
    }

    Header::~Header()
    {
    }

    byte Header::getValue()
    {
        return value;
    }

    byte Header::getInitiator()
    {
        return ((value & 0xF0) >> 4);
    }

    byte Header::getDestination()
    {
        return (value & 0x0F);
    }

    CECDefinitionAddress* Header::getDefinitionInitiator()
    {
        return definitionInitiator;
    }

    CECDefinitionAddress* Header::getDefinitionDestination()
    {
        return definitionDestination;
    }

    std::string Header::toString()
    {
        stringstream ss;

        ss <<"header: initiator=" <<definitionInitiator->getName() <<" [" <<(int)getInitiator() <<"], destination=" <<definitionDestination->getName() <<" [" <<(int)getDestination() <<"]\n";

        return ss.str();
    }

    //==============================================================
    // Opcode
    //==============================================================
    Opcode::Opcode()
    {
        value = 0;
        definition = NULL;
    }

    Opcode::Opcode(const Opcode& obj)
    {
        value = obj.value;
        definition = obj.definition;
    }

    Opcode::Opcode(byte opcode)
    {
        value = opcode;
        definition = CECMessageFactory::getInstance()->getDefinitionMessage(opcode);
    }

    Opcode::~Opcode()
    {
    }

    byte Opcode::getValue()
    {
        return value;
    }

    CECDefinitionMessage* Opcode::getDefinition()
    {
        return definition;
    }

    std::string Opcode::toString()
    {
        stringstream ss;

        ss <<"opcode: " <<definition->getName() <<" [" <<(int)value <<"]\n";

        return ss.str();
    }

    //==============================================================
    // Operand
    //==============================================================
    Operand::Operand()
    {
        definition = NULL;
    }

    Operand::Operand(std::vector<bit> operand)
    {
        value = operand;
        definition = NULL;
    }

    Operand::~Operand()
    {
    }

    std::vector<bit> Operand::getValueBits()
    {
        return value;
    }

    std::vector<byte> Operand::getValueBytes()
    {
        vector<byte> bytes(ceil(value.size() / 8));
        int shift = 0;
        for (int i = value.size() - 1; i >= 0; i--)
        {
            bytes[(int)(i / 8)] |= value[i] <<(shift % 8);
            shift++;
        }

        return bytes;
    }

    int Operand::getValueInt()
    {
        int integer;
        int shift = 0;
        for (int i = value.size() - 1; i >= 0; i--)
        {
            integer |= value[i] <<shift;
            shift++;
        }

        return 0;
    }

    int Operand::getLength()
    {
        return value.size();
    }

    CECDefinitionOperand* Operand::getDefinition()
    {
        return definition;
    }

    std::string Operand::toString()
    {
        stringstream ss;

        ss <<"operand: " <<definition->getName() <<" [";
        vector<byte> bytes = getValueBytes();
        for (unsigned int j = 0; j < bytes.size(); j++)
        {
            if (j != 0)
            {
                ss <<" ";
            }

            ss <<std::hex <<std::setfill('0') <<std::setw(2) <<(int)bytes[j];
        }

        for (map<int, std::string>::const_iterator j = definition->getOptions().begin(); j != definition->getOptions().end(); j++)
        {
            if (j->first == getValueInt())
            {
                ss <<" = " <<j->second;
                break;
            }
        }

        ss <<"]\n";

        return ss.str();
    }

    //==============================================================
    // CECMessage
    //==============================================================
    std::string CECMessage::toString()
    {
        std::stringstream ss;

        if (header != NULL)
        {
            ss <<header->toString();
        }

        if (opcode != NULL)
        {
            ss <<opcode->toString();
        }

        for (unsigned int i = 0; i < operands.size(); i++)
        {
            ss <<operands[i]->toString();
        }

        return ss.str();
    }

    //==============================================================
    // CECMessage
    //==============================================================
    CECMessage::CECMessage()
    {
        header = NULL;
        opcode = NULL;
    }

    CECMessage::CECMessage(const CECMessage& obj)
    {
        header = obj.header;
        opcode = obj.opcode;
        operands = obj.operands;
    }

    CECMessage::~CECMessage()
    {
        if (header != NULL)
        {
            delete header;
        }

        if (opcode != NULL)
        {
            delete opcode;
        }

        for (unsigned int i = 0; i < operands.size(); i++)
        {
            delete operands[i];
        }
    }

    Header* CECMessage::getHeader()
    {
        return header;
    }

    Opcode* CECMessage::getOpcode()
    {
        return opcode;
    }

    std::vector<Operand*> CECMessage::getOperands()
    {
        return operands;
    }

    CECDefinitionMessage* CECMessage::getDefinition()
    {
        return opcode->getDefinition();
    }

    //==============================================================
    // Connector
    //==============================================================
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
        while(runThreadPoll)
        {
            readCECMessage();
            readDebugMessage();

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

    CECMessage* Connector::readCECMessage()
    {
        byte data[16];
        int readBytes = readData(READ_CEC_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            CECMessage* message = CECMessageFactory::getInstance()->create(data, readBytes);

            notifyListenersCECMessage(message);

            return NULL;
        }

        return NULL;
    }

    bool Connector::sendCECMessage(CECMessage* message)
    {
        //TODO implement
        /*
        int wroteBytes = sendData(SEND_CEC_MESSAGE, message.data, message.size);

        if (wroteBytes == message.size)
        {
            return true;
        }
        */

        return false;
    }

    Config* Connector::readConfig()
    {
        byte data[8];
        int readBytes = readData(READ_DEBUG_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            //TODO implement
            Config* config = new Config;

            notifyListenersConfig(config);

            return config;
        }

        return NULL;
    }

    bool Connector::sendConfig(Config* config)
    {
        //TODO implement
        return false;
    }

    DebugMessage* Connector::readDebugMessage()
    {
        byte data[8];
        int readBytes = readData(READ_DEBUG_MESSAGE, data, sizeof(data));
        if (readBytes > 0)
        {
            DebugMessage* message = new DebugMessage;
            message->size = readBytes;

            for (int i = 0; i < readBytes; i++)
            {
                message->data[i] = data[i];
            }

            notifyListenersDebugMessage(message);

            return message;
        }

        return NULL;
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

    void Connector::notifyListenersCECMessage(CECMessage* message)
    {
        for (unsigned int i = 0; i < listenersCECMessage.size(); i++)
        {
            listenersCECMessage[i]->call(message);
        }
    }

    void Connector::notifyListenersDebugMessage(DebugMessage* message)
    {
        for (unsigned int i = 0; i < listenersDebugMessage.size(); i++)
        {
            listenersDebugMessage[i]->call(message);
        }
    }

    void Connector::notifyListenersConfig(Config* config)
    {
        for (unsigned int i = 0; i < listenersDebugMessage.size(); i++)
        {
            listenersConfig[i]->call(config);
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

    //==============================================================
    // CECDefinitionAddress
    //==============================================================
    CECDefinitionAddress::CECDefinitionAddress()
    {
    }

    int CECDefinitionAddress::getId()
    {
        return id;
    }

    const std::string& CECDefinitionAddress::getName()
    {
        return name;
    }

    std::string CECDefinitionAddress::toString()
    {
        stringstream ss;

        ss <<"address: " <<name <<" [" <<id <<"]\n";

        return ss.str();
    }

    //==============================================================
    // CECDefinitionOperand
    //==============================================================
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

    int CECDefinitionOperand::getId()
    {
        return id;
    }

    int CECDefinitionOperand::getLength()
    {
        return length;
    }

    const std::string& CECDefinitionOperand::getName()
    {
        return name;
    }

    const std::string& CECDefinitionOperand::getDescription()
    {
        return description;
    }

    const std::map<int, std::string>& CECDefinitionOperand::getOptions()
    {
        return options;
    }

    const std::map<CECDefinitionOperand*, std::vector<int> >& CECDefinitionOperand::getConstraints()
    {
        return constraints;
    }

    const CECDefinitionOperand* CECDefinitionOperand::getParent()
    {
        return parent;
    }

    const std::vector<CECDefinitionOperand*>& CECDefinitionOperand::getChilds()
    {
        return childs;
    }

    string CECDefinitionOperand::toString()
    {
        return toString(this, 1);
    }

    int CECDefinitionOperand::getPosStart()
    {
        return getPosEnd() - length;
    }

    int CECDefinitionOperand::getPosEnd()
    {
        if (parent != NULL)
        {
            return length + parent->getPosEnd();
        }

        return 0;
    }

    //==============================================================
    // CECDefinitionOpcode
    //==============================================================
    CECDefinitionMessage::CECDefinitionMessage()
    {
        id = 0;
        direct = false;
        broadcast = false;
        operands = NULL;
    }

    int CECDefinitionMessage::getId()
    {
        return id;
    }

    const std::string& CECDefinitionMessage::getName()
    {
        return name;
    }

    const std::string& CECDefinitionMessage::getDescription()
    {
        return description;
    }

    bool CECDefinitionMessage::isDirect()
    {
        return direct;
    }

    bool CECDefinitionMessage::isBroadcast()
    {
        return broadcast;
    }

    const CECDefinitionOperand* CECDefinitionMessage::getOperands()
    {
        return operands;
    }

    string CECDefinitionMessage::toString()
    {
        std::stringstream ss;

        //print message
        ss <<"opcode: " <<name <<" [id: " <<id <<", description: " <<description.substr(0, 20) <<"..., direct: " <<(direct ? "true" : "false") <<", broadcast: " <<(broadcast ? "true" : "false") <<"]"  <<"\n";

        //print operands
        ss <<operands->toString();

        return ss.str();
    }

    //==============================================================
    // CECFactory
    //==============================================================
    CECMessageFactory* CECMessageFactory::instance = NULL;

    CECMessageFactory::CECMessageFactory()
    {
        loadDefinitions();
    }

    CECMessageFactory* CECMessageFactory::getInstance()
    {
        if (instance == NULL)
        {
            instance = new CECMessageFactory();
        }

        return instance;
    }

    CECMessageFactory::~CECMessageFactory()
    {
        unloadDefinitions();
    }

    void CECMessageFactory::loadOperands(void* nodeCurrent, CECDefinitionOperand* parent)
    {
        //loop through operands
        for (xml_node<>* nodeOperand = ((xml_node<>*)nodeCurrent)->first_node("operand"); nodeOperand; nodeOperand = nodeOperand->next_sibling("operand"))
        {
            //get the operand definition
            CECDefinitionOperand* operandDefinition = getDefinitionOperand(atoi(nodeOperand->first_attribute("id")->value()));
            if (operandDefinition != NULL)
            {
                CECDefinitionOperand* operand = new CECDefinitionOperand();
                operand->id = operandDefinition->id;
                operand->length = operandDefinition->length;
                operand->name = operandDefinition->name;
                operand->description = operandDefinition->description;
                operand->options.insert(operandDefinition->options.begin(), operandDefinition->options.end());
                operand->parent = parent;
                parent->childs.push_back(operand);

                //loop through constraints
                for (xml_node<>* nodeConstraint = nodeOperand->first_node("constraint"); nodeConstraint; nodeConstraint = nodeConstraint->next_sibling("constraint"))
                {
                    //loop through previously added operands and get the dependent operand
                    int operandId = atoi(nodeConstraint->first_attribute("operand")->value());
                    CECDefinitionOperand* operandParent = operand->parent;
                    while(operandParent != NULL)
                    {
                        if (operandParent->id == operandId)
                        {
                            vector<int> values;

                            //loop through all values of the constraint
                            for (xml_node<>* nodeValue = nodeConstraint->first_node("value"); nodeValue; nodeValue = nodeValue->next_sibling("value"))
                            {
                                values.push_back(atoi(nodeValue->value()));
                            }

                            if (values.size() > 0)
                            {
                                operand->constraints.insert(pair<CECDefinitionOperand*, vector<int> >(operandParent, values));
                            }

                            break;
                        }

                        operandParent = operandParent->parent;
                    }
                }

                loadOperands(nodeOperand, operand);
            }
        }
    }

    bool CECMessageFactory::loadDefinitions()
    {
        file<> xmlFile("E:/Programmierung/Microcontroller/avr-cec/avr-cec/avr-cec-client/cec.xml");
        xml_document<> xmlDoc;
        xmlDoc.parse<0>(xmlFile.data());

        xml_node<>* nodeRoot = xmlDoc.first_node("cec");
        xml_node<>* nodeAddresses = nodeRoot->first_node("addresses");
        xml_node<>* nodeOperands = nodeRoot->first_node("operands");
        xml_node<>* nodeMessages = nodeRoot->first_node("messages");

        //loop through addresses
        for (xml_node<>* nodeAdress = nodeAddresses->first_node("address"); nodeAdress; nodeAdress = nodeAdress->next_sibling("address"))
        {
            CECDefinitionAddress* address = new CECDefinitionAddress;
            address->id = atoi(nodeAdress->first_attribute("id")->value());
            address->name = nodeAdress->first_node("name")->value();

            definitionsAddress.push_back(address);
        }

        //loop through operands
        for (xml_node<>* nodeOperand = nodeOperands->first_node("operand"); nodeOperand; nodeOperand = nodeOperand->next_sibling("operand"))
        {
            CECDefinitionOperand* operand = new CECDefinitionOperand;
            operand->id = atoi(nodeOperand->first_attribute("id")->value());
            operand->length = atoi(nodeOperand->first_attribute("length")->value());
            operand->name = nodeOperand->first_node("name")->value();
            operand->description = nodeOperand->first_node("description")->value();

            for (xml_node<>* subNode = nodeOperand->first_node("option"); subNode; subNode = subNode->next_sibling())
            {
                operand->options.insert(std::pair<int, string>(atoi(subNode->first_attribute("value")->value()), subNode->value()));
            }

            definitionsOperand.push_back(operand);
        }

        //loop through opcodes
        for (xml_node<>* nodeMessage = nodeMessages->first_node("message"); nodeMessage; nodeMessage = nodeMessage->next_sibling("message"))
        {
            CECDefinitionMessage* messageDefinition = new CECDefinitionMessage;
            messageDefinition->id = atoi(nodeMessage->first_attribute("id")->value());
            messageDefinition->name = nodeMessage->first_node("name")->value();
            messageDefinition->description = nodeMessage->first_node("description")->value();
            messageDefinition->direct = strcmp(nodeMessage->first_node("direct")->value(), "true") == 0;
            messageDefinition->broadcast = strcmp(nodeMessage->first_node("broadcast")->value(), "true") == 0;
            messageDefinition->operands = new CECDefinitionOperand;

            loadOperands(nodeMessage, messageDefinition->operands);

            definitionsMessage.push_back(messageDefinition);
            //printf("%s", messageDefinition->toString().c_str());
        }

        return true;
    }

    bool CECMessageFactory::unloadDefinitions()
    {
        for (unsigned int i = 0; i < definitionsAddress.size(); i++)
        {
            delete definitionsAddress[i];
        }

        definitionsAddress.clear();

        for (unsigned int i = 0; i < definitionsMessage.size(); i++)
        {
            /*TODO unload!
            for (unsigned int j = 0; j < opcodeDefinitions[i]->operands.size(); j++)
            {
                delete opcodeDefinitions[i]->operands[j];
            }
            */

            delete definitionsMessage[i];
        }

        definitionsMessage.clear();

        return true;
    }

    CECMessage* CECMessageFactory::create(byte* data, int size)
    {
        CECMessage* message = new CECMessage;
        message->header = new Header(data[0]);
        if (message->header->getDefinitionDestination() == NULL || message->header->getDefinitionInitiator() == NULL)
        {
            delete message->header;
            message->header = NULL;

            //TODO ERROR
            printf("ERROR: invalid header\n");
        }

        if (size > 1)
        {
            message->opcode = new Opcode(data[1]);
            if (message->opcode->getDefinition() == NULL)
            {
                delete message->opcode;
                message->opcode = NULL;

                //TODO ERROR
                printf("ERROR: invalid opcode\n");
            }
        }

        if (size > 2)
        {
            //get bits from all operands bytes (because a single operand can be smaller than 1 byte)
            vector<bit> allOperandsBits;
            int power2[] = {0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000};
            for (int i = 2; i < size; i++)
            {
                for (int j = 7; j >= 0; j--)
                {
                    allOperandsBits.push_back((data[i] & power2[j]) >> j);
                }
            }

            //loop through the operand definition tree and decode the operands
            CECDefinitionOperand* operandDefinition = message->getDefinition()->operands;
            while(operandDefinition != NULL)
            {
                if (operandDefinition->parent != NULL)
                {
                    int start = operandDefinition->getPosStart();
                    int end = operandDefinition->getPosEnd();

                    if ((end / 8) > size)
                    {
                        //TODO ERROR
                        printf("ERROR: invalid operand length\n");
                    }
                    else
                    {
                        vector<bit> operandBits(allOperandsBits.begin() + start, allOperandsBits.begin() + end);
                        Operand* operand = new Operand(operandBits);
                        operand->definition = operandDefinition;
                        message->operands.push_back(operand);
                    }
                }

                //decide which is the next operandDefinition
                bool match = false;
                for (unsigned int i = 0; i < operandDefinition->childs.size(); i++)
                {
                    //no constraints
                    if (operandDefinition->childs[i]->constraints.size() == 0)
                    {
                        match = true;
                    }

                    //loop through all constraints
                    for (map<CECDefinitionOperand*, vector<int> >::iterator j = operandDefinition->childs[i]->constraints.begin(); j != operandDefinition->childs[i]->constraints.end(); j++)
                    {
                        //loop to all previous operand values to get the depended operand
                        for (unsigned int k = 0; k < message->operands.size(); k++)
                        {
                            //is the depended operand?
                            if (message->operands[k]->getDefinition()->id == j->first->id)
                            {
                                //loop through all possible constraint values
                                for (unsigned int l = 0; l < j->second.size(); l++)
                                {
                                    //value matches with depended operand value?
                                    if (message->operands[k]->getValueInt() == j->second[l])
                                    {
                                        match = true;
                                        break;
                                    }
                                }

                                if (match)
                                {
                                    break;
                                }
                            }

                            if (match)
                            {
                                break;
                            }
                        }

                        if (match)
                        {
                            break;
                        }
                    }

                    if (match)
                    {
                        operandDefinition = operandDefinition->childs[i];

                        break;
                    }
                }

                if (!match)
                {
                    operandDefinition = NULL;
                }
            }
        }

        printf("\nraw: ");
        for (int i = 0; i < size; i++)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");

        //printf("%s", message->toString().c_str());

        return message;
    }

    CECMessage* CECMessageFactory::create(Header header, Opcode opcode)
    {
        return NULL;
    }

    CECMessage* CECMessageFactory::create(Header header, Opcode opcode, std::vector<Operand> operands)
    {
        return NULL;
    }

    std::vector<CECDefinitionAddress*> CECMessageFactory::getDefinitionsAddress()
    {
        return definitionsAddress;
    }

    std::vector<CECDefinitionMessage*> CECMessageFactory::getDefinitionsMessage()
    {
        return definitionsMessage;
    }

    std::vector<CECDefinitionOperand*> CECMessageFactory::getDefinitionsOperand()
    {
        return definitionsOperand;
    }

    CECDefinitionAddress* CECMessageFactory::getDefinitionAddress(int id)
    {
        for (unsigned int i = 0; i < definitionsAddress.size(); i++)
        {
            if (definitionsAddress[i]->id == id)
            {
                return definitionsAddress[i];
            }
        }

        return NULL;
    }

    CECDefinitionOperand* CECMessageFactory::getDefinitionOperand(int id)
    {
        for (unsigned int i = 0; i < definitionsOperand.size(); i++)
        {
            if (definitionsOperand[i]->id == id)
            {
                return definitionsOperand[i];
            }
        }

        return NULL;
    }

    CECDefinitionMessage* CECMessageFactory::getDefinitionMessage(int id)
    {
        for (unsigned int i = 0; i < definitionsMessage.size(); i++)
        {
            if (definitionsMessage[i]->id == id)
            {
                return definitionsMessage[i];
            }
        }

        return NULL;
    }

    CECDefinitionAddress* CECMessageFactory::getDefinitionAddress(std::string name)
    {
        for (unsigned int i = 0; i < definitionsAddress.size(); i++)
        {
            if (definitionsAddress[i]->name == name)
            {
                return definitionsAddress[i];
            }
        }

        return NULL;
    }

    CECDefinitionOperand* CECMessageFactory::getDefinitionOperand(std::string name)
    {
        for (unsigned int i = 0; i < definitionsOperand.size(); i++)
        {
            if (definitionsOperand[i]->name == name)
            {
                return definitionsOperand[i];
            }
        }

        return NULL;
    }

    CECDefinitionMessage* CECMessageFactory::getDefinitionMessage(std::string name)
    {
        for (unsigned int i = 0; i < definitionsMessage.size(); i++)
        {
            if (definitionsMessage[i]->name == name)
            {
                return definitionsMessage[i];
            }
        }

        return NULL;
    }
}
