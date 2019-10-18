/*

SensorIoT-GW
MAIN MODULE

ESP8266 based wifi to radio Gateway
Gateway code with suport for ESP8266-based boards

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

Based on the RFM69GW project by Xose Perez

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

Forked/Updated 2017-2018 by Chris Brinton for the SensorIoT project

*/

#include <Arduino.h>
#include "config/all.h"

// -----------------------------------------------------------------------------
// Prototypes
// -----------------------------------------------------------------------------
#include <NtpClientLib.h>
#include <ESPAsyncWebServer.h>
#include <AsyncMqttClient.h>
#include <JustWifi.h>
#include <ArduinoJson.h>
#include "RadioHeadManager.h"
#include "FS.h"
#include "NodeList.h"
template<typename T> String getSetting(const String& key, T defaultValue);
template<typename T> bool setSetting(const String& key, T value);

struct _GDnode_t {
  String RSSI = "";
  String BAT = "";
  String F = "";
  String H = "";
  String P = "";
  String shortname = "";
  String longname = "";
};

struct _node_t {
  unsigned long count = 0;
  unsigned long missing = 0;
  unsigned long duplicates = 0;
  unsigned char lastPacketID = 0;
  uint8_t messageID = 0;
  uint8_t senderID = 0;
  uint8_t targetID = 0;
  uint8_t packetID = 0;
  _GDnode_t* THP;
};

_node_t nodeInfo[255];

NodeList nodeList;

//
// -----------------------------------------------------------------------------
// METHODS
// -----------------------------------------------------------------------------


String getIdentifier() {
    char identifier[20];
    sprintf(identifier, "%s_%06X", DEVICE, ESP.getChipId());
    return String(identifier);
}

void ledOn() {
    digitalWrite(LED_PIN, LOW);
}

void ledOff() {
    digitalWrite(LED_PIN, HIGH);
}

void blink(unsigned int delayms, unsigned char times = 1) {
    for (unsigned char i=0; i<times; i++) {
        if (i>0) delay(delayms);
        ledOn();
        delay(delayms);
        ledOff();
    }
}

void clearCounts() {
    for(unsigned int i=0; i<255; i++) {
        nodeInfo[i].duplicates = 0;
        nodeInfo[i].missing = 0;
    }
}

void sendSensorData() {
  sendNodeToMQTT(&nodeInfo[1]);
}

void getNicknames() {
    
    static bool nicknamesRequested = false;

    if(!nicknamesRequested) {

        // get command topic from config and replace placeholder strings
        String commandTopic = getSetting("mqttCommandTopic", MQTT_COMMAND_TOPIC);
        String tmpHN = String(getSetting("hostname", APP_NAME));
        tmpHN.replace("_","/");
        commandTopic.replace("{hostname}", tmpHN);

        if ((WiFi.status() == WL_CONNECTED) && (mqttConnected())) {
            nicknamesRequested = true;
            mqttSend((char*)commandTopic.c_str(), (char*)"get_nicknames");
            DEBUG_MSG("[DISPLAY] Nicknames requested via MQTT\n");
        
        } 
    }
}

void setNickname(char* json) {
    // Parse JSON input
    const size_t capacity = JSON_OBJECT_SIZE(3)+60;
    DynamicJsonBuffer jsonBuffer(capacity);;
    JsonObject& root = jsonBuffer.parseObject(json);

    if (!root.success()) {
        DEBUG_MSG("[REPLY] Nickname reply not deserializable. Ignoring\n");
        return;
    }
    int node_id = root["node_id"];
    const char* shortname = root["shortname"];
    const char* longname = root["longname"];
    if(nodeInfo[node_id].THP == 0) {
      nodeInfo[node_id].THP = new _GDnode_t;
    }
    nodeInfo[node_id].THP->shortname = shortname;
    nodeInfo[node_id].THP->longname = longname;

}

void processCommandReply(char* topic, char* reply) {
    String sTopic = topic;
    String sReply = reply;

    int index = sTopic.lastIndexOf('/') + 1;
    String command = sTopic.substring(index, sTopic.length());
    if(command == "get_nicknames") {
        DEBUG_MSG("[REPLY] Processing get_nicknames command response\n");
        setNickname(reply);
    } else if (command == "get_config") {
        DEBUG_MSG("[REPLY] Processing get_config command response\n");
    } else {
        DEBUG_MSG("[REPLY] Unrecognized command reply %s\n", command.c_str());
    }

}

void processMessage(packet_t * data) {

    blink(5, 1);

    DEBUG_MSG(
        "[MESSAGE] messageID:%lu senderID:%d targetID:%d packetID:%d rssi:%d\n",
        data->messageID,
        data->senderID,
        data->targetID,
        data->packetID,
        data->rssi
    );

    DEBUG_MSG("[MESSAGE] processing %d pairs\n", data->pairCount);
    for (uint8_t i=0;i<data->pairCount;i++){
      DEBUG_MSG("[MESSAGE] name: %s value: %s\n", data->pairs[i].name, data->pairs[i].value);
    }

    // Detect duplicates and missing packets
    // packetID==0 means device is not sending packetID info
    //DEBUG_MSG("[MESSAGE] checking for dups and gaps\n");
    if (data->packetID > 0) {
        if (nodeInfo[data->senderID].count > 0) {

            unsigned char gap = data->packetID - nodeInfo[data->senderID].lastPacketID;

            if (gap == 0) {
                DEBUG_MSG(" DUPLICATED\n");
                nodeInfo[data->senderID].duplicates = nodeInfo[data->senderID].duplicates + 1;
                return;
            }

            if ((gap > 1) && (data->packetID > 1)) {
                DEBUG_MSG(" MISSING PACKETS!!\n");
                nodeInfo[data->senderID].missing = nodeInfo[data->senderID].missing + gap - 1;
            }
        }

    }

    //DEBUG_MSG("[MESSAGE] Packaging into nodeInfo\n");
    nodeInfo[data->senderID].lastPacketID = data->packetID;
    nodeInfo[data->senderID].count = nodeInfo[data->senderID].count + 1;
    nodeInfo[data->senderID].senderID = data->senderID;
    nodeInfo[data->senderID].targetID = data->targetID;
    nodeInfo[data->senderID].messageID = data->messageID;
    nodeInfo[data->senderID].packetID = data->packetID;

    if(nodeInfo[data->senderID].THP == 0) {
      nodeInfo[data->senderID].THP = new _GDnode_t;
    }
    nodeInfo[data->senderID].THP->RSSI = data->rssi;

    if(!nodeList.nodeExists(data->senderID)){
      nodeList.insertNode(data->senderID);
    }
    //DEBUG_MSG("[MESSAGE] nodeInfo created\n");

    //Set the time that we got the message. If it goes too long without another update, the diplay routing will clear is out.
    nodeList.setUpdateTime(data->senderID, time(NULL));

    //DEBUG_MSG("[MESSAGE] putting name-value pairs into nodeInfo\n");
    for (uint8_t j=0;j<data->pairCount;j++) {
        if(strncmp(data->pairs[j].name,"F",1) == 0){
        nodeInfo[data->senderID].THP->F = data->pairs[j].value;
        }
        if(strncmp(data->pairs[j].name,"H",1) == 0){
        nodeInfo[data->senderID].THP->H = data->pairs[j].value;
        }
        if(strncmp(data->pairs[j].name,"P",1) == 0){
        nodeInfo[data->senderID].THP->P = data->pairs[j].value;
        }
        if(strncmp(data->pairs[j].name,"BAT",1) == 0){
        nodeInfo[data->senderID].THP->BAT = data->pairs[j].value;
        }

        // Send info to websocket clients
        char buffer[150];
        
        //DEBUG_MSG("SenderID: %u\n", data->senderID);
        //DEBUG_MSG("TargetID: %u\n", data->targetID);
        //DEBUG_MSG("PacketID: %u\n", data->packetID);
        //DEBUG_MSG("Name: %s\n", data->pairs[j].name);
        //DEBUG_MSG("Value: %s\n", data->pairs[j].value);
        //DEBUG_MSG("RSSI: %d\n", data->rssi);
        //DEBUG_MSG("Dups: %u\n", nodeInfo[data->senderID].duplicates);
        //DEBUG_MSG("Missing: %u\n", nodeInfo[data->senderID].missing);
        
        sprintf_P(
            buffer,
            PSTR("{\"packet\": {\"senderID\": %u, \"targetID\": %u, \"packetID\": %u, \"name\": \"%s\", \"value\": \"%s\", \"rssi\": %d, \"duplicates\": %u, \"missing\": %u}}"),
            data->senderID, data->targetID, data->packetID, data->pairs[j].name, data->pairs[j].value, data->rssi,
            nodeInfo[data->senderID].duplicates , nodeInfo[data->senderID].missing);
        wsSend(buffer);

      
      
    }

    sendNodeToMQTT(&nodeInfo[data->senderID]);

}

String getTopicMapping(String ID, String key) {
  String returnTopic = "BLANK";
  // Try to find a matching mapping
  unsigned int count = getSetting("mappingCount", "0").toInt();
  for (unsigned int i=0; i<count; i++) {
      if ((getSetting("nodeid" + String(i)) == ID) &&
          (getSetting("key" + String(i)) == key)) {
          returnTopic = String(String(getSetting("topic" + String(i)).c_str()) + key);
          DEBUG_MSG("[MESSAGE] getTopicMapping returning: %s\n", returnTopic.c_str());
          return returnTopic;
      }
  }

  //If we made it to here, we didnt find a mapping. Return the default topic
  returnTopic = getSetting("defaultTopic", MQTT_DEFAULT_TOPIC);
  if (returnTopic.length() > 0) {
      returnTopic.replace("{nodeid}", ID);
      returnTopic.replace("{key}", key);
      String tmpHN = String(getSetting("hostname", APP_NAME));
      tmpHN.replace("_","/");
      returnTopic.replace("{hostname}", tmpHN);
      returnTopic += "/";
      DEBUG_MSG("[MESSAGE] getTopicMapping returning default topic: %s\n", returnTopic.c_str());
      return returnTopic;
    }
    DEBUG_MSG("[MESSAGE] getTopicMapping returning: BLANK This shouldnt happen\n");
  return returnTopic;
}

void sendNodeToMQTT(_node_t * node) {

  mqttSend((char*) getTopicMapping(String(node->senderID), String("F")).c_str(), (char *) node->THP->F.c_str());
  mqttSend((char*) getTopicMapping(String(node->senderID), String("H")).c_str(), (char *) node->THP->H.c_str());
  mqttSend((char*) getTopicMapping(String(node->senderID), String("P")).c_str(), (char *) node->THP->P.c_str());
  mqttSend((char*) getTopicMapping(String(node->senderID), String("BAT")).c_str(), (char *) node->THP->BAT.c_str());
  mqttSend((char*) getTopicMapping(String(node->senderID), String("RSSI")).c_str(), (char *) node->THP->RSSI.c_str());

}

// -----------------------------------------------------------------------------
// Hardware
// -----------------------------------------------------------------------------

void hardwareSetup() {
	
    Serial.begin(SERIAL_BAUDRATE);
    while(!Serial);

    SPIFFS.begin();
    pinMode(LED_PIN, OUTPUT);
    ledOff();
	
}

void hardwareLoop() {

    // Heartbeat
    static unsigned long last_heartbeat = 0;
    if (mqttConnected()) {
        if ((millis() - last_heartbeat > HEARTBEAT_INTERVAL) || (last_heartbeat == 0)) {
            last_heartbeat = millis();
            String sTopic = getSetting("hbTopic", MQTT_HEARTBEAT_TOPIC);
            String tmpHN = String(getSetting("hostname", APP_NAME));
            tmpHN.replace("_","/");
            sTopic.replace("{hostname}", tmpHN);
            mqttSend((char *) sTopic.c_str(), (char *) String(APP_VERSION).c_str());
            DEBUG_MSG("[BEAT] Free heap: %d\n", ESP.getFreeHeap());
            DEBUG_MSG("[NTP] Time: %s\n", (char *) NTP.getTimeDateString().c_str());
        }
    }

}

void welcome() {

    delay(2000);
    Serial.printf("%s %s\n", (char *) APP_NAME, (char *) APP_VERSION);
    Serial.printf("%s\n%s\n\n", (char *) APP_AUTHOR, (char *) APP_WEBSITE);
    //Serial.printf("Device: %s\n", (char *) getIdentifier().c_str());
    Serial.printf("ChipID: %06X\n", ESP.getChipId());
    Serial.printf("Last reset reason: %s\n", (char *) ESP.getResetReason().c_str());
    Serial.printf("Memory size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    FSInfo fs_info;
    if (SPIFFS.info(fs_info)) {
        Serial.printf("File system total size: %d bytes\n", fs_info.totalBytes);
        Serial.printf("            used size : %d bytes\n", fs_info.usedBytes);
        Serial.printf("            block size: %d bytes\n", fs_info.blockSize);
        Serial.printf("            page size : %d bytes\n", fs_info.pageSize);
        Serial.printf("            max files : %d\n", fs_info.maxOpenFiles);
        Serial.printf("            max length: %d\n", fs_info.maxPathLength);
    }
    Serial.println();
    Serial.println();

}

int restartCountdown;

void restartSetup() {
    restartCountdown = 0;
}

void restartLoop() {
    static int lastMillis = 0;
    int nowMillis = millis();
    if(nowMillis > (lastMillis + 1000)) {
        lastMillis = nowMillis;
        if(restartCountdown) {
            restartCountdown--;
            DEBUG_MSG("[RESTART] Restart countdown: %i\n", restartCountdown);
            if(!restartCountdown){
                ESP.restart();
            }
        }
    }
}

void setRestartCountdown(int counter) {
    restartCountdown = counter;
    DEBUG_MSG("[RESTART] Restart countdown set to: %i\n", counter);
}

// -----------------------------------------------------------------------------
// Bootstrap methods
// -----------------------------------------------------------------------------

void setup() {
    

    hardwareSetup();

    welcome();

    settingsSetup();
    if (getSetting("hostname").length() == 0) {
        setSetting("hostname", String() + getIdentifier());
        saveSettings();
    }
    //ensure backwards compatibility
    if(getSetting("mqttEnabled", MQTT_ENABLED) == "true") {
        setSetting("mqttEnabled", "on");
        saveSettings();
    }

    wifiSetup();
    otaSetup();
    mqttSetup();
    
    radioSetup();
    webSetup();
    ntpSetup();
    sensorSetup();
    displaySetup();
    restartSetup();

}

void loop() {

    hardwareLoop();
    settingsLoop();
    wifiLoop();
    otaLoop();
    mqttLoop();
    radioLoop();
    ntpLoop();
    sensorLoop();
    displayLoop();
    restartLoop();
    getNicknames();


    yield();

}
