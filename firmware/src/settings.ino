/*

SETTINGS MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include "Embedis.h"
#include <EEPROM.h>
#include "spi_flash.h"
#include <StreamString.h>

#define AUTO_SAVE 1

Embedis embedis(Serial);

// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------

unsigned long settingsSize() {
    bool zero = false;
    unsigned long size = 0;
    for (unsigned int i = SPI_FLASH_SEC_SIZE; i>0; i--) {
        size++;
        if (EEPROM.read(i) == 0) {
            if (zero) break;
            zero = true;
        } else {
            zero = false;
        }
    }
    return size-2;
}

void settingsSetup() {

    EEPROM.begin(SPI_FLASH_SEC_SIZE);

    Embedis::dictionary( F("EEPROM"),
        SPI_FLASH_SEC_SIZE,
        [](size_t pos) -> char { return EEPROM.read(pos); },
        [](size_t pos, char value) { EEPROM.write(pos, value); },
        #if AUTO_SAVE
            []() { EEPROM.commit(); }
        #else
            []() {}
        #endif
    );

    Embedis::hardware( F("WIFI"), [](Embedis* e) {
        StreamString s;
        WiFi.printDiag(s);
        e->response(s);
    }, 0);

    Embedis::command( F("RECONNECT"), [](Embedis* e) {
        wifiConfigure();
        wifiDisconnect();
        e->response(Embedis::OK);
    });

    Embedis::command( F("RESET"), [](Embedis* e) {
        e->response(Embedis::OK);
        ESP.reset();
    });

    Embedis::command( F("NTP.RESET"), [](Embedis* e) {
        e->response(Embedis::OK);
        ntpConnect();
    });

    Embedis::command( F("EEPROM.DUMP"), [](Embedis* e) {
        for (unsigned int i = 0; i < SPI_FLASH_SEC_SIZE; i++) {
            if (i % 16 == 0) Serial.printf("\n[%04X] ", i);
            Serial.printf("%02X ", EEPROM.read(i));
        }
        Serial.printf("\n");
        e->response(Embedis::OK);
    });

    Embedis::command( F("EEPROM.DUMPCHAR"), [](Embedis* e) {
        for (unsigned int i = 0; i < SPI_FLASH_SEC_SIZE; i++) {
            if (i % 16 == 0) Serial.printf("\n[%04X] ", i);
            int chr = EEPROM.read(i);
            if(chr>=32 && chr <=126){
                Serial.printf("%c ", EEPROM.read(i));
            } else {
                Serial.printf("%02X ", EEPROM.read(i));            
            }
        }
        Serial.printf("\n");
        e->response(Embedis::OK);
    });

    Embedis::command( F("EEPROM.ERASE"), [](Embedis* e) {
        eraseAllSettings();
        e->response(Embedis::OK);
    });

    Embedis::command( F("SETTINGS.SIZE"), [](Embedis* e) {
        e->response(String(settingsSize()));
    });

    DEBUG_MSG("[SETTINGS] EEPROM size: %d bytes\n", SPI_FLASH_SEC_SIZE);
    DEBUG_MSG("[SETTINGS] Settings size: %lu bytes\n", settingsSize());

}

void settingsLoop() {
    embedis.process();
}

template<typename T> String getSetting(const String& key, T defaultValue) {
    String value;
    if (!Embedis::get(key, value)){
        value = String(defaultValue);
    } else {
        //DEBUG_MSG("[SETTINGS] Embeddis returned key: %s, value: %s\n", key.c_str(), value.c_str());
    }
    return value;
}

String getSetting(const String& key) {
    return getSetting(key, "");
}

template<typename T> bool setSetting(const String& key, T value) {
    DEBUG_MSG("[SETTINGS] setting key:%s value: %s \n", (char *)key.c_str(),(char *)String(value).c_str());
    return Embedis::set(key, String(value));
}

bool delSetting(const String& key) {
    return Embedis::del(key);
}

void saveSettings() {
    DEBUG_MSG("[SETTINGS] Saving\n");
    #if not AUTO_SAVE
        EEPROM.commit();
    #endif
}

bool eraseAllSettings() {
  for (unsigned int i = 0; i < SPI_FLASH_SEC_SIZE; i++) {
      EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();

  DEBUG_MSG("[SETTINGS] All Settings Erased\n");
  stageDisplayMsg((char*)"All Settings Erased");

  return true;
}
