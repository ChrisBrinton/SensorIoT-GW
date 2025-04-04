[![version](https://img.shields.io/badge/version-1.0.8-brightgreen.svg)](CHANGELOG.md)
# The firmware for the SensorIoT GW

The firmware is based on Xose Perez project [RFM69GW](https://bitbucket.org/xoseperez/rfm69gw) which has been modified to include a display as well as working with the SensorIoT App and server side as well as moving to the RFM95 Lora radio.

## Installation

### Prerequisits
The firmware project uses Platformio to manage and build. The recommended environment for running Platformio is [VSCode](https://code.visualstudio.com/). After installing VSCode, you can search for the Platformio Extension and install. Platformio can be used command line only if needed. Further details can be found at [Platformio.org](https://platformio.org/)

You'll need the VCP (Virtual Com Port) drivers from [here](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)

And of course you'll need [git](https://www.git-scm.com/downloads) and [node](https://nodejs.org/en/) if you dont have already

Then the following will install the repo and build:

    git clone https://github.com/ChrisBrinton/SensorIoT-GW
    cd SensorIoT-GW/firmware
    npm install
    pio run

If the build fails due to errors such as

    .pio\libdeps\node\ESP Async WebServer_ID306\src\WebHandlers.cpp:78:25: error: 'time' was not declared in this scope

You'll need to remove .pio\libdeps\node\Time_ID44\Time.h. This error happens on windows due to a lack of case sensitivity between Time.h (not needed for this project) and time.h (needed for this project)

To deploy to your device you need to upload both the SPIFFS filesystem that contains the static web data and the initial settings, then the firmware code:
    To build the static web pages:
        gulp
    To upload filesystem (including static web pages):
        pio run --target uploadfs -e node
    To upload firmware:
        pio run --target upload -e node



## Publishing

The GW publishes to MQTT in the following way:

The topic format for our backend looks like:
/<prefix>/<hostname>/<node_id>/<data_type>/<data>

 <prefix> is GDESGW1 for no particular reason other than I needed something to differentiate messages at a high level in case I wanted to use the broker for a completely different set of messages. For those trivia minded, it stands for GeekDayEnvironmentSensorGateWay model 1

<hostname> is a unique 6 digit number. Just pick one for each independent MQTT publisher

<node_id> is for each differentiated sensor cluster from the hostname. So you could publish two different temp readings from the same MQTT publisher by changing the node #

<data_type> is the sensor data type along the lines of H for humidity, P for pressure and F for temperature (sorry metric users)

<data> is the (presumably) numeric value (assumed to be a float of not too many decimal places)

There are a couple of builtin monitoring data types as well that can be safely ignored if you arent using my hardware: BAT for battery voltage and RSSI for signal strength of the node

That all boils down to a real message that looks like:
/GDESGW1/141188/1/F/78.33