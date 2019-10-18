/*

RADIO MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include "RadioHeadManager.h"
#include "config/all.h"
#include <RHReliableDatagram.h>
#ifdef IS_RF95
    #include <RH_RF95.h>
#endif
#if defined(IS_RF69) || defined(IS_RF69HCW)
    #include <RH_RF69.h>
#endif
#include <SPI.h>

#ifdef IS_RF95
    RH_RF95 driver(2,15);
#endif
#if defined(IS_RF69) || defined(IS_RF69HCW)

    RH_RF69 driver(2,15);
#endif

RadioHeadManager radio(driver, GATEWAYID);

void processMessage(packet_t * data);

// -----------------------------------------------------------------------------
// RADIO
// -----------------------------------------------------------------------------

void radioSetup() {
#ifdef IS_RF95
    // Hard Reset the RF_95 module
    pinMode(RADIO_RST, OUTPUT);
    digitalWrite(RADIO_RST, HIGH);
    delay(10);
    digitalWrite(RADIO_RST, LOW);
    delay(10);
    digitalWrite(RADIO_RST, HIGH);
    delay(10);
//    pinMode(RADIO_RST, INPUT);
#endif
#if defined (IS_RF69) || defined (IS_RF69HCW)
    // Hard Reset the RFM module
    pinMode(RADIO_RST, OUTPUT);
    digitalWrite(RADIO_RST, HIGH);
    delay(100);
    digitalWrite(RADIO_RST, LOW);
    delay(100);
#endif

    radio.initialize(FREQUENCY, NODEID, ENCRYPTKEY, GATEWAYID);
    driver.setFrequency(FREQUENCY);
    radio.onMessage(processMessage);
    #if RADIO_DEBUG
        DEBUG_MSG("[RADIO] Working at %d Mhz\n", FREQUENCY);
        DEBUG_MSG("[RADIO] Node ID: %d\n", NODEID);
        DEBUG_MSG("[RADIO] Gateway ID: %d\n", GATEWAYID);
        if (GATEWAYID == NODEID) {
            DEBUG_MSG("[RADIO] This node is a gateway\n");
        } else {
            DEBUG_MSG("[RADIO] This node is not a gateway\n");
        }
    #endif

}

void radioLoop() {

    radio.loop();

}
