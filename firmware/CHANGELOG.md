# SensorIoT-GW change log
## [1.1.2]

### Changed

- Updated MQTT keepalive time to 240s to be creater than ping interval (60s) on branch using RadioHead radio lib

## [1.0.10] 10/22/2019

### Changed

- Updated MQTT keepalive time to 240s to be creater than ping interval (60s) on branch using lowpowerlab radio lib

## [1.1.1] 9/26/2019

### Changed

- Moved to Radiohead radio library so support RFM95 LoRa radios. This is a pin compatible radio to the RFM69HCW which was used in the previous HW.

### Added

- Supports LTR-329 ambient light sensor and digipot controlled display voltage. This is new in GW HW 1.15 and above. The ambient light is measured using the sensor and the display is adjusted based on this value and the display brightness setting which has been added to the config web pages in the display section.
- The radiohead library is used in the reliable datagram mode where the nodes and the GW exchange ACKs to confirm message receipt. The ACKing will stop on the node side after some number of minutes to conserve node battery life. While the ACKs are happening, TX strength on the node side is tuned.

### Fixed

- Fixed bug in nodelist that caused a crash when deleting an element.

## [1.0.9] 6/18/2019

### Changed

- Reorganized main display code to move slowly side to side (1px/m) to wear OLED displays evenly.
- Updated startup logo to SensorIoT

### Added

- Display has optional shutdown hours, configurable in web config

### Fixed

- Locked platformio core to 2.0.4 to resolve instability

## [1.0.8] 2/18/2019

### Changed

- Updated JustWifi lib to 2.0.2 and refactored to support
- Updated backwards compatible libs to current versions:
  - AsyncMqttClient@0.8.2
  - ESPAsyncTCP@1.2.0
  - ArduinoJson@5.13.4
  - Adafruit GFX Library@b920674
  - ES PAsync WebServer@1.2.0
  - NtpClientLib@2.5.1
  - Sparkfun BME280@2.0.4
  - RFM69@ebf2cb86ff
- Installation directions more complete

### Fixed

- Updated platformio.ini to fix ESP Async WebServer and RFM69 lib defs
- Changed battery thresholds to be more accomodating to 1.5v battery ranges.

### Added

- GW now retrieves nickname from server via MQTT and displays the shortname when available rather than ID:#

## [1.0.7] 1/10/2019

### Added

- Completed support for http firmware upload for both FW and SPIFFS

### Changed

- Display logic is now non-blocking

## [1.0.6] 1/2/2019

### Added

- Initial support for http firmware upload

## [1.0.5] 12/20/2018

### Fixed

- Convert getTopicMapping to string from char\* in effort to eliminate bad topic publishes
- Fix for display of time ending in 1/4/9 shifting and not being centered.

## [1.0.4] 10/1/2018

### Added

- Add erase all firmware function to settings - bring firmware back to factory default

## [1.0.3] 7/1/2018

### Added

- Added display logic to show node 1 data and received node info on display while in AP mode. This allows full HW testing without configuring wifi

### Changed

- Publish node 1 data to MQTT. Packet parsing reworked to facilitate.

## [1.0.0] 7/1/2017

### Added

- Initial version
- Fork of [RFM69GW](https://bitbucket.org/xoseperez/rfm69gw/overview) by Xose Perez over on bitbucket
- Added 128x32 pixel OLED display (based on SSD1305 display driver) to show time and node data
