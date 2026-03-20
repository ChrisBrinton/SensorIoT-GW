/*

Radio

Radio Manager for SensorIoT-GW

Based on sample code by Felix Rusu - http://LowPowerLab.com/contact
Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>
Converted to RadioHead library by Chris Brinton

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef RadioHeadManager_h
#define RadioHeadManager_h

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define PING_EVERY          3
#define RETRIES             2
#define REQUESTACK          1
#define RADIO_DEBUG         1
#define SEND_PACKET_ID      1
#define PACKET_SEPARATOR0   ' '
#define PACKET_SEPARATOR1   ':'
#define NUMOFPAIRS          10
#define RETURN_COMMAND_LEN  15

typedef struct {
    char * name;
    char * value;
} pair_t;

typedef struct {
    unsigned long messageID;
    unsigned char packetID;
    unsigned char senderID;
    unsigned char targetID;
    uint8_t pairCount;
    pair_t pairs[NUMOFPAIRS];
    int16_t rssi;
} packet_t;

typedef void (*TMessageCallback)(packet_t *);

class RadioHeadManager: public RHReliableDatagram {

    public:
//uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t radioType, uint8_t interruptNum=0
        RadioHeadManager(RHGenericDriver &driver, uint8_t thisAddress):
            RHReliableDatagram(driver, thisAddress) {};

        bool initialize(int frequency, uint8_t nodeID, const char* key, uint8_t gatewayID = 0, int16_t targetRSSI = -70);
        void onMessage(TMessageCallback fn);
        void separator0(char sep);
        void separator1(char sep);
        bool send(uint8_t destinationID, char * name, char * value, uint8_t retries = RETRIES, bool requestACK = REQUESTACK);
        bool send(char * name, char * value, uint8_t retries = RETRIES) {
            return send(_gatewayID, name, value, retries, false);
        }
        bool send(char * name, char * value, bool requestACK = REQUESTACK) {
            return send(_gatewayID, name, value, 0, requestACK);
        }
        bool loop();
        packet_t * getMessage() {
            return &_message;
        }

    protected:
        uint8_t _slaveSelectPin;
        uint8_t _interruptPin;
        uint8_t _radioType;
        uint8_t _interruptNum;
        int16_t _targetRSSI;
        packet_t _message;
        TMessageCallback _callback = NULL;
        uint8_t _gatewayID = 0;
        unsigned long _receiveCount = 0;
        #if SEND_PACKET_ID
            unsigned char _sendCount = 0;
        #endif
        unsigned int _ackCount = 0;
        char _separator0 = PACKET_SEPARATOR0;
        char _separator1 = PACKET_SEPARATOR1;

};
#endif