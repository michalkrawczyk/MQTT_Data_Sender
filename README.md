# MQTT_Data_Sender
Template for Low Power Device sending data over WiFi with MQTT

That template is designed for ESP8266 devices and allows to:
-Connect to Network and MQTT broker
-Send Data over MQTT
-Deep Sleep (With saving Data and last error to RTC memory)
Note: Deep sleep state provides power consumption on about 20uA
-Read RTC memory which can be used to restore last state before Deep Sleep was used


05.09.2020 - Project has been put on hold - Because of long time to establish WiFi connection after deepSleep (required for low current usage).
Will wait for decision after test with Bluetooth Low Energy.