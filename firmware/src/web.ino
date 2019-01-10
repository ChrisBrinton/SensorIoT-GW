
/*

WEBSERVER MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

Updated 2018-2019 by Chris Brinton to support file upload and firmware update

*/

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Hash.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <Ticker.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Ticker deferred;

extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;

typedef struct {
    IPAddress ip;
    unsigned long timestamp = 0;
} ws_ticket_t;
ws_ticket_t _ticket[WS_BUFFER_SIZE];

// -----------------------------------------------------------------------------
// WEBSOCKETS
// -----------------------------------------------------------------------------

bool wsSend(const char * payload) {
    //DEBUG_MSG("[WEBSOCKET] Broadcasting '%s'\n", payload);
    ws.textAll(payload);
}

bool wsSend(uint32_t client_id, const char * payload) {
    //DEBUG_MSG("[WEBSOCKET] Sending '%s' to #%ld\n", payload, client_id);
    ws.text(client_id, payload);
}

void _wsParse(uint32_t client_id, uint8_t * payload, size_t length) {

    // Parse JSON input
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((char *) payload);
    if (!root.success()) {
        DEBUG_MSG("[WEBSOCKET] Error parsing data\n");
        ws.text(client_id, "{\"message\": \"Error parsing data!\"}");
        return;
    }

    // Check actions
    if (root.containsKey("action")) {

        String action = root["action"];
        DEBUG_MSG("[WEBSOCKET] Requested action: %s\n", action.c_str());

        if (action.equals("reset")) ESP.reset();
        if (action.equals("reconnect")) {

            // Let the HTTP request return and disconnect after 100ms
            deferred.once_ms(100, wifiDisconnect);

        }
        if (action.equals("clear-counts")) clearCounts();

    };

    // Check config
    if (root.containsKey("config") && root["config"].is<JsonArray&>()) {

        JsonArray& config = root["config"];
        DEBUG_MSG("[WEBSOCKET] Parsing configuration data\n");

        bool dirty = false;
        bool dirtyMQTT = false;
        bool mqttEnableFlag = false;
        unsigned int network = 0;
        unsigned int mappingCount = getSetting("mappingCount", "0").toInt();
        unsigned int mapping = 0;

        for (unsigned int i=0; i<config.size(); i++) {

            String key = config[i]["name"];
            String value = config[i]["value"];
            DEBUG_MSG("[WEBSOCKET] Config: key: %s - value: %s\n", key.c_str(), value.c_str());

            // Do not change the password if empty
            if (key == "adminPass") {
                if (value.length() == 0) continue;
            }

            if (key == "ssid") {
                key = key + String(network);
            }
            if (key == "pass") {
                key = key + String(network);
                ++network;
            }
            if (key == "nodeid") {
                if (value == "") break;
                key = key + String(mapping);
            }
            if (key == "key") {
                key = key + String(mapping);
            }
            if (key == "topic") {
                key = key + String(mapping);
                ++mapping;
            }
            if (key == "mqttEnabled") {
              if (value == "on") {
                DEBUG_MSG("[WEBSOCKET] Setting mqttEnableFlag to true\n");
                mqttEnableFlag = true;
              }
            }
            if (value != getSetting(key)) {
                setSetting(key, value);
                DEBUG_MSG("[WEBSOCKET] Setting %s to %s\n", key.c_str(), value.c_str());
                dirty = true;
                if (key.startsWith("mqtt")) dirtyMQTT = true;
            }

        }

        if((getSetting("mqttEnabled") != "false") && (mqttEnableFlag == false)){
          setSetting("mqttEnabled","false");
          DEBUG_MSG("[WEBSOCKET] Setting mqttEnabled to false\n");
          dirty = true;
        }

        if((getSetting("mqttEnabled") != "true") && mqttEnableFlag) {
          setSetting("mqttEnabled","true");
          DEBUG_MSG("[WEBSOCKET] Setting mqttEnabled to true\n");
          dirty = true;
        }

        // delete remaining mapping
        for (unsigned int i=mapping; i<mappingCount; i++) {
            delSetting("nodeid" + String(i));
            delSetting("key" + String(i));
            delSetting("topic" + String(i));
            dirty = true;
        }

        String value = String(mapping);
        setSetting("mappingCount", value);

        // Save settings
        if (dirty) {

            saveSettings();
            wifiConfigure();
            otaConfigure();

            // Check if we should reconfigure MQTT connection
            if (dirtyMQTT) {
                mqttDisconnect();
            }

            ws.text(client_id, "{\"message\": \"Changes saved\"}");

        } else {

            ws.text(client_id, "{\"message\": \"No changes detected\"}");

        }

    }

}

void _wsStart(uint32_t client_id) {

    char app[64];
    sprintf(app, "%s %s", APP_NAME, APP_VERSION);

    char chipid[6];
    sprintf(chipid, "%06X", ESP.getChipId());

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["app"] = app;
    root["hostname"] = getSetting("hostname", HOSTNAME);
    root["chipid"] = chipid;
    DEBUG_MSG("[WEBSOCKET] Before mac\n");
    root["mac"] = WiFi.macAddress();
    root["device"] = String(DEVICE);
    DEBUG_MSG("[WEBSOCKET] Before getNetwork\n");
    root["network"] = getNetwork();
    DEBUG_MSG("[WEBSOCKET] Before getIP\n");
    root["ip"] = getIP();
    DEBUG_MSG("[WEBSOCKET] Before mqttStatus\n");
    root["mqttStatus"] = mqttConnected();
    DEBUG_MSG("[WEBSOCKET] Before mqttEnabled\n");
    root["mqttEnabled"] = getSetting("mqttEnabled", MQTT_ENABLED);
    root["mqttServer"] = getSetting("mqttServer", MQTT_SERVER);
    root["mqttPort"] = getSetting("mqttPort", String(MQTT_PORT));
    root["mqttUser"] = getSetting("mqttUser");
    root["mqttPassword"] = getSetting("mqttPassword");
    root["ipTopic"] = getSetting("ipTopic", MQTT_IP_TOPIC);
    root["hbTopic"] = getSetting("hbTopic", MQTT_HEARTBEAT_TOPIC);
    root["defaultTopic"] = getSetting("defaultTopic", MQTT_DEFAULT_TOPIC);

    DEBUG_MSG("[WEBSOCKET] Before wifi array\n");
    JsonArray& wifi = root.createNestedArray("wifi");
    for (byte i=0; i<3; i++) {
        JsonObject& network = wifi.createNestedObject();
        network["ssid"] = getSetting("ssid" + String(i));
        network["pass"] = getSetting("pass" + String(i));
    }

    DEBUG_MSG("[WEBSOCKET] Before mapping array\n");
    JsonArray& mappings = root.createNestedArray("mapping");
    byte mappingCount = getSetting("mappingCount", "0").toInt();
    for (byte i=0; i<mappingCount; i++) {
        JsonObject& mapping = mappings.createNestedObject();
        mapping["nodeid"] = getSetting("nodeid" + String(i));
        mapping["key"] = getSetting("key" + String(i));
        mapping["topic"] = getSetting("topic" + String(i));
    }

    String output;
    root.printTo(output);
    ws.text(client_id, (char *) output.c_str());
    DEBUG_MSG("[WEBSOCKET] Sent config to page: %s\n", (char *) output.c_str());

}

bool _wsAuth(AsyncWebSocketClient * client) {

    IPAddress ip = client->remoteIP();
    unsigned long now = millis();
    unsigned short index = 0;

    for (index = 0; index < WS_BUFFER_SIZE; index++) {
        if ((_ticket[index].ip == ip) && (now - _ticket[index].timestamp < WS_TIMEOUT)) break;
    }

    if (index == WS_BUFFER_SIZE) {
        DEBUG_MSG("[WEBSOCKET] Validation check failed\n");
        ws.text(client->id(), "{\"message\": \"Session expired, please reload page...\"}");
        return false;
    }

    return true;

}

void _wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

    static uint8_t * message;

    // Authorize
    #ifndef NOWSAUTH
        if (!_wsAuth(client)) return;
    #endif

    if (type == WS_EVT_CONNECT) {
        IPAddress ip = client->remoteIP();
        DEBUG_MSG("[WEBSOCKET] #%u connected, ip: %d.%d.%d.%d, url: %s\n", client->id(), ip[0], ip[1], ip[2], ip[3], server->url());
        _wsStart(client->id());
    } else if(type == WS_EVT_DISCONNECT) {
        DEBUG_MSG("[WEBSOCKET] #%u disconnected\n", client->id());
    } else if(type == WS_EVT_ERROR) {
        DEBUG_MSG("[WEBSOCKET] #%u error(%u): %s\n", client->id(), *((uint16_t*)arg), (char*)data);
    } else if(type == WS_EVT_PONG) {
        DEBUG_MSG("[WEBSOCKET] #%u pong(%u): %s\n", client->id(), len, len ? (char*) data : "");
    } else if(type == WS_EVT_DATA) {

        AwsFrameInfo * info = (AwsFrameInfo*)arg;

        // First packet
        if (info->index == 0) {
            //Serial.printf("Before malloc: %d\n", ESP.getFreeHeap());
            message = (uint8_t*) malloc(info->len);
            //Serial.printf("After malloc: %d\n", ESP.getFreeHeap());
        }

        // Store data
        memcpy(message + info->index, data, len);

        // Last packet
        if (info->index + len == info->len) {
            _wsParse(client->id(), message, info->len);
            //Serial.printf("Before free: %d\n", ESP.getFreeHeap());
            free(message);
            //Serial.printf("After free: %d\n", ESP.getFreeHeap());
        }

    }

}

// -----------------------------------------------------------------------------
// WEBSERVER
// -----------------------------------------------------------------------------

void _logRequest(AsyncWebServerRequest *request) {
    DEBUG_MSG("[WEBSERVER] Request: %s %s\n", request->methodToString(), request->url().c_str());
}

bool _authenticate(AsyncWebServerRequest *request) {
    String password = getSetting("adminPass", ADMIN_PASS);
    char httpPassword[password.length() + 1];
    password.toCharArray(httpPassword, password.length() + 1);
    return request->authenticate(HTTP_USERNAME, httpPassword);
}

void _onAuth(AsyncWebServerRequest *request) {

    _logRequest(request);

    if (!_authenticate(request)) return request->requestAuthentication();

    IPAddress ip = request->client()->remoteIP();
    unsigned long now = millis();
    unsigned short index;
    for (index = 0; index < WS_BUFFER_SIZE; index++) {
        if (_ticket[index].ip == ip) break;
        if (_ticket[index].timestamp == 0) break;
        if (now - _ticket[index].timestamp > WS_TIMEOUT) break;
    }
    if (index == WS_BUFFER_SIZE) {
        request->send(423);
    } else {
        _ticket[index].ip = ip;
        _ticket[index].timestamp = now;
        request->send(204);
    }

}

void _onHome(AsyncWebServerRequest *request) {

    _logRequest(request);

    if (!_authenticate(request)) return request->requestAuthentication();

    request->send(SPIFFS, "/index.html");
}

bool _apiAuth(AsyncWebServerRequest *request) {

    if (getSetting("apiEnabled").toInt() == 0) {
        DEBUG_MSG("[WEBSERVER] HTTP API is not enabled\n");
        request->send(403);
        return false;
    }

    if (!request->hasParam("apikey", (request->method() == HTTP_PUT))) {
        DEBUG_MSG("[WEBSERVER] Missing apikey parameter\n");
        request->send(403);
        return false;
    }

    AsyncWebParameter* p = request->getParam("apikey", (request->method() == HTTP_PUT));
    if (!p->value().equals(getSetting("apiKey"))) {
        DEBUG_MSG("[WEBSERVER] Wrong apikey parameter\n");
        request->send(403);
        return false;
    }

    return true;

}

void _onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    //TODO: check if this is a legit file (MD5 hash? or just filename format?)
    int update_command;
    uint32_t maxSpace;
    static bool goodFilename = false;

    //DEBUG_MSG("[WEBSERVER] Upload in progress: %s, index: %d  len: %d  final: %d\n", filename.c_str(), index, len, final);

    if(0 == index) {
        DEBUG_MSG("[WEBSERVER] Firmware upload requested for %s. Validating filename\n", filename.c_str());
        if(filename.indexOf("GDESGW1_SPIFFS")>-1){
            update_command = U_SPIFFS;
            DEBUG_MSG("[WEBSERVER] OTA for SPIFFS selected\n");
            maxSpace = ((size_t) &_SPIFFS_end - (size_t) &_SPIFFS_start);
            SPIFFS.end();
            goodFilename = true;
        } else if(filename.indexOf("GDESGW1_FW")>-1) {
            update_command = U_FLASH;
            DEBUG_MSG("[WEBSERVER] OTA for Firmware selected\n");
            maxSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            goodFilename = true;
        } else {
            DEBUG_MSG("[WEBSERVER] No filename format for %s ! Restarting!\n", filename.c_str());
            char buffer[70] = "{\"firmwareEvent\": \"Upload not accepted. Bad Filename.\"}";
            wsSend(buffer);
            setRestartCountdown(5);
            goodFilename = false;
            return;
        }
        DEBUG_MSG("[WEBSERVER] Upload start: %s\n", filename.c_str());
        //Update.begin(maxSketchSpace, U_SPIFFS);
        if(!Update.begin(maxSpace, update_command)){ 
            DEBUG_MSG("[WEBSERVER] Update begin failure! Restarting!\n");
            Update.printError(Serial); 
            char buffer[70] = "{\"firmwareEvent\": \"Upload failed due to an internal error.\"}";
            wsSend(buffer);
            setRestartCountdown(5);
        } else {
            DEBUG_MSG("[WEBSERVER] Update starting\n");
            Update.runAsync(true); // this is required to run with the ESPAsyncWebServer
        }
    }

    //if we didnt get a good filename initially, just ignore the upload
    if (goodFilename == false){
        return;
    }
    if(Update.write(data, len) != len){
        Update.printError(Serial);
    } else { 
        //DEBUG_MSG("[WEBSERVER] Write: %d bytes\n", len); 
        DEBUG_MSG(".");
    }
    if(final) {
      DEBUG_MSG("\n[WEBSERVER] UploadEnd: %s (%u)\n", filename.c_str(), index+len);
      if (Update.end(true)) {
        DEBUG_MSG("[WEBSERVER] Update succesful! Restarting gateway.\n");
        char buffer[60] = "{\"firmwareEvent\": \"uploadSuccess\"}";
        wsSend(buffer);
        setRestartCountdown(15);
       } else {
        Update.printError(Serial);   
       }
    }
}

void webSetup() {

    // Setup websocket plugin
    ws.onEvent(_wsEvent);
    server.addHandler(&ws);

    // Serve home (password protected)
    server.on("/", HTTP_GET, _onHome);
    server.on("/index.html", HTTP_GET, _onHome);
    server.on("/auth", HTTP_GET, _onAuth);
    // upload a file to /upload
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
      request->send(200);
    }, _onUpload);

    // Serve static files
    server.serveStatic("/", SPIFFS, "/");

    // 404
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404);
    });

    // Run server
    server.begin();

}
