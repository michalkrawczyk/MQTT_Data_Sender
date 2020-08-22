#include <Arduino.h>
#include "MqttSender.hpp"
#include "Rtc.hpp"

connection::MqttSender dev(2,"data");

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if(!dev.connectWLAN())
  {
    rtc::goDeepSleep(rtc::RtcErrorCode::NO_WIFI, 3e6);
  }
  if(!dev.connectMqtt())
  {
    rtc::goDeepSleep(rtc::RtcErrorCode::NO_MQTT, 3e6);
  }


  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  dev.sendMsg("Awaken");

  rtc::goDeepSleep(static_cast<rtc::RtcErrorCode>(rtc::rtc_data.last_error), 3e6); //go deep sleep for 3 s
}
