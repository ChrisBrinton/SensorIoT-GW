//------------------------------------------------------------------------------
// GENERAL
//------------------------------------------------------------------------------

#define SERIAL_BAUDRATE         115200
#define LED_PIN                 0

//------------------------------------------------------------------------------
// RADIO
//------------------------------------------------------------------------------

#define NODEID                  1
#define GATEWAYID               1
#define NETWORKID               100
#define PROMISCUOUS             0
#define FREQUENCY               RF69_915MHZ
#define ENCRYPTKEY              "sampleEncryptKey"
#define SPI_CS                  2
#define IRQ_PIN                 15
#define RFM69_RST               16
#define IS_RFM69HW              1

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

#define DISPLAY_ALWAYS_ENABLED  "on"
#define DISPLAY_ENABLE_TIME    "06:00"
#define DISPLAY_DISABLE_TIME   "22:00"

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
// DEFAULTS
// -----------------------------------------------------------------------------

#define HEARTBEAT_INTERVAL      60000
#define HOSTNAME                APP_NAME
#define DEVICE                  APP_NAME
