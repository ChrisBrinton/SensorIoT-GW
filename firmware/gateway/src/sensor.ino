#include <SparkFunBME280.h>
#include <LTR303.h>
#include <Wire.h>
#include <SPI.h>
#include "config/global.h"

void printI2CError(byte error) {
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error) {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}

//
// Globals for BME280 Sensor module
//

BME280 bme280;
double F,P,H;
char buffer[50];

//
// Globals for LTR-329 Sensor
//

LTR303 ltr329;
double avgLight;

void sensorSetup(){

  THPSensorSetup();
  if(DISPLAY_HAS_LIGHT_SENSOR) {
    ALSensorSetup();
  }

}

void THPSensorSetup() {

  bme280.settings.commInterface = I2C_MODE;
  bme280.settings.I2CAddress = 0x77;
  bme280.settings.runMode = BME280_RUNMODE; //Normal mode
  bme280.settings.tStandby = BME280_STANDBY;
  bme280.settings.filter = BME280_FILTER;
  bme280.settings.tempOverSample = BME280_OVERSAMPLE;
  bme280.settings.pressOverSample = BME280_OVERSAMPLE;
  bme280.settings.humidOverSample = BME280_OVERSAMPLE;

}

void ALSensorSetup(){

  // Initialize the LTR303 library
  // 100ms 	initial startup time required
  delay(100);

  resetALSensor();

  setALSensor();

  avgLight = 30;

}

// Perform a SW reset of the LTR-329
void resetALSensor() {
  DEBUG_MSG("[SENSOR] Resetting LTR-329\n");
  // You can pass nothing to begin() for the default I2C address (0x29)
  ltr329.begin();
  //Performa a SW reset of the device (gain here is ignored)
  ltr329.setControl(LTR329_GAIN, true, false);
  delay(200); // wait after reset;
}

//Set the control registers of the LTR-329 and start capturing data
void setALSensor() {

  // You can pass nothing to begin() for the default I2C address (0x29)
  ltr329.begin();

  ltr329.setControl(LTR329_GAIN, false, false);

  ltr329.setMeasurementRate(LTR329_INT_TIME,LTR329_MEAS_RATE);

  ltr329.setPowerUp();
  printAllLTR329Regs();

}

void sensorLoop(){

  THPSensorLoop();
  if(DISPLAY_HAS_LIGHT_SENSOR) {
    AmbLightSensorLoop();
  }
}

void THPSensorLoop() {

  static int lastMillis = 0;
  int nowMillis = millis();
  //take a sample every 10min to avoid self heating
  if(lastMillis==0 || (nowMillis > (lastMillis + 600000)) || nowMillis < lastMillis){
    //read BME sensor
    bme280.begin();
    P = bme280.readFloatPressure() * 0.0002953; //read Pa and convert to inHg
    F = bme280.readTempF();
    H = bme280.readFloatHumidity();
    bme280.writeRegister(BME280_CTRL_MEAS_REG, 0x00); //sleep the BME280
        nodeInfo[SENSORID].count = nodeInfo[SENSORID].count + 1;

    if(nodeInfo[SENSORID].THP == 0) {
      nodeInfo[SENSORID].THP = new _GDnode_t;
    }
    nodeInfo[SENSORID].THP->RSSI = String("0");
    nodeInfo[SENSORID].THP->BAT = String("4.0v");
    nodeInfo[SENSORID].senderID = SENSORID;
    nodeInfo[SENSORID].targetID = 1; //packets are always sent to node 1 (the gateway)
    nodeInfo[SENSORID].messageID = 1; //default value to fully populate struct
    nodeInfo[SENSORID].packetID = 0; //zero means we arent participating in missing/duplicate packet checks

    if(!nodeList.nodeExists(SENSORID)){
      nodeList.insertNode(SENSORID);
    }
    char tmpStr[10];
    dtostrf(F, 3,2, tmpStr);
    nodeInfo[SENSORID].THP->F = String(tmpStr);
    dtostrf(H, 3,2, tmpStr);
    nodeInfo[SENSORID].THP->H = String(tmpStr);
    dtostrf(P, 3,2, tmpStr);
    nodeInfo[SENSORID].THP->P = String(tmpStr);

    lastMillis = nowMillis;
    DEBUG_MSG("[SENSOR] Reading - F:%s H:%s P:%s\n", nodeInfo[SENSORID].THP->F.c_str(), nodeInfo[SENSORID].THP->H.c_str(), nodeInfo[SENSORID].THP->P.c_str());

    sendSensorData();
  }

}

void AmbLightSensorLoop(){
  static int lastMillis = 0;
  int nowMillis = millis();
  //take a reading every 10sec
  if(lastMillis==0 || (nowMillis > (lastMillis + 4000)) || nowMillis < lastMillis){
    lastMillis = nowMillis;

    ltr329.begin();
    //printAllLTR329Regs();

    bool reset, mode;
    byte gain;
    ltr329.getControl(gain, reset, mode);

    if(!mode){
      //This shouldnt happen after initialization, but if it does the sensor needs to be reconfigured to collect data.
      DEBUG_MSG("[SENSOR] LTR-329 mode inactive. Setting registers\n");
      setALSensor();
    }

    // Check if there is new data to be read
    if(ltr329.hasNewData()) {
      //DEBUG_MSG("[SENSOR] LTR-329 has new data\n");
      // Retrieve the data from the device:
      unsigned int data0, data1;
      if (ltr329.getData(data0,data1)) {
        // getData() returned true, communication was successful
        double lux;    // Resulting lux value
        if(ltr329.getLux(LTR329_GAIN,LTR329_INT_TIME,data0,data1,lux)) {

          //Using data0 here (Visible + IR light) due to jumpiness of lux calc. The two channels have some
          //lag between them and this causes lux to swing wildly.
          double new_sample = data0;
          avgLight -= avgLight/LTR329_ROLLING_AVG_SIZE;
          avgLight += new_sample/LTR329_ROLLING_AVG_SIZE;

          //DEBUG_MSG("[SENSOR] V+IR: %i IR: %i Avg Light: %4.1f\n", data0, data1, avgLight);
        }    
      }
      else {
        // getData() returned false because of an I2C error, inform the user.
        byte error = ltr329.getError();
        printI2CError(error);
      }
    }
  }
}

void printAllLTR329Regs() {
        bool valid, intrStatus, dataStatus, reset, mode;
        byte gain, intTime, measRate;
        ltr329.getStatus(valid, gain, intrStatus, dataStatus);

        ltr329.getControl(gain, reset, mode);

        ltr329.getMeasurementRate(intTime, measRate);

        DEBUG_MSG("[SENSOR] LTR-329 registers contain - gain: %i, reset: %i, mode: %i, ", gain, reset, mode);
        DEBUG_MSG("intTime: %i, measRate: %i, ", intTime, measRate);
        DEBUG_MSG("valid: %i, intrStatus: %i, dataStatus: %i\n", valid, intrStatus, dataStatus);
}

double getAvgLight() {
  return avgLight;
}