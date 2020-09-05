#include <Arduino.h>
#include "MqttSender.hpp"
#include "Rtc.hpp"

/* SELECT DEVICE TYPE - UNCOMMENT Desired Type or #define somewhere 
    If all are commented - device works on default type - double */

// #define UINT_RTC 0         //Device with uint64_t data
// #define INT_RTC 0          //Device with long long int data

using RtcErr = rtc::ErrorCode;
using SendDev = connection::MqttSender;

SendDev dev{2,"data"};


void setup() {
  Serial.begin(9600);
  while (!Serial);

  dev.connectWiFi();
  if(!dev.checkWiFiConn())
  {
    RTC.deepSleepErr(RtcErr::NO_WIFI);
  }
  if(!dev.connectMqtt())
  {
    RTC.deepSleepErr(RtcErr::NO_MQTT);
  }


  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  dev.sendMsg("Awaken");

  RTC.deepSleep(3e6); //go deep sleep for 3 s
}
