/*

NTP MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include <TimeLib.h>
#include <NtpClientLib.h>
#include <WiFiClient.h>

// -----------------------------------------------------------------------------
// NTP
// -----------------------------------------------------------------------------

void ntpConnect() {
    DEBUG_MSG("[NTP] starting with Server: %s, Time Offset: %i, Daylight: %s, Update Interval: %i\n", NTP_SERVER, NTP_TIME_OFFSET, NTP_DAY_LIGHT ? "true":"false", NTP_UPDATE_INTERVAL);
    NTP.begin(NTP_SERVER, NTP_TIME_OFFSET, NTP_DAY_LIGHT);
    NTP.setInterval(NTP_UPDATE_INTERVAL);
    NTP.setDSTZone(DST_ZONE_USA);
}

void ntpSetup() {

    NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
        if (error) {
            if (error == noResponse) {
                DEBUG_MSG("[NTP] Error: NTP server not reachable\n");
            } else if (error == invalidAddress) {
                DEBUG_MSG("[NTP] Error: Invalid NTP server address\n");
            }
        } else {
            DEBUG_MSG("[NTP] Time: %s\n", (char *) NTP.getTimeDateString(NTP.getLastNTPSync()).c_str());
        }
    });

    //NTP.setTimeZone(getSetting("mqttServer", NTP_TIME_ZONE).toInt());
    NTP.setTimeZone(NTP_TIME_OFFSET);

}

void ntpLoop() {
    now();
}
