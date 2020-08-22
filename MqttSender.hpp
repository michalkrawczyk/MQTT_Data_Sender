#ifndef MQTT_SENDER_HPP
#define MQTT_SENDER_HPP

#include <stdint.h>
#include <string>
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT_Client.h>

#include "Network_Settings.h" // this file contains Network and MQTT Setup Settings
/* Note: That file includes: 
*(#define) WLAN_SSID, WLAN_PASS, IO_SERVER, IO_PORT, IO_USER, IO_KEY, IO_ERROR_FEED
*/

namespace connection
{
    enum class SignalCode : uint8_t;
    
    extern WiFiClient client;
    extern Adafruit_MQTT_Client mqtt;

    #ifdef ESP8266
    inline namespace Esp
    {
        class MqttSender;

        const IPAddress k_ip(IP_ADDR);
        const IPAddress k_gateway(GATEWAY_ADDR);
        const IPAddress k_subnet(SUBNET_ADDR);
        
    }// namespace Esp
    #endif //ESP8266
}// namespace connection

enum class connection::SignalCode : uint8_t
{
    NO_ERROR = 0u,
    BAD_PAYLOAD,            //For Incorrect Data in Payload
    BAD_PAYLOAD_SIZE,       //For Incorrect Number of Variables
    BAD_DEVICE_ID,          //For Commands that are for other Devices
    READING_ERROR,          //Error occured during reading data from sensor
    UNKNOWN_ERROR

};

#ifdef ESP8266
class connection::MqttSender final
{
public:
    explicit MqttSender(const uint8_t &device_id, const std::string &feed_name);
    static bool connectWLAN();
    static bool connectMqtt();

    const bool sendError(const SignalCode &err_code);
    const bool sendMsg(const std::string &msg, const std::string &additional_msg = ""); //TODO:Exit

    const bool sendData(const uint32_t &data, const bool &save = true); 
    const bool sendData(const float &data,
                        uint8_t decimal_number,
                        const bool &save = true);
    const bool sendData(const double &data,
                        uint16_t decimal_number = 2,
                        const bool &save = true);
    const bool sendData(const int &data, const bool &save = true);
    const bool sendData(const uint16_t &data1,
                        const uint16_t &data2, 
                        const bool &save = true);

    //TODO: consider below functions (e.g to send Turn Off signal)
    const bool compareID(const uint8_t &device_id);
    void checkForCommands();
    //bool retainConn();

private:
    const uint8_t _k_dev_id;
    const std::string _feed;

    std::string _last_msg;
};
#endif //ESP8266

#endif //MQTT_SENDER_HPP