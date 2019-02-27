/*

WIFI MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include "JustWifi.h"

#if defined(ARDUINO_ARCH_ESP32)
    #include <ESPmDNS.h>
#else
    #include <ESP8266mDNS.h>
#endif

// -----------------------------------------------------------------------------
// Callbacks
// -----------------------------------------------------------------------------
void mdnsCallback(justwifi_messages_t code, char * parameter) {

    if (code == MESSAGE_CONNECTED) {

        #if defined(ARDUINO_ARCH_ESP32)
            String hostname = String(WiFi.getHostname());
        #else
            String hostname = WiFi.hostname();
        #endif

        // Configure mDNS
        if (MDNS.begin((char *) hostname.c_str())) {
            Serial.printf("[MDNS] OK\n");

            // Publish as if there is an TELNET service available
            MDNS.addService("telnet", "tcp", 22);

        } else {
            Serial.printf("[MDNS] FAIL\n");
        }

    }

}

// -----------------------------------------------------------------------------
// Captive portal
// -----------------------------------------------------------------------------

#include <DNSServer.h>

DNSServer dnsServer;

void captivePortalCallback(justwifi_messages_t code, char * parameter) {

    if (code == MESSAGE_ACCESSPOINT_CREATED) {
        if (!WiFi.isConnected()) dnsServer.start(53, "*", WiFi.softAPIP());
    }

    if (code == MESSAGE_CONNECTED) {
        dnsServer.stop();
    }

}

void wifiInfoCallback(justwifi_messages_t code, char * parameter) {

	#ifdef DEBUG_PORT

		if (code == MESSAGE_SCANNING) {
			DEBUG_MSG("[WIFI] Scanning\n");
		}

		if (code == MESSAGE_SCAN_FAILED) {
			DEBUG_MSG("[WIFI] Scan failed\n");
		}

		if (code == MESSAGE_NO_NETWORKS) {
			DEBUG_MSG("[WIFI] No networks found\n");
		}

		if (code == MESSAGE_NO_KNOWN_NETWORKS) {
			DEBUG_MSG("[WIFI] No known networks found\n");
		}

		if (code == MESSAGE_FOUND_NETWORK) {
			DEBUG_MSG("[WIFI] %s\n", parameter);
		}

		if (code == MESSAGE_CONNECTING) {
			DEBUG_MSG("[WIFI] Connecting to %s\n", parameter);
		}

		if (code == MESSAGE_CONNECT_WAITING) {
			// too much noise
		}

		if (code == MESSAGE_CONNECT_FAILED) {
			DEBUG_MSG("[WIFI] Could not connect to %s\n", parameter);
		}

		if (code == MESSAGE_CONNECTED) {
			DEBUG_MSG("[WIFI] MODE STA -------------------------------------\n");
			DEBUG_MSG("[WIFI] SSID %s\n", WiFi.SSID().c_str());
			DEBUG_MSG("[WIFI] IP   %s\n", WiFi.localIP().toString().c_str());
			DEBUG_MSG("[WIFI] MAC  %s\n", WiFi.macAddress().c_str());
			DEBUG_MSG("[WIFI] GW   %s\n", WiFi.gatewayIP().toString().c_str());
			DEBUG_MSG("[WIFI] MASK %s\n", WiFi.subnetMask().toString().c_str());
			DEBUG_MSG("[WIFI] DNS  %s\n", WiFi.dnsIP().toString().c_str());
			DEBUG_MSG("[WIFI] HOST %s\n", WiFi.hostname().c_str());
			DEBUG_MSG("[WIFI] ----------------------------------------------\n");

			stageWifiInfo((char*)"Connected to:", (char*)WiFi.SSID().c_str(), (char*)WiFi.localIP().toString().c_str());
		}

		if (code == MESSAGE_ACCESSPOINT_CREATED) {
			DEBUG_MSG("[WIFI] MODE AP --------------------------------------\n");
			DEBUG_MSG("[WIFI] SSID %s\n", jw.getAPSSID().c_str());
			DEBUG_MSG("[WIFI] PASS %s\n", getSetting("adminPass", ADMIN_PASS).c_str());
			DEBUG_MSG("[WIFI] IP   %s\n", WiFi.softAPIP().toString().c_str());
			DEBUG_MSG("[WIFI] MAC  %s\n", WiFi.softAPmacAddress().c_str());
			DEBUG_MSG("[WIFI] ----------------------------------------------\n");

			stageWifiInfo((char*)"AP Mode - SSID: ", (char*)jw.getAPSSID().c_str(), (char*)WiFi.softAPIP().toString().c_str());
		}

		if (code == MESSAGE_DISCONNECTED) {
			DEBUG_MSG("[WIFI] Disconnected\n");
		}

		if (code == MESSAGE_ACCESSPOINT_CREATING) {
			DEBUG_MSG("[WIFI] Creating access point\n");
		}

		if (code == MESSAGE_ACCESSPOINT_FAILED) {
			DEBUG_MSG("[WIFI] Could not create access point\n");
		}

	#endif

	// NTP connection reset
	if (code == MESSAGE_CONNECTED) {
		ntpConnect();
	}

}

// -----------------------------------------------------------------------------
// WIFI
// -----------------------------------------------------------------------------

String getIP() {
    if (WiFi.getMode() == WIFI_AP) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

String getNetwork() {
    if (WiFi.getMode() == WIFI_AP) {
        return jw.getAPSSID();
    }
    return WiFi.SSID();
}

void wifiDisconnect() {
    jw.disconnect();
}

void resetConnectionTimeout() {
    jw.resetReconnectTimeout();
}

bool wifiConnected() {
    return jw.connected();
}


/*
bool createAP() {
	jw.disconnect();
	js.resetReconnectTimeout();
    return jw.createAP();
}
*/
void wifiConfigure() {
	// AP mode only as fallback
	jw.enableAP(false);
	jw.enableAPFallback(true);
    jw.setSoftAP(getSetting("hostname", HOSTNAME).c_str(), getSetting("adminPass", ADMIN_PASS).c_str());
	//give the user 5 min to do what they need to do before cycling if its in AP mode
	jw.setReconnectTimeout(600000);

	jw.enableSTA(true);
	jw.enableScan(true);
	jw.cleanNetworks();

    if (getSetting("ssid0").length() > 0){
		jw.addNetwork(getSetting("ssid0").c_str(), getSetting("pass0").c_str());
    	DEBUG_MSG("[WIFI] Added Network - ssid:%s\n", getSetting("ssid0").c_str());
	} 
	if (getSetting("ssid1").length() > 0){
		jw.addNetwork(getSetting("ssid1").c_str(), getSetting("pass1").c_str());
		DEBUG_MSG("[WIFI] Added Network - ssid:%s\n", getSetting("ssid1").c_str());
	} 
    if (getSetting("ssid2").length() > 0){
		jw.addNetwork(getSetting("ssid2").c_str(), getSetting("pass2").c_str());
    	DEBUG_MSG("[WIFI] Added Network - ssid:%s\n", getSetting("ssid2").c_str());
	} 
}

void wifiSetup() {

    jw.setHostname(getSetting("hostname", HOSTNAME).c_str());

    // Message callbacks
    jw.subscribe(wifiInfoCallback);
    jw.subscribe(mdnsCallback);
    jw.subscribe(captivePortalCallback);

	wifiConfigure();

}

void wifiLoop() {
    jw.loop();
}

