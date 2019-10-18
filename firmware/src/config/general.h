//------------------------------------------------------------------------------
// GENERAL
//------------------------------------------------------------------------------

#define SERIAL_BAUDRATE         74880
#define LED_PIN                 0

//------------------------------------------------------------------------------
// RADIO
//------------------------------------------------------------------------------

#define NODEID                  1
#define GATEWAYID               1
#define FREQUENCY               915
#define ENCRYPTKEY              "sampleEncryptKey"
#define RADIO_SPI_CS            2
#define RADIO_IRQ_PIN           15
#define RADIO_RST               16
#define IS_RF95                 1   //Use for RF95/96/97/98
//#define IS_RF69                 1   //Use for RFM69
//#define IS_RF69HCW             1   //Use only for RFM69HCW
#define ACK_TIMEOUT             500 //millis to wait for an ack
#define RF95_POWER              20  //RF95s allow power to be set between 5 and 23
#define RF69HCW_POWER           20  //RF69HCSs allow power to be set between -2 and 20
#define RF95_CONFIG             RH_RF95::Bw125Cr45Sf128 //see RH_RF95.h for other options
#define RF69HCW_CONFIG          RH_RF69::OOK_Rb1Bw1 //see RH_RF69.h for other options
// -----------------------------------------------------------------------------
// WIFI
// -----------------------------------------------------------------------------

#define WIFI_RECONNECT_INTERVAL 300000
#define WIFI_MAX_NETWORKS       3
#define ADMIN_PASS              "fibonacci"
#define HTTP_USERNAME           "admin"
#define WS_BUFFER_SIZE          5
#define WS_TIMEOUT              1800000

// -----------------------------------------------------------------------------
// OTA
// -----------------------------------------------------------------------------

#define OTA_PORT                8266

// -----------------------------------------------------------------------------
// Display
// -----------------------------------------------------------------------------

#define DISPLAY_ALWAYS_ENABLED      "on"
#define DISPLAY_ENABLE_TIME         "06:00"
#define DISPLAY_DISABLE_TIME        "22:00"
#define DISPLAY_MAX_BRIGHTNESS      0x4F
#define DISPLAY_BRIGHTNESS_RANGE    25
#define DISPLAY_AUTO_BRIGHTNESS     "on"

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

#define MQTT_ENABLED            "on"
#define MQTT_SERVER             "brintontech.com" // aka "74.208.159.205"
#define MQTT_PORT               1883
#define MQTT_RETAIN             false
#define MQTT_QOS                0
#define MQTT_KEEPALIVE          30
#define MQTT_RECONNECT_DELAY    10000
#define MQTT_TRY_INTERVAL       30000
#define MQTT_MAX_TRIES          20
#define MQTT_USER               "sampleuser"
#define MQTT_PASS               "sampleuserkey"
#define MQTT_IP_TOPIC           "/{hostname}/ip/"
#define MQTT_HEARTBEAT_TOPIC    "/{hostname}/ping/"
#define MQTT_DEFAULT_TOPIC      "/{hostname}/{nodeid}/{key}"
#define MQTT_REPLY_TOPIC        "/reply/{hostname}/#"
#define MQTT_COMMAND_TOPIC      "/command/{hostname}"
// -----------------------------------------------------------------------------
// CONFIG SERVER
// -----------------------------------------------------------------------------

#define CSERV_HOST              "brintontech.com"
#define CSERV_PORT              443
#define CSERV_CERT    "40 B6 D1 3A 9C FC 0A FC 58 24 92 D9 19 66 8C 14 06 D8 F4 81"

// -----------------------------------------------------------------------------
// NTP
// -----------------------------------------------------------------------------

#define NTP_SERVER              "pool.ntp.org"
#define NTP_TIME_OFFSET         -5
#define NTP_DAY_LIGHT           true
#define NTP_UPDATE_INTERVAL     1800
#define TIMEID                  0

// -----------------------------------------------------------------------------
// Onboard BME280 Sensor
// -----------------------------------------------------------------------------

#define BME280_RUNMODE          3
#define BME280_OVERSAMPLE       1
#define BME280_FILTER           0
#define BME280_STANDBY          0
#define SENSORID                1

// -----------------------------------------------------------------------------
// Onboard LTR-329 (Ambient Light) Sensor
// -----------------------------------------------------------------------------

#define LTR329_GAIN             1 //Set gain to 2x
#define LTR329_INT_TIME         3 //400ms
#define LTR329_MEAS_RATE        5 //2000ms
#define LTR329_ROLLING_AVG_SIZE 10

// -----------------------------------------------------------------------------
// DEFAULTS
// -----------------------------------------------------------------------------

#define HEARTBEAT_INTERVAL      60000
#define HOSTNAME                APP_NAME
#define DEVICE                  APP_NAME
