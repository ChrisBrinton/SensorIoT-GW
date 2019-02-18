# SensorIoT-GW change log

## [1.0.8] 2/18/2019
### Changed
 - Updated JustWifi lib to 2.0.2
 - Installation directions more complete
### Fixed
 - Updated platformio.ini to fix ESP Async WebServer and RFM69 lib defs

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
 - Convert getTopicMapping to string from char* in effort to eliminate bad topic publishes
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