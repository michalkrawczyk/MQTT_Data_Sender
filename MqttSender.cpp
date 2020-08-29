#include "MqttSender.hpp"
#include "Rtc.hpp"
#include <sstream>

static const std::string feedToString(const std::string &feed_name);

/**
* @brief Convert data to string
* @note: Compiler Bug causes not finding std::to_string even in <string>
* 
* @param data - data to convert
* @return data as string
*/
template <typename T>
static std::string to_string(const T &data) //
{
    std::ostringstream osstream ;
    osstream << data ;

    return osstream.str() ;
}

namespace connection
{
    #ifdef ESP8266

    extern WiFiClient client{};
    extern Adafruit_MQTT_Client mqtt{&client, IO_SERVER, IO_PORT, IO_USER, IO_KEY};

    MqttSender::MqttSender(const uint8_t &device_id, const std::string &feed_name):
    _k_dev_id(device_id),
    _feed(feedToString(feed_name))
    {

    }

    /**
    * @brief Starts connection with Network
    * 
    * @return true if connected
    */
    bool MqttSender::connectWLAN()
    {
        #ifdef DEBUG
            Serial.println("Connecting with Network");
        #endif// DEBUG

        WiFi.persistent(false); //Avoiding Unnecessary writing to Flash and issues connected with
        WiFi.enableSTA(true);
        delay(100);

        WiFi.mode(WIFI_STA); //WiFi mode station (connect to wifi router only)
        if(!WiFi.forceSleepWake())
        {
            wifi_station_connect();
        } //Avoiding issue when esp not always connect with WiFi
        delay(10);
        
        if(!WiFi.config(IP_ADDR, GATEWAY_ADDR, SUBNET_ADDR))
        {
            return false;
        } //Static Ip Connection is much faster in ESP than DHCP
        // WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
        // WiFi.setPhyMode(); //WIFI_PHY_MODE_11B: 802.11b mode - WIFI_PHY_MODE_11G: 802.11g mode - WIFI_PHY_MODE_11N: 802.11n mode
        // WiFi.setOutputPower(17.5);

        if(rtc::rtc_memory.isValid())
        {   
            WiFi.begin(WLAN_SSID, WLAN_PASS, rtc::rtc_memory.getData().channel, rtc::rtc_memory.getData().bssid);
        }
        else
        {
        
            WiFi.begin(WLAN_SSID, WLAN_PASS);
        }

        
        uint8_t retries(CONN_RETRIES);

        while ((WiFi.status() != WL_CONNECTED) && (retries--)) 
        {
            delay(100);

            #ifdef DEBUG
                Serial.print(".");
            #endif// DEBUG
        }

        return WiFi.status() == WL_CONNECTED;
    }

    /**
    * @brief Starts connection with MQTT
    * 
    * @return true if connected
    */
    bool MqttSender::connectMqtt()
    {
        if (mqtt.connected()) 
        {
            return true;
        }

        #ifdef DEBUG
            Serial.println("Connecting with MQTT Broker");
        #endif// DEBUG
        uint8_t retries(CONN_RETRIES);
            
        int8_t ret;

        while (ret = mqtt.connect()) // mqtt.connect will return 0 if connected
        { 
            #ifdef DEBUG
                Serial.println(mqtt.connectErrorString(ret));
                Serial.println("Failed - Retrying 0.5 seconds...");
            #endif// DEBUG

            mqtt.disconnect();

            // ESP.wdtFeed(); //feed to avoid reseting during reconnect
            delay(500); //TODO: Consider changing to sleep function
            retries--;
                
            if (!retries) 
            {
                return false;
            }
        }
        return true;
    }

    /**
    * @brief Publish Error Message to MQTT Error Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param err_code - enum with code of error
    * 
    * @return true if published
    */
    const bool MqttSender::sendError(const SignalCode &err_code)
    {
        bool result(false);

        if(mqtt.connected())
        {
            std::string msg(to_string(static_cast<int>(_k_dev_id)));
            //TODO:Add Key encryption?
        
            msg += '/' + to_string(static_cast<int>(err_code));

            #ifdef DEBUG
            Serial.print("Error Signal Sent:");
            Serial.println(msg.c_str());
            #endif //DEBUG

            result = mqtt.publish(IO_ERROR_FEED, msg.c_str());
        }
        
        if(!result)
        {
            rtc::rtc_memory.goDeepSleep(rtc::RtcErrorCode::MQTT_PUB_FAIL);
        }

        return result;
    }

    /**
    * @brief Publish Message to MQTT Publish Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param msg - string with message to publish
    * @param additional_msg - string with additional message to publish(may be needed in future)
    * 
    * @return true if published
    */
    const bool MqttSender::sendMsg(const std::string &msg, const std::string &additional_msg)
    {
        bool result(false);

        if(mqtt.connected())
        {
            std::string msg_sent(to_string(static_cast<int>(_k_dev_id)));
            //TODO:Add Key encryption?
        
            msg_sent += ':' + msg;

            if (!additional_msg.empty())
            {
                msg_sent += ':' + additional_msg;
            }

            #ifdef DEBUG
            Serial.print("Signal Sent:");
            Serial.println(msg_sent.c_str());
            #endif //DEBUG

            result = mqtt.publish(_feed.c_str(), msg_sent.c_str(), 1);
        }

        if(!result)
        {
            #ifdef DEBUG
            Serial.print("Signal Sending Failed:");
            Serial.println(msg.c_str());
            #endif //DEBUG

            rtc::rtc_memory.goDeepSleep(rtc::RtcErrorCode::MQTT_PUB_FAIL);
        }

        return false;
    }


    /**
    * @brief Publish Float data to MQTT Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param data - data to publish
    * @param decimal_number - number of decimal places
    * @param save - if true - saves data to RTC Memory
    * 
    * @return true if published
    */
    const bool MqttSender::sendData(const float &data,
                                    uint8_t decimal_number,
                                    const bool &save)
    {
        std::string msg = to_string(data) + "f_";    //By now send float as it is - maybe change in future

        if(save)
        {
            rtc::rtc_memory.saveToMemWithData((static_cast<rtc_mem_t>(data)));
        }
        
        return sendMsg(msg);
    }

    /**
    * @brief Publish Double data to MQTT Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param data - data to publish
    * @param decimal_number - number of decimal places
    * @param save - if true - saves data to RTC Memory
    * 
    * @return true if published
    */
    const bool MqttSender::sendData(const double &data,
                                    uint16_t decimal_number,
                                    const bool &save)
    {
        std::string msg = to_string(data) + "d_";    //By now send double as it is - maybe change in future

        if(save)
        {
            rtc::rtc_memory.saveToMemWithData((static_cast<rtc_mem_t>(data)));
        }
        
        return sendMsg(msg);
    }

/**
    * @brief Publish Long Long data to MQTT Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param data - data to publish
    * @param save - if true - saves data to RTC Memory
    * 
    * @return true if published
    */
    const bool MqttSender::sendData(const long long int &data, const bool &save)
    {
        std::string msg = to_string(data) + "ll_";

        if(save)
        {
            rtc::rtc_memory.saveToMemWithData((static_cast<rtc_mem_t>(data)));
        }
        
        return sendMsg(msg);
    }

    /**
    * @brief Publish Uint64_t data to MQTT Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param data - data to publish
    * @param save - if true - saves data to RTC Memory
    * 
    * @return true if published
    */
    const bool MqttSender::sendData(const uint64_t &data, const bool &save)
    {
        std::string msg = to_string(data) + "u64_";

        if(save)
        {
            rtc::rtc_memory.saveToMemWithData((static_cast<rtc_mem_t>(data)));
        }
        
        return sendMsg(msg);
    }

    /**
    * @brief Publish Int data to MQTT Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param data - data to publish
    * @param save - if true - saves data to RTC Memory
    * 
    * @return true if published
    */
    const bool MqttSender::sendData(const int &data, const bool &save)
    {
        std::string msg = to_string(data) + "i_";

        if(save)
        {
            rtc::rtc_memory.saveToMemWithData((static_cast<rtc_mem_t>(data)));
        }
        
        return sendMsg(msg);
    }

    /**
    * @brief Publish Uin32_t data to MQTT Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param data - data to publish
    * @param save - if true - saves data to RTC Memory
    * 
    * @return true if published
    */
    const bool MqttSender::sendData(const uint32_t &data, const bool &save)
    {
        std::string msg = to_string(data) + "u32_";

        if(save)
        {
            rtc::rtc_memory.saveToMemWithData((static_cast<rtc_mem_t>(data)));
        }
        
        return sendMsg(msg);
    }

    /**
    * @brief Publish Two Uint16_t data to MQTT Feed
    * if failed - goes deep Sleep with proper ERROR
    * @param data1 and data2 - data to publish
    * 
    * @return true if published
    */
    const bool MqttSender::sendData(const uint16_t &data1, const uint16_t &data2)
    {
        std::string msg = to_string(data1) + '&' +  to_string(data2) + "u16_";
        return sendMsg(msg);
    }

    /**
    * @brief Checks if given ID matches ID of Device
    * @note That function will be used for checking if received instruction is for this device or other
    * @param device_id - id to compare
    * 
    * @return true if published
    */
    const bool MqttSender::compareID(const uint8_t &device_id)
    {
        return _k_dev_id == device_id;
    }

    void checkForCommands()
    {
        //TODO: Implement if decide that part is needed
    }
    #endif //ESP8266

}//namespace connection

/**
* @brief Formats string to MQTT Feed form
* 
* @param feed_name - string that will be formatted
* @return MQTT feed as string
*/
static const std::string feedToString(const std::string &feed_name)
{
    if(!feed_name.empty() && feed_name[0] == 'feeds/data')
    {
        return std::string(IO_USER + feed_name);
    }

    std::string feed(IO_USER);
    feed += '/' + feed_name;
    return feed;
}
