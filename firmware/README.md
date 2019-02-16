# The firmware for the SensorIoT GW

The firmware is based on Xose Perez project [RFM69GW](https://bitbucket.org/xoseperez/rfm69gw) which has been modified to include a display as well as working with the SensorIoT App and server side.

## Installation

### Prerequisits
The firmware project uses Platformio to manage and build. The recommended environment for running Platformio is [VSCode](https://code.visualstudio.com/). After installing VSCode, you can search for the Platformio Extension and install. Further details can be found at [Platformio](https://platformio.org/)

You'll need the VCP (Virtual Com Port) drivers from [here](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)

And of course you'll need [git](https://www.git-scm.com/downloads) if you dont have it already

Then install the repo

    git clone https://github.com/ChrisBrinton/SensorIoT-GW

From within the platformio environment execute platformio run