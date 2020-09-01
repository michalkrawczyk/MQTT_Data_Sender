#include <Arduino.h>
#include "MqttSender.hpp"
#include "Rtc.hpp"

/* SELECT DEVICE TYPE - UNCOMMENT Desired Type or #define somewhere 
    If all are commented - device works on default type - double */

// #define UINT_RTC 0
// #define INT_RTC 0

connection::MqttSender dev(2,"data");

void setup() {
  Serial.begin(9600);
  while (!Serial);

  dev.connectWiFi();
  if(!dev.checkWiFiConnection())
  {
    rtc::rtc_memory.goDeepSleep(rtc::RtcErrorCode::NO_WIFI, 3e6);
  }
  if(!dev.connectMqtt())
  {
    rtc::rtc_memory.goDeepSleep(rtc::RtcErrorCode::NO_MQTT, 3e6);
  }


  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  dev.sendMsg("Awaken");

  rtc::rtc_memory.goDeepSleep(static_cast<rtc::RtcErrorCode>(rtc::RtcErrorCode::NONE), 3e6); //go deep sleep for 3 s
}
