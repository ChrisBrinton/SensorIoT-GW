#include <SparkFunBME280.h>
#include <SPI.h>
#include "config/global.h"
//
// BME280 Sensor module
//

BME280 bme280;
char buffer[50];

void sensorSetup(){

  bme280.settings.commInterface = I2C_MODE;
  bme280.settings.I2CAddress = 0x77;
  bme280.settings.runMode = BME280_RUNMODE; //Normal mode
  bme280.settings.tStandby = BME280_STANDBY;
  bme280.settings.filter = BME280_FILTER;
  bme280.settings.tempOverSample = BME280_OVERSAMPLE;
  bme280.settings.pressOverSample = BME280_OVERSAMPLE;
  bme280.settings.humidOverSample = BME280_OVERSAMPLE;

}

double F,P,H;

void sensorLoop(){

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
