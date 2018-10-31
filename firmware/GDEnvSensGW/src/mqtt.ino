/*

MQTT MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>

AsyncMqttClient mqtt;
unsigned int connectTries = 0;

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

bool mqttConnected() {
    return mqtt.connected();
}

void mqttDisconnect() {
    mqtt.disconnect();
}

void mqttSend(char * topic, char * message) {
    if (!mqtt.connected()) return;
    DEBUG_MSG("[MQTT] Sending %s%s\n", topic, message);
    mqtt.publish(topic, MQTT_QOS, MQTT_RETAIN, message);
}

void _mqttOnConnect(bool sessionPresent) {

    DEBUG_MSG("[MQTT] Connected!\n");
    connectTries = 0;
    // Send status via webSocket
    wsSend((char *) "{\"mqttStatus\": true}");

    String sTopic = getSetting("ipTopic", MQTT_IP_TOPIC);
    String tmpHN = String(getSetting("hostname", APP_NAME));
    tmpHN.replace("_","/");
    sTopic.replace("{hostname}", tmpHN);

    // Say hello and report our IP and VERSION
    mqttSend((char *) sTopic.c_str(), (char *) getIP().c_str());

}

void _mqttOnDisconnect(AsyncMqttClientDisconnectReason reason) {

    // Send status via webSocket
    wsSend((char *) "{\"mqttStatus\": false}");
    DEBUG_MSG("[MQTT] Disconnected!!! Reason:%i\n", reason);

}

void mqttConnect() {

    if (!mqtt.connected()) {

        // Last option: reconnect to wifi after MQTT_MAX_TRIES attemps in a row
        static unsigned long last_try = millis();
        if (millis() - last_try < MQTT_TRY_INTERVAL) {
            if (++connectTries >= MQTT_MAX_TRIES) {
                DEBUG_MSG("[MQTT] Failed to connect to MQTT broker. Max tries exceeded\n");
                displayMsg((char*)"MQTT Connect failed. Restarting");
                wifiDisconnect();
                connectTries = 0;
                return;
            }
        } else {
            connectTries = 0;
        }
        last_try = millis();

		mqtt.disconnect();

        String host = getSetting("mqttServer", MQTT_SERVER);
        String port = getSetting("mqttPort", String(MQTT_PORT));
        String user = getSetting("mqttUser");
        String pass = getSetting("mqttPassword");

		if (host.length() == 0) return;

        DEBUG_MSG("[MQTT] Connecting to broker at %s:%s", (char *) host.c_str(), (char *) port.c_str());

        mqtt.setServer(host.c_str(), port.toInt());
        mqtt
            .setKeepAlive(MQTT_KEEPALIVE)
            .setCleanSession(false)
            //.setWill("topic/online", 2, true, "no")
            .setClientId(getSetting("hostname", HOSTNAME).c_str());

        if ((user != "") & (pass != "")) {
            DEBUG_MSG(" as user %s with pass |%s|\n", (char *) user.c_str(), (char *) pass.c_str());
            mqtt.setCredentials(user.c_str(), pass.c_str());
        } else {
            DEBUG_MSG(" anonymously\n");
        }

        mqtt.connect();

    }

}

void mqttSetup() {
    mqtt.onConnect(_mqttOnConnect);
    mqtt.onDisconnect(_mqttOnDisconnect);
}

void mqttLoop() {

    static unsigned long lastPeriod = 0;
    if(getSetting("mqttEnabled", MQTT_ENABLED) == "true") {
      if (WiFi.status() == WL_CONNECTED) {
        if (WiFi.getMode() != WIFI_AP) {
          if (!mqtt.connected()) {
          	unsigned long currPeriod = millis() / MQTT_RECONNECT_DELAY;
          	if (currPeriod != lastPeriod) {
          	    lastPeriod = currPeriod;
                  mqttConnect();
            }
          }
        }
      }
    }
}
