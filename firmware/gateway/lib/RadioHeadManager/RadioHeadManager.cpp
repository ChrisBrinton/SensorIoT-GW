/*

Radio

Radio Manager for SensorIoT-GW

Based on sample code by Felix Rusu - http://LowPowerLab.com/contact
Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>
Converted to use RadioHead library by Chris Brinton 2019

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

#include "RadioHeadManager.h"

#define DEBUG_PORT Serial
#ifdef DEBUG_PORT
    #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG(...)
#endif

bool RadioHeadManager::initialize(int frequency, uint8_t nodeID, const char* key, uint8_t gatewayID, int16_t targetRSSI) {

//    encrypt(key);
    _gatewayID = gatewayID;
    _targetRSSI = targetRSSI;
//    if (_gatewayID > 0) enableAutoPower(targetRSSI);
//    if (_isRFM69HW) setHighPower();

    bool ret = init();
    if(ret) {
        DEBUG_MSG("[RADIO] Initialization succeeded\n");
    } else {
        DEBUG_MSG("[RADIO] Initialization failed\n");
    }

    return ret;

}

void RadioHeadManager::onMessage(TMessageCallback fn) {
    _callback = fn;
}

void RadioHeadManager::separator0(char sep0) {
    _separator0 = sep0;
}

void RadioHeadManager::separator1(char sep1) {
    _separator1 = sep1;
}

char returnMsg[RETURN_COMMAND_LEN] = "NOP";

bool RadioHeadManager::loop() {
    
    boolean ret = false;

    if (available()) {
        uint8_t senderID;
        char buffer[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t length=RH_RF95_MAX_MESSAGE_LEN;
        uint8_t targetID = _gatewayID;
        int16_t rssi = _driver.lastRssi();
        strcpy(returnMsg,"NOP");
        if(rssi < (_targetRSSI - 5)) {
            strcpy(returnMsg,"TXPWR_UP");
        } else if (rssi > (_targetRSSI + 5)) {
            strcpy(returnMsg,"TXPWR_DN");
        }
        //DEBUG_MSG("[MESSAGE] buffer available len: %i max: %i\n", length, RH_RF95_MAX_MESSAGE_LEN);
        if (recvfromAck((uint8_t*)&buffer, &length, &senderID))
        {
            buffer[length] = 0;
            DEBUG_MSG("[MESSAGE] recv - len %i - raw msg: %s\n", length, buffer);

            uint8_t parts = 1;
            for (uint8_t i=0; i<length; i++) {
                if (buffer[i] == _separator0) ++parts;
            }
            //DEBUG_MSG("[MESSAGE] parts: %i\n", parts);

            if (parts > 1) {

                char sep[3] = {_separator0, _separator1, 0};
                char* names[NUMOFPAIRS];
                char* values[NUMOFPAIRS];
                names[0] = strtok(buffer, sep);
                values[0] = strtok(NULL, sep);
                //DEBUG_MSG("[MESSAGE] name: %s value: %s\n", names[0], values[0]);
                for (uint8_t i=1; i<parts;i++) {
                names[i] = strtok(NULL, sep);
                values[i] = strtok(NULL, sep);
                //DEBUG_MSG("[MESSAGE] name: %s value: %s\n", names[i], values[i]);
                }
    //            uint8_t packetID = 0;
    //            if (parts > 2) {
    //                char * packet = strtok(NULL, sep);
    //                packetID = atoi(packet);
    //            }

                _message.messageID = ++_receiveCount;
                _message.packetID = _receiveCount;
                _message.senderID = senderID;
                _message.targetID = targetID;
                _message.pairCount = parts;
                for (uint8_t i=0; i<parts; i++) {
                _message.pairs[i].name = names[i];
                _message.pairs[i].value = values[i];
                }
                _message.rssi = rssi;
                ret = true;

                if (_callback != NULL) {
                    _callback(&_message);
                }

            }
                  // Send a reply back to the originator client
            DEBUG_MSG("[MESSAGE] Sending control msg: %s to node: %i\n", returnMsg, senderID);
            if (!sendtoWait((uint8_t *)returnMsg, sizeof(returnMsg), senderID)) {
                DEBUG_MSG("[MESSAGE] Failed to send msg to node\n");
            } else {
                DEBUG_MSG("[MESSAGE] Control msg sent successfully. Target RSSI: %i\n", _targetRSSI);
            }
        }
    }

    return ret;

}

bool RadioHeadManager::send(uint8_t destinationID, char * name, char * value, uint8_t retries, bool requestACK) {

    char message[RH_RF95_MAX_MESSAGE_LEN];

    #if SEND_PACKET_ID
        if (++_sendCount == 0) _sendCount = 1;
        snprintf(message, RH_RF95_MAX_MESSAGE_LEN-1, "%s%c%s%c%d", name, _separator0, value, _separator0, _sendCount);
    #else
        snprintf(message, RH_RF95_MAX_MESSAGE_LEN-1, "%s%c%s", name, _separator0, value);
    #endif

    #if RADIO_DEBUG
        DEBUG_MSG("[RADIO] Sending: %s\n", message);
    #endif

    bool ret = true;
    setRetries(retries);
    ret = sendtoWait((uint8_t*)&message, strlen(message), destinationID);

    #if RADIO_DEBUG
        if (ret) {
            DEBUG_MSG(" OK\n");
        } else {
            DEBUG_MSG(" KO\n");
        }
    #endif

    return ret;

}
